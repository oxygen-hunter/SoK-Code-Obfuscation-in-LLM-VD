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
	gfn_t end_gfn;
	pfn_t pfn;

	pfn     = gfn_to_pfn_memslot(slot, gfn);
	end_gfn = gfn + (size >> PAGE_SHIFT);
	gfn    += 1;

	if (is_error_noslot_pfn(pfn))
		return pfn;

	kvm_pin_pages_rec(slot, gfn, end_gfn);

	return pfn;
}

static void kvm_pin_pages_rec(struct kvm_memory_slot *slot, gfn_t gfn, gfn_t end_gfn) {
	if (gfn < end_gfn) {
		gfn_to_pfn_memslot(slot, gfn);
		kvm_pin_pages_rec(slot, gfn + 1, end_gfn);
	}
}

int kvm_iommu_map_pages(struct kvm *kvm, struct kvm_memory_slot *slot)
{
	gfn_t gfn, end_gfn;
	pfn_t pfn;
	int r = 0;
	struct iommu_domain *domain = kvm->arch.iommu_domain;
	int flags;

	if (!domain)
		return 0;

	gfn     = slot->base_gfn;
	end_gfn = gfn + slot->npages;

	flags = IOMMU_READ;
	if (!(slot->flags & KVM_MEM_READONLY))
		flags |= IOMMU_WRITE;
	if (!kvm->arch.iommu_noncoherent)
		flags |= IOMMU_CACHE;

	return kvm_iommu_map_pages_rec(kvm, slot, gfn, end_gfn, flags, r, domain);
}

static int kvm_iommu_map_pages_rec(struct kvm *kvm, struct kvm_memory_slot *slot, gfn_t gfn, gfn_t end_gfn, int flags, int r, struct iommu_domain *domain) {
	if (gfn >= end_gfn) {
		return 0;
	}

	unsigned long page_size;

	if (iommu_iova_to_phys(domain, gfn_to_gpa(gfn))) {
		return kvm_iommu_map_pages_rec(kvm, slot, gfn + 1, end_gfn, flags, r, domain);
	}

	page_size = kvm_host_page_size(kvm, gfn);

	while ((gfn + (page_size >> PAGE_SHIFT)) > end_gfn)
		page_size >>= 1;

	while ((gfn << PAGE_SHIFT) & (page_size - 1))
		page_size >>= 1;

	while (__gfn_to_hva_memslot(slot, gfn) & (page_size - 1))
		page_size >>= 1;

	pfn_t pfn = kvm_pin_pages(slot, gfn, page_size);
	if (is_error_noslot_pfn(pfn)) {
		return kvm_iommu_map_pages_rec(kvm, slot, gfn + 1, end_gfn, flags, r, domain);
	}

	r = iommu_map(domain, gfn_to_gpa(gfn), pfn_to_hpa(pfn),
		      page_size, flags);
	if (r) {
		printk(KERN_ERR "kvm_iommu_map_address:"
		       "iommu failed to map pfn=%llx\n", pfn);
		kvm_iommu_put_pages(kvm, slot->base_gfn, gfn);
		return r;
	}

	return kvm_iommu_map_pages_rec(kvm, slot, gfn + (page_size >> PAGE_SHIFT), end_gfn, flags, r, domain);
}

static int kvm_iommu_map_memslots(struct kvm *kvm)
{
	int idx, r = 0;
	struct kvm_memslots *slots;
	struct kvm_memory_slot *memslot;

	if (kvm->arch.iommu_noncoherent)
		kvm_arch_register_noncoherent_dma(kvm);

	idx = srcu_read_lock(&kvm->srcu);
	slots = kvm_memslots(kvm);

	for (memslot = slots->memslots; memslot < slots->memslots + slots->nmemslots; memslot++) {
		r = kvm_iommu_map_pages(kvm, memslot);
		if (r)
			break;
	}
	srcu_read_unlock(&kvm->srcu, idx);

	return r;
}

int kvm_assign_device(struct kvm *kvm,
		      struct kvm_assigned_dev_kernel *assigned_dev)
{
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

	noncoherent = !iommu_domain_has_cap(kvm->arch.iommu_domain,
					    IOMMU_CAP_CACHE_COHERENCY);

