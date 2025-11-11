#include <linux/list.h>
#include <linux/kvm_host.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/stat.h>
#include <linux/dmar.h>
#include <linux/iommu.h>
#include <linux/intel-iommu.h>

typedef enum {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, HALT
} InstructionSet;

typedef struct {
    int stack[256];
    int sp;
    int pc;
    bool running;
} VirtualMachine;

void vm_init(VirtualMachine* vm) {
    vm->sp = -1;
    vm->pc = 0;
    vm->running = true;
}

void vm_push(VirtualMachine* vm, int value) {
    vm->stack[++vm->sp] = value;
}

int vm_pop(VirtualMachine* vm) {
    return vm->stack[vm->sp--];
}

void vm_execute(VirtualMachine* vm, int* program) {
    while (vm->running) {
        switch (program[vm->pc++]) {
            case PUSH:
                vm_push(vm, program[vm->pc++]);
                break;
            case POP:
                vm_pop(vm);
                break;
            case ADD: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a + b);
                break;
            }
            case SUB: {
                int b = vm_pop(vm);
                int a = vm_pop(vm);
                vm_push(vm, a - b);
                break;
            }
            case JMP:
                vm->pc = program[vm->pc];
                break;
            case JZ: {
                int addr = program[vm->pc++];
                if (vm_pop(vm) == 0) {
                    vm->pc = addr;
                }
                break;
            }
            case LOAD:
                vm_push(vm, program[vm->pc++]);
                break;
            case STORE:
                program[program[vm->pc++]] = vm_pop(vm);
                break;
            case HALT:
                vm->running = false;
                break;
        }
    }
}

static int kvm_iommu_unmap_memslots_vm(struct kvm *kvm);
static void kvm_iommu_put_pages_vm(struct kvm *kvm, gfn_t base_gfn, unsigned long npages);

static pfn_t kvm_pin_pages_vm(struct kvm_memory_slot *slot, gfn_t gfn, unsigned long size) {
    gfn_t end_gfn;
    pfn_t pfn;

    pfn = gfn_to_pfn_memslot(slot, gfn);
    end_gfn = gfn + (size >> PAGE_SHIFT);
    gfn += 1;

    if (is_error_noslot_pfn(pfn))
        return pfn;

    while (gfn < end_gfn)
        gfn_to_pfn_memslot(slot, gfn++);

    return pfn;
}

int kvm_iommu_map_pages_vm(struct kvm *kvm, struct kvm_memory_slot *slot) {
    gfn_t gfn, end_gfn;
    pfn_t pfn;
    int r = 0;
    struct iommu_domain *domain = kvm->arch.iommu_domain;
    int flags;

    if (!domain)
        return 0;

    gfn = slot->base_gfn;
    end_gfn = gfn + slot->npages;

    flags = IOMMU_READ;
    if (!(slot->flags & KVM_MEM_READONLY))
        flags |= IOMMU_WRITE;
    if (!kvm->arch.iommu_noncoherent)
        flags |= IOMMU_CACHE;

    int program[] = {
        LOAD, gfn,
        LOAD, end_gfn,
        LOAD, 1,
        ADD,
        LOAD, (int)&iommu_iova_to_phys,
        CALL,
        JZ, 20,
        LOAD, (int)&kvm_host_page_size,
        CALL,
        LOAD, PAGE_SHIFT,
        SHR,
        ADD,
        JMP, 0,
        HALT
    };

    VirtualMachine vm;
    vm_init(&vm);
    vm_execute(&vm, program);

    return 0;
}

static int kvm_iommu_map_memslots_vm(struct kvm *kvm) {
    int idx, r = 0;
    struct kvm_memslots *slots;
    struct kvm_memory_slot *memslot;

    if (kvm->arch.iommu_noncoherent)
        kvm_arch_register_noncoherent_dma(kvm);

    idx = srcu_read_lock(&kvm->srcu);
    slots = kvm_memslots(kvm);

    kvm_for_each_memslot(memslot, slots) {
        r = kvm_iommu_map_pages_vm(kvm, memslot);
        if (r)
            break;
    }
    srcu_read_unlock(&kvm->srcu, idx);

    return r;
}

int kvm_assign_device_vm(struct kvm *kvm, struct kvm_assigned_dev_kernel *assigned_dev) {
    struct pci_dev *pdev = NULL;
    struct iommu_domain *domain = kvm->arch.iommu_domain;
    int r;
    bool noncoherent;

    if (!domain)
        return 0;

    pdev = assigned_dev->dev;
    if (pdev == NULL)
        return -ENODEV;

    r = iommu_attach_device(domain, &pdev->dev);
    if (r) {
        dev_err(&pdev->dev, "kvm assign device failed ret %d", r);
        return r;
    }

    noncoherent = !iommu_domain_has_cap(kvm->arch.iommu_domain, IOMMU_CAP_CACHE_COHERENCY);

    if (noncoherent != kvm->arch.iommu_noncoherent) {
        kvm_iommu_unmap_memslots_vm(kvm);
        kvm->arch.iommu_noncoherent = noncoherent;
        r = kvm_iommu_map_memslots_vm(kvm);
        if (r)
            goto out_unmap;
    }

    pdev->dev_flags |= PCI_DEV_FLAGS_ASSIGNED;

    dev_info(&pdev->dev, "kvm assign device\n");

    return 0;
out_unmap:
    kvm_iommu_unmap_memslots_vm(kvm);
    return r;
}

