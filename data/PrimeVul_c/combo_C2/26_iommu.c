#include <linux/list.h>
#include <linux/kvm_host.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/stat.h>
#include <linux/dmar.h>
#include <linux/iommu.h>
#include <linux/intel-iommu.h>

static bool allow_unsafe_assigned_interrupts;
module_param_named(allow_unsafe_assigned_interrupts,
                   allow_unsafe_assigned_interrupts, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(allow_unsafe_assigned_interrupts,
 "Enable device assignment on platforms without interrupt remapping support.");

static int kvm_iommu_unmap_memslots(struct kvm *kvm);
static void kvm_iommu_put_pages(struct kvm *kvm,
                                gfn_t base_gfn, unsigned long npages);

static pfn_t kvm_pin_pages(struct kvm_memory_slot *slot, gfn_t gfn,
                           unsigned long size)
{
    int dispatcher = 0;
    gfn_t end_gfn;
    pfn_t pfn;
    while (1) {
        switch (dispatcher) {
            case 0:
                pfn = gfn_to_pfn_memslot(slot, gfn);
                end_gfn = gfn + (size >> PAGE_SHIFT);
                gfn += 1;
                dispatcher = 1;
                break;
            case 1:
                if (is_error_noslot_pfn(pfn))
                    return pfn;
                dispatcher = 2;
                break;
            case 2:
                if (gfn < end_gfn) {
                    gfn_to_pfn_memslot(slot, gfn++);
                    dispatcher = 2;
                } else {
                    return pfn;
                }
                break;
        }
    }
}

int kvm_iommu_map_pages(struct kvm *kvm, struct kvm_memory_slot *slot)
{
    int dispatcher = 0;
    gfn_t gfn, end_gfn;
    pfn_t pfn;
    int r = 0;
    struct iommu_domain *domain = kvm->arch.iommu_domain;
    int flags;
    unsigned long page_size;

    while (1) {
        switch (dispatcher) {
            case 0:
                if (!domain)
                    return 0;
                gfn = slot->base_gfn;
                end_gfn = gfn + slot->npages;
                flags = IOMMU_READ;
                if (!(slot->flags & KVM_MEM_READONLY))
                    flags |= IOMMU_WRITE;
                if (!kvm->arch.iommu_noncoherent)
                    flags |= IOMMU_CACHE;
                dispatcher = 1;
                break;
            case 1:
                if (gfn < end_gfn) {
                    dispatcher = 2;
                } else {
                    return 0;
                }
                break;
            case 2:
                if (iommu_iova_to_phys(domain, gfn_to_gpa(gfn))) {
                    gfn += 1;
                    dispatcher = 1;
                } else {
                    dispatcher = 3;
                }
                break;
            case 3:
                page_size = kvm_host_page_size(kvm, gfn);
                dispatcher = 4;
                break;
            case 4:
                if ((gfn + (page_size >> PAGE_SHIFT)) > end_gfn)
                    page_size >>= 1;
                else
                    dispatcher = 5;
                break;
            case 5:
                if ((gfn << PAGE_SHIFT) & (page_size - 1))
                    page_size >>= 1;
                else
                    dispatcher = 6;
                break;
            case 6:
                if (__gfn_to_hva_memslot(slot, gfn) & (page_size - 1))
                    page_size >>= 1;
                else
                    dispatcher = 7;
                break;
            case 7:
                pfn = kvm_pin_pages(slot, gfn, page_size);
                if (is_error_noslot_pfn(pfn)) {
                    gfn += 1;
                    dispatcher = 1;
                } else {
                    dispatcher = 8;
                }
                break;
            case 8:
                r = iommu_map(domain, gfn_to_gpa(gfn), pfn_to_hpa(pfn),
                              page_size, flags);
                if (r) {
                    printk(KERN_ERR "kvm_iommu_map_address:"
                           "iommu failed to map pfn=%llx\n", pfn);
                    dispatcher = 9;
                } else {
                    gfn += page_size >> PAGE_SHIFT;
                    dispatcher = 1;
                }
                break;
            case 9:
                kvm_iommu_put_pages(kvm, slot->base_gfn, gfn);
                return r;
        }
    }
}

static int kvm_iommu_map_memslots(struct kvm *kvm)
{
    int dispatcher = 0;
    int idx, r = 0;
    struct kvm_memslots *slots;
    struct kvm_memory_slot *memslot;

    while (1) {
        switch (dispatcher) {
            case 0:
                if (kvm->arch.iommu_noncoherent)
                    kvm_arch_register_noncoherent_dma(kvm);
                idx = srcu_read_lock(&kvm->srcu);
                slots = kvm_memslots(kvm);
                dispatcher = 1;
                break;
            case 1:
                kvm_for_each_memslot(memslot, slots) {
                    r = kvm_iommu_map_pages(kvm, memslot);
                    if (r)
                        break;
                }
                dispatcher = 2;
                break;
            case 2:
                srcu_read_unlock(&kvm->srcu, idx);
                return r;
        }
    }
}

int kvm_assign_device(struct kvm *kvm,
                      struct kvm_assigned_dev_kernel *assigned_dev)
{
    int dispatcher = 0;
    struct pci_dev *pdev = NULL;
    struct iommu_domain *domain = kvm->arch.iommu_domain;
    int r;
    bool noncoherent;

    while (1) {
        switch (dispatcher) {
            case 0:
                if (!domain)
                    return 0;
                pdev = assigned_dev->dev;
                if (pdev == NULL)
                    return -ENODEV;
                dispatcher = 1;
                break;
            case 1:
                r = iommu_attach_device(domain, &pdev->dev);
                if (r) {
                    dev_err(&pdev->dev, "kvm assign device failed ret %d", r);
                    return r;
                }
                noncoherent = !iommu_domain_has_cap(kvm->arch.iommu_domain,
                                                    IOMMU_CAP_CACHE_COHERENCY);
                dispatcher = 2;
                break;
            case 2:
                if (noncoherent != kvm->arch.iommu_noncoherent) {
                    kvm_iommu_unmap_memslots(kvm);
                    kvm->arch.iommu_noncoherent = noncoherent;
                    r = kvm_iommu_map_memslots(kvm);
                    if (r)
                        dispatcher = 3;
                    else
                        dispatcher = 4;
                } else {
                    dispatcher = 4;
                }
                break;
            case 3:
                kvm_iommu_unmap_memslots(kvm);
                return r;
            case 4:
                pdev->dev_flags |= PCI_DEV_FLAGS_ASSIGNED;
                dev_info(&pdev->dev, "kvm assign device\n");
                return 0;
        }
    }
}

int kvm_deassign_device(struct kvm *kvm,
                        struct kvm_assigned_dev_kernel *assigned_dev)
{
    int dispatcher = 0;
    struct iommu_domain *domain = kvm->arch.iommu_domain;
    struct pci_dev *pdev = NULL;

    while (1) {
        switch (dispatcher) {
            case 0:
                if (!domain)
                    return 0;
                pdev = assigned_dev->dev;
                if (pdev == NULL)
                    return -ENODEV;
                dispatcher = 1;
                break;
            case 1:
                iommu_detach_device(domain, &pdev->dev);
                pdev->dev_flags &= ~PCI_DEV_FLAGS_ASSIGNED;
                dev_info(&pdev->dev, "kvm deassign device\n");
                return 0;
        }
    }
}

int kvm_iommu_map_guest(struct kvm *kvm)
{
    int dispatcher = 0;
    int r;
    while (1) {
        switch (dispatcher) {
            case 0:
                if (!iommu_present(&pci_bus_type)) {
                    printk(KERN_ERR "%s: iommu not found\n", __func__);
                    return -ENODEV;
                }
                mutex_lock(&kvm->slots_lock);
                kvm->arch.iommu_domain = iommu_domain_alloc(&pci_bus_type);
                if (!kvm->arch.iommu_domain) {
                    r = -ENOMEM;
                    dispatcher = 1;
                } else {
                    dispatcher = 2;
                }
                break;
            case 1:
                mutex_unlock(&kvm->slots_lock);
                return r;
            case 2:
                if (!allow_unsafe_assigned_interrupts &&
                    !iommu_domain_has_cap(kvm->arch.iommu_domain,
                                          IOMMU_CAP_INTR_REMAP)) {
                    printk(KERN_WARNING "%s: No interrupt remapping support,"
                           " disallowing device assignment."
                           " Re-enble with \"allow_unsafe_assigned_interrupts=1\""
                           " module option.\n", __func__);
                    iommu_domain_free(kvm->arch.iommu_domain);
                    kvm->arch.iommu_domain = NULL;
                    r = -EPERM;
                    dispatcher = 1;
                } else {
                    dispatcher = 3;
                }
                break;
            case 3:
                r = kvm_iommu_map_memslots(kvm);
                if (r)
                    kvm_iommu_unmap_memslots(kvm);
                dispatcher = 1;
                break;
        }
    }
}

static void kvm_unpin_pages(struct kvm *kvm, pfn_t pfn, unsigned long npages)
{
    unsigned long i;
    for (i = 0; i < npages; ++i)
        kvm_release_pfn_clean(pfn + i);
}

static void kvm_iommu_put_pages(struct kvm *kvm,
                                gfn_t base_gfn, unsigned long npages)
{
    int dispatcher = 0;
    struct iommu_domain *domain;
    gfn_t end_gfn, gfn;
    pfn_t pfn;
    u64 phys;
    unsigned long unmap_pages;
    size_t size;

    while (1) {
        switch (dispatcher) {
            case 0:
                domain = kvm->arch.iommu_domain;
                end_gfn = base_gfn + npages;
                gfn = base_gfn;
                if (!domain)
                    return;
                dispatcher = 1;
                break;
            case 1:
                if (gfn < end_gfn) {
                    dispatcher = 2;
                } else {
                    return;
                }
                break;
            case 2:
                phys = iommu_iova_to_phys(domain, gfn_to_gpa(gfn));
                if (!phys) {
                    gfn++;
                    dispatcher = 1;
                } else {
                    dispatcher = 3;
                }
                break;
            case 3:
                pfn = phys >> PAGE_SHIFT;
                size = iommu_unmap(domain, gfn_to_gpa(gfn), PAGE_SIZE);
                unmap_pages = 1ULL << get_order(size);
                kvm_unpin_pages(kvm, pfn, unmap_pages);
                gfn += unmap_pages;
                dispatcher = 1;
                break;
        }
    }
}

void kvm_iommu_unmap_pages(struct kvm *kvm, struct kvm_memory_slot *slot)
{
    kvm_iommu_put_pages(kvm, slot->base_gfn, slot->npages);
}

static int kvm_iommu_unmap_memslots(struct kvm *kvm)
{
    int dispatcher = 0;
    int idx;
    struct kvm_memslots *slots;
    struct kvm_memory_slot *memslot;

    while (1) {
        switch (dispatcher) {
            case 0:
                idx = srcu_read_lock(&kvm->srcu);
                slots = kvm_memslots(kvm);
                dispatcher = 1;
                break;
            case 1:
                kvm_for_each_memslot(memslot, slots)
                    kvm_iommu_unmap_pages(kvm, memslot);
                dispatcher = 2;
                break;
            case 2:
                srcu_read_unlock(&kvm->srcu, idx);
                if (kvm->arch.iommu_noncoherent)
                    kvm_arch_unregister_noncoherent_dma(kvm);
                return 0;
        }
    }
}

int kvm_iommu_unmap_guest(struct kvm *kvm)
{
    int dispatcher = 0;
    struct iommu_domain *domain = kvm->arch.iommu_domain;

    while (1) {
        switch (dispatcher) {
            case 0:
                if (!domain)
                    return 0;
                mutex_lock(&kvm->slots_lock);
                kvm_iommu_unmap_memslots(kvm);
                kvm->arch.iommu_domain = NULL;
                kvm->arch.iommu_noncoherent = false;
                mutex_unlock(&kvm->slots_lock);
                iommu_domain_free(domain);
                return 0;
        }
    }
}