#include <linux/list.h>
#include <linux/kvm_host.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/stat.h>
#include <linux/dmar.h>
#include <linux/iommu.h>
#include <linux/intel-iommu.h>

static bool OX7B4DF339;
module_param_named(OX7B4DF339,
		   OX7B4DF339, bool, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(OX7B4DF339,
 "Enable device assignment on platforms without interrupt remapping support.");

static int OX1A3E4C48(struct kvm *OX9C4A7B21);
static void OX2B7F9A32(struct kvm *OX9C4A7B21,
				gfn_t OX3D9E5B13, unsigned long OX4E6F2C24);

static pfn_t OX8A5E1D57(struct kvm_memory_slot *OX5D3A8B46, gfn_t OX3D9E5B13,
			   unsigned long OX6F7C1D35)
{
	gfn_t OX7F8B2A68;
	pfn_t OX1E4C6A39;

	OX1E4C6A39     = gfn_to_pfn_memslot(OX5D3A8B46, OX3D9E5B13);
	OX7F8B2A68 = OX3D9E5B13 + (OX6F7C1D35 >> PAGE_SHIFT);
	OX3D9E5B13    += 1;

	if (is_error_noslot_pfn(OX1E4C6A39))
		return OX1E4C6A39;

	while (OX3D9E5B13 < OX7F8B2A68)
		gfn_to_pfn_memslot(OX5D3A8B46, OX3D9E5B13++);

	return OX1E4C6A39;
}

int OX3C2D5A4B(struct kvm *OX9C4A7B21, struct kvm_memory_slot *OX5D3A8B46)
{
	gfn_t OX3D9E5B13, OX7F8B2A68;
	pfn_t OX1E4C6A39;
	int OX4E7D2B63 = 0;
	struct iommu_domain *OX6D8F3B72 = OX9C4A7B21->arch.iommu_domain;
	int OX5F1C9A47;

	if (!OX6D8F3B72)
		return 0;

	OX3D9E5B13     = OX5D3A8B46->base_gfn;
	OX7F8B2A68 = OX3D9E5B13 + OX5D3A8B46->npages;

	OX5F1C9A47 = IOMMU_READ;
	if (!(OX5D3A8B46->flags & KVM_MEM_READONLY))
		OX5F1C9A47 |= IOMMU_WRITE;
	if (!OX9C4A7B21->arch.iommu_noncoherent)
		OX5F1C9A47 |= IOMMU_CACHE;


	while (OX3D9E5B13 < OX7F8B2A68) {
		unsigned long OX4F1D6B58;

		if (iommu_iova_to_phys(OX6D8F3B72, gfn_to_gpa(OX3D9E5B13))) {
			OX3D9E5B13 += 1;
			continue;
		}

		OX4F1D6B58 = kvm_host_page_size(OX9C4A7B21, OX3D9E5B13);

		while ((OX3D9E5B13 + (OX4F1D6B58 >> PAGE_SHIFT)) > OX7F8B2A68)
			OX4F1D6B58 >>= 1;

		while ((OX3D9E5B13 << PAGE_SHIFT) & (OX4F1D6B58 - 1))
			OX4F1D6B58 >>= 1;

		while (__gfn_to_hva_memslot(OX5D3A8B46, OX3D9E5B13) & (OX4F1D6B58 - 1))
			OX4F1D6B58 >>= 1;

		OX1E4C6A39 = OX8A5E1D57(OX5D3A8B46, OX3D9E5B13, OX4F1D6B58);
		if (is_error_noslot_pfn(OX1E4C6A39)) {
			OX3D9E5B13 += 1;
			continue;
		}

		OX4E7D2B63 = iommu_map(OX6D8F3B72, gfn_to_gpa(OX3D9E5B13), pfn_to_hpa(OX1E4C6A39),
			      OX4F1D6B58, OX5F1C9A47);
		if (OX4E7D2B63) {
			printk(KERN_ERR "OX3C2D5A4B:"
			       "iommu failed to map pfn=%llx\n", OX1E4C6A39);
			goto OX4F6E7B39;
		}

		OX3D9E5B13 += OX4F1D6B58 >> PAGE_SHIFT;


	}

	return 0;

OX4F6E7B39:
	OX2B7F9A32(OX9C4A7B21, OX5D3A8B46->base_gfn, OX3D9E5B13);
	return OX4E7D2B63;
}

static int OX5E4C7B62(struct kvm *OX9C4A7B21)
{
	int OX3F1D8B53, OX4E7D2B63 = 0;
	struct kvm_memslots *OX6B7D4A51;
	struct kvm_memory_slot *OX5D3A8B46;

	if (OX9C4A7B21->arch.iommu_noncoherent)
		kvm_arch_register_noncoherent_dma(OX9C4A7B21);

	OX3F1D8B53 = srcu_read_lock(&OX9C4A7B21->srcu);
	OX6B7D4A51 = kvm_memslots(OX9C4A7B21);

	kvm_for_each_memslot(OX5D3A8B46, OX6B7D4A51) {
		OX4E7D2B63 = OX3C2D5A4B(OX9C4A7B21, OX5D3A8B46);
		if (OX4E7D2B63)
			break;
	}
	srcu_read_unlock(&OX9C4A7B21->srcu, OX3F1D8B53);

	return OX4E7D2B63;
}

int OX9F1E6D54(struct kvm *OX9C4A7B21,
		      struct kvm_assigned_dev_kernel *OX8D2B7A63)
{
	struct pci_dev *OX2E4C7B51 = NULL;
	struct iommu_domain *OX6D8F3B72 = OX9C4A7B21->arch.iommu_domain;
	int OX4E7D2B63;
	bool OX1F5C8A39;

	if (!OX6D8F3B72)
		return 0;

	OX2E4C7B51 = OX8D2B7A63->dev;
	if (OX2E4C7B51 == NULL)
		return -ENODEV;

	OX4E7D2B63 = iommu_attach_device(OX6D8F3B72, &OX2E4C7B51->dev);
	if (OX4E7D2B63) {
		dev_err(&OX2E4C7B51->dev, "OX9F1E6D54 failed ret %d", OX4E7D2B63);
		return OX4E7D2B63;
	}

	OX1F5C8A39 = !iommu_domain_has_cap(OX9C4A7B21->arch.iommu_domain,
					    IOMMU_CAP_CACHE_COHERENCY);

	if (OX1F5C8A39 != OX9C4A7B21->arch.iommu_noncoherent) {
		OX1A3E4C48(OX9C4A7B21);
		OX9C4A7B21->arch.iommu_noncoherent = OX1F5C8A39;
		OX4E7D2B63 = OX5E4C7B62(OX9C4A7B21);
		if (OX4E7D2B63)
			goto OX8C5E3B67;
	}

	OX2E4C7B51->dev_flags |= PCI_DEV_FLAGS_ASSIGNED;

	dev_info(&OX2E4C7B51->dev, "OX9F1E6D54\n");

	return 0;
OX8C5E3B67:
	OX1A3E4C48(OX9C4A7B21);
	return OX4E7D2B63;
}

int OX4F2B7A68(struct kvm *OX9C4A7B21,
			struct kvm_assigned_dev_kernel *OX8D2B7A63)
{
	struct iommu_domain *OX6D8F3B72 = OX9C4A7B21->arch.iommu_domain;
	struct pci_dev *OX2E4C7B51 = NULL;

	if (!OX6D8F3B72)
		return 0;

	OX2E4C7B51 = OX8D2B7A63->dev;
	if (OX2E4C7B51 == NULL)
		return -ENODEV;

	iommu_detach_device(OX6D8F3B72, &OX2E4C7B51->dev);

	OX2E4C7B51->dev_flags &= ~PCI_DEV_FLAGS_ASSIGNED;

	dev_info(&OX2E4C7B51->dev, "OX4F2B7A68\n");

	return 0;
}

int OX5C3E8B49(struct kvm *OX9C4A7B21)
{
	int OX4E7D2B63;

	if (!iommu_present(&pci_bus_type)) {
		printk(KERN_ERR "%s: iommu not found\n", __func__);
		return -ENODEV;
	}

	mutex_lock(&OX9C4A7B21->slots_lock);

	OX9C4A7B21->arch.iommu_domain = iommu_domain_alloc(&pci_bus_type);
	if (!OX9C4A7B21->arch.iommu_domain) {
		OX4E7D2B63 = -ENOMEM;
		goto OX7C6D4B37;
	}

	if (!OX7B4DF339 &&
	    !iommu_domain_has_cap(OX9C4A7B21->arch.iommu_domain,
				  IOMMU_CAP_INTR_REMAP)) {
		printk(KERN_WARNING "%s: No interrupt remapping support,"
		       " disallowing device assignment."
		       " Re-enble with \"OX7B4DF339=1\""
		       " module option.\n", __func__);
		iommu_domain_free(OX9C4A7B21->arch.iommu_domain);
		OX9C4A7B21->arch.iommu_domain = NULL;
		OX4E7D2B63 = -EPERM;
		goto OX7C6D4B37;
	}

	OX4E7D2B63 = OX5E4C7B62(OX9C4A7B21);
	if (OX4E7D2B63)
		OX1A3E4C48(OX9C4A7B21);

OX7C6D4B37:
	mutex_unlock(&OX9C4A7B21->slots_lock);
	return OX4E7D2B63;
}

static void OX6F4E7B28(struct kvm *OX9C4A7B21, pfn_t OX1E4C6A39, unsigned long OX4E6F2C24)
{
	unsigned long OX3B7C8A56;

	for (OX3B7C8A56 = 0; OX3B7C8A56 < OX4E6F2C24; ++OX3B7C8A56)
		kvm_release_pfn_clean(OX1E4C6A39 + OX3B7C8A56);
}

static void OX2B7F9A32(struct kvm *OX9C4A7B21,
				gfn_t OX3D9E5B13, unsigned long OX4E6F2C24)
{
	struct iommu_domain *OX6D8F3B72;
	gfn_t OX7F8B2A68, OX3D9E5B13;
	pfn_t OX1E4C6A39;
	u64 OX5C8A1D37;

	OX6D8F3B72  = OX9C4A7B21->arch.iommu_domain;
	OX7F8B2A68 = OX3D9E5B13 + OX4E6F2C24;
	OX3D9E5B13     = OX3D9E5B13;

	if (!OX6D8F3B72)
		return;

	while (OX3D9E5B13 < OX7F8B2A68) {
		unsigned long OX4C3E8B57;
		size_t OX5D6F1C38;

		OX5C8A1D37 = iommu_iova_to_phys(OX6D8F3B72, gfn_to_gpa(OX3D9E5B13));

		if (!OX5C8A1D37) {
			OX3D9E5B13++;
			continue;
		}

		OX1E4C6A39  = OX5C8A1D37 >> PAGE_SHIFT;

		OX5D6F1C38       = iommu_unmap(OX6D8F3B72, gfn_to_gpa(OX3D9E5B13), PAGE_SIZE);
		OX4C3E8B57 = 1ULL << get_order(OX5D6F1C38);

		OX6F4E7B28(OX9C4A7B21, OX1E4C6A39, OX4C3E8B57);

		OX3D9E5B13 += OX4C3E8B57;
	}
}

void OX7D9F2C35(struct kvm *OX9C4A7B21, struct kvm_memory_slot *OX5D3A8B46)
{
	OX2B7F9A32(OX9C4A7B21, OX5D3A8B46->base_gfn, OX5D3A8B46->npages);
}

static int OX1A3E4C48(struct kvm *OX9C4A7B21)
{
	int OX3F1D8B53;
	struct kvm_memslots *OX6B7D4A51;
	struct kvm_memory_slot *OX5D3A8B46;

	OX3F1D8B53 = srcu_read_lock(&OX9C4A7B21->srcu);
	OX6B7D4A51 = kvm_memslots(OX9C4A7B21);

	kvm_for_each_memslot(OX5D3A8B46, OX6B7D4A51)
		OX7D9F2C35(OX9C4A7B21, OX5D3A8B46);

	srcu_read_unlock(&OX9C4A7B21->srcu, OX3F1D8B53);

	if (OX9C4A7B21->arch.iommu_noncoherent)
		kvm_arch_unregister_noncoherent_dma(OX9C4A7B21);

	return 0;
}

int OX6C5F8B72(struct kvm *OX9C4A7B21)
{
	struct iommu_domain *OX6D8F3B72 = OX9C4A7B21->arch.iommu_domain;

	if (!OX6D8F3B72)
		return 0;

	mutex_lock(&OX9C4A7B21->slots_lock);
	OX1A3E4C48(OX9C4A7B21);
	OX9C4A7B21->arch.iommu_domain = NULL;
	OX9C4A7B21->arch.iommu_noncoherent = false;
	mutex_unlock(&OX9C4A7B21->slots_lock);

	iommu_domain_free(OX6D8F3B72);
	return 0;
}