int kvm_deassign_device_vm(struct kvm *kvm, struct kvm_assigned_dev_kernel *assigned_dev) {
    struct iommu_domain *domain = kvm->arch.iommu_domain;
    struct pci_dev *pdev = NULL;

    if (!domain)
        return 0;

    pdev = assigned_dev->dev;
    if (pdev == NULL)
        return -ENODEV;

    iommu_detach_device(domain, &pdev->dev);

    pdev->dev_flags &= ~PCI_DEV_FLAGS_ASSIGNED;

    dev_info(&pdev->dev, "kvm deassign device\n");

    return 0;
}

int kvm_iommu_map_guest_vm(struct kvm *kvm) {
    int r;

    if (!iommu_present(&pci_bus_type)) {
        printk(KERN_ERR "%s: iommu not found\n", __func__);
        return -ENODEV;
    }

    mutex_lock(&kvm->slots_lock);

    kvm->arch.iommu_domain = iommu_domain_alloc(&pci_bus_type);
    if (!kvm->arch.iommu_domain) {
        r = -ENOMEM;
        goto out_unlock;
    }

    if (!allow_unsafe_assigned_interrupts &&
        !iommu_domain_has_cap(kvm->arch.iommu_domain, IOMMU_CAP_INTR_REMAP)) {
        printk(KERN_WARNING "%s: No interrupt remapping support,"
               " disallowing device assignment."
               " Re-enble with \"allow_unsafe_assigned_interrupts=1\""
               " module option.\n", __func__);
        iommu_domain_free(kvm->arch.iommu_domain);
        kvm->arch.iommu_domain = NULL;
        r = -EPERM;
        goto out_unlock;
    }

    r = kvm_iommu_map_memslots_vm(kvm);
    if (r)
        kvm_iommu_unmap_memslots_vm(kvm);

out_unlock:
    mutex_unlock(&kvm->slots_lock);
    return r;
}

static void kvm_unpin_pages_vm(struct kvm *kvm, pfn_t pfn, unsigned long npages) {
    unsigned long i;

    for (i = 0; i < npages; ++i)
        kvm_release_pfn_clean(pfn + i);
}

static void kvm_iommu_put_pages_vm(struct kvm *kvm, gfn_t base_gfn, unsigned long npages) {
    struct iommu_domain *domain;
    gfn_t end_gfn, gfn;
    pfn_t pfn;
    u64 phys;

    domain = kvm->arch.iommu_domain;
    end_gfn = base_gfn + npages;
    gfn = base_gfn;

    if (!domain)
        return;

    while (gfn < end_gfn) {
        unsigned long unmap_pages;
        size_t size;

        phys = iommu_iova_to_phys(domain, gfn_to_gpa(gfn));

        if (!phys) {
            gfn++;
            continue;
        }

        pfn = phys >> PAGE_SHIFT;
        size = iommu_unmap(domain, gfn_to_gpa(gfn), PAGE_SIZE);
        unmap_pages = 1ULL << get_order(size);
        kvm_unpin_pages_vm(kvm, pfn, unmap_pages);

        gfn += unmap_pages;
    }
}

void kvm_iommu_unmap_pages_vm(struct kvm *kvm, struct kvm_memory_slot *slot) {
    kvm_iommu_put_pages_vm(kvm, slot->base_gfn, slot->npages);
}

static int kvm_iommu_unmap_memslots_vm(struct kvm *kvm) {
    int idx;
    struct kvm_memslots *slots;
    struct kvm_memory_slot *memslot;

    idx = srcu_read_lock(&kvm->srcu);
    slots = kvm_memslots(kvm);

    kvm_for_each_memslot(memslot, slots)
        kvm_iommu_unmap_pages_vm(kvm, memslot);

    srcu_read_unlock(&kvm->srcu, idx);

    if (kvm->arch.iommu_noncoherent)
        kvm_arch_unregister_noncoherent_dma(kvm);

    return 0;
}

int kvm_iommu_unmap_guest_vm(struct kvm *kvm) {
    struct iommu_domain *domain = kvm->arch.iommu_domain;

    if (!domain)
        return 0;

    mutex_lock(&kvm->slots_lock);
    kvm_iommu_unmap_memslots_vm(kvm);
    kvm->arch.iommu_domain = NULL;
    kvm->arch.iommu_noncoherent = false;
    mutex_unlock(&kvm->slots_lock);

    iommu_domain_free(domain);
    return 0;
}