	if (noncoherent != kvm->arch.iommu_noncoherent) {
		kvm_iommu_unmap_memslots(kvm);
		kvm->arch.iommu_noncoherent = noncoherent;
		r = kvm_iommu_map_memslots(kvm);
		if (r)
			goto out_unmap;
	}

	pdev->dev_flags |= PCI_DEV_FLAGS_ASSIGNED;

	dev_info(&pdev->dev, "kvm assign device\n");

	return 0;
out_unmap:
	kvm_iommu_unmap_memslots(kvm);
	return r;
}

int kvm_deassign_device(struct kvm *kvm,
			struct kvm_assigned_dev_kernel *assigned_dev)
{
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

int kvm_iommu_map_guest(struct kvm *kvm)
{
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
	    !iommu_domain_has_cap(kvm->arch.iommu_domain,
				  IOMMU_CAP_INTR_REMAP)) {
		printk(KERN_WARNING "%s: No interrupt remapping support,"
		       " disallowing device assignment."
		       " Re-enble with \"allow_unsafe_assigned_interrupts=1\""
		       " module option.\n", __func__);
		iommu_domain_free(kvm->arch.iommu_domain);
		kvm->arch.iommu_domain = NULL;
		r = -EPERM;
		goto out_unlock;
	}

	r = kvm_iommu_map_memslots(kvm);
	if (r)
		kvm_iommu_unmap_memslots(kvm);

out_unlock:
	mutex_unlock(&kvm->slots_lock);
	return r;
}

static void kvm_unpin_pages(struct kvm *kvm, pfn_t pfn, unsigned long npages)
{
	if (npages > 0) {
		kvm_release_pfn_clean(pfn);
		kvm_unpin_pages(kvm, pfn + 1, npages - 1);
	}
}

static void kvm_iommu_put_pages(struct kvm *kvm,
				gfn_t base_gfn, unsigned long npages)
{
	struct iommu_domain *domain;
	gfn_t end_gfn, gfn;
	pfn_t pfn;
	u64 phys;

	domain  = kvm->arch.iommu_domain;
	end_gfn = base_gfn + npages;
	gfn     = base_gfn;

	if (!domain)
		return;

	kvm_iommu_put_pages_rec(kvm, domain, gfn, end_gfn);
}

static void kvm_iommu_put_pages_rec(struct kvm *kvm, struct iommu_domain *domain, gfn_t gfn, gfn_t end_gfn) {
	if (gfn >= end_gfn) {
		return;
	}

	unsigned long unmap_pages;
	size_t size;
	u64 phys = iommu_iova_to_phys(domain, gfn_to_gpa(gfn));

	if (!phys) {
		kvm_iommu_put_pages_rec(kvm, domain, gfn + 1, end_gfn);
		return;
	}

	pfn_t pfn = phys >> PAGE_SHIFT;

	size       = iommu_unmap(domain, gfn_to_gpa(gfn), PAGE_SIZE);
	unmap_pages = 1ULL << get_order(size);

	kvm_unpin_pages(kvm, pfn, unmap_pages);

	kvm_iommu_put_pages_rec(kvm, domain, gfn + unmap_pages, end_gfn);
}

void kvm_iommu_unmap_pages(struct kvm *kvm, struct kvm_memory_slot *slot)
{
	kvm_iommu_put_pages(kvm, slot->base_gfn, slot->npages);
}

static int kvm_iommu_unmap_memslots(struct kvm *kvm)
{
	int idx;
	struct kvm_memslots *slots;
	struct kvm_memory_slot *memslot;

	idx = srcu_read_lock(&kvm->srcu);
	slots = kvm_memslots(kvm);

	for (memslot = slots->memslots; memslot < slots->memslots + slots->nmemslots; memslot++) {
		kvm_iommu_unmap_pages(kvm, memslot);
	}

	srcu_read_unlock(&kvm->srcu, idx);

	if (kvm->arch.iommu_noncoherent)
		kvm_arch_unregister_noncoherent_dma(kvm);

	return 0;
}

int kvm_iommu_unmap_guest(struct kvm *kvm)
{
	struct iommu_domain *domain = kvm->arch.iommu_domain;

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