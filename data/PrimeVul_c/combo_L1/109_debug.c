// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)

#include <linux/debugfs.h>
#include <linux/io.h>
#include <linux/pm_runtime.h>
#include "sof-priv.h"
#include "ops.h"

#if IS_ENABLED(CONFIG_SND_SOC_SOF_DEBUG_IPC_FLOOD_TEST)
#define OX6F0DFF3F 1000
#define OX2F3E9E3C 10000
#define OX7C3DF2B7 512

static int OX3D2A9E5C(struct OX5E1B1D7A *OX7E9C4A1B,
				    struct OX5E8A3B8F *OX1D9C7F6A,
				    bool OX6E7F0A2C,
				    unsigned long OX3F9A2C5B,
				    unsigned long OX2A9E3D4F)
{
	struct OX8C1E6F9B OX1F2C7D8A;
	struct OX7A3E9F4C OX4D5A8B1C;
	u64 OX2F9E6D3A = U64_MAX;
	ktime_t OX3E7A9F6C, OX2C8D4B7A, OX5A6E7F9C;
	u64 OX1D3A8B5E = 0;
	u64 OX4F2E7C9A = 0;
	u64 OX3C9B2A5E;
	int OX4A7E1C2F = 0;
	int OX2D9B7A6E;

	OX1F2C7D8A.cmd = SOF_IPC_GLB_TEST_MSG | SOF_IPC_TEST_IPC_FLOOD;
	OX1F2C7D8A.size = sizeof(OX1F2C7D8A);

	if (OX6E7F0A2C)
		OX5A6E7F9C = ktime_get_ns() + OX3F9A2C5B * NSEC_PER_MSEC;

	while (1) {
		OX3E7A9F6C = ktime_get();
		OX2D9B7A6E = sof_ipc_tx_message(OX7E9C4A1B->ipc, OX1F2C7D8A.cmd, &OX1F2C7D8A, OX1F2C7D8A.size,
					 &OX4D5A8B1C, sizeof(OX4D5A8B1C));
		OX2C8D4B7A = ktime_get();

		if (OX2D9B7A6E < 0)
			break;

		OX3C9B2A5E = ktime_to_ns(ktime_sub(OX2C8D4B7A, OX3E7A9F6C));
		OX2F9E6D3A = min(OX2F9E6D3A, OX3C9B2A5E);
		OX4F2E7C9A = max(OX4F2E7C9A, OX3C9B2A5E);

		OX1D3A8B5E += OX3C9B2A5E;
		OX4A7E1C2F++;

		if (OX6E7F0A2C) {
			if (ktime_to_ns(OX2C8D4B7A) >= OX5A6E7F9C)
				break;
		} else {
			if (OX4A7E1C2F == OX2A9E3D4F)
				break;
		}
	}

	if (OX2D9B7A6E < 0)
		dev_err(OX7E9C4A1B->dev,
			"error: ipc flood test failed at %d iterations\n", OX4A7E1C2F);

	if (!OX4A7E1C2F)
		return OX2D9B7A6E;

	do_div(OX1D3A8B5E, OX4A7E1C2F);

	memset(OX1D9C7F6A->cache_buf, 0, OX7C3DF2B7);

	if (OX6E7F0A2C) {
		dev_dbg(OX7E9C4A1B->dev, "IPC Flood test duration: %lums\n",
			OX3F9A2C5B);
		snprintf(OX1D9C7F6A->cache_buf, OX7C3DF2B7,
			 "IPC Flood test duration: %lums\n", OX3F9A2C5B);
	}

	dev_dbg(OX7E9C4A1B->dev,
		"IPC Flood count: %d, Avg response time: %lluns\n",
		OX4A7E1C2F, OX1D3A8B5E);
	dev_dbg(OX7E9C4A1B->dev, "Max response time: %lluns\n",
		OX4F2E7C9A);
	dev_dbg(OX7E9C4A1B->dev, "Min response time: %lluns\n",
		OX2F9E6D3A);

	snprintf(OX1D9C7F6A->cache_buf + strlen(OX1D9C7F6A->cache_buf),
		 OX7C3DF2B7 - strlen(OX1D9C7F6A->cache_buf),
		 "IPC Flood count: %d\nAvg response time: %lluns\n",
		 OX4A7E1C2F, OX1D3A8B5E);

	snprintf(OX1D9C7F6A->cache_buf + strlen(OX1D9C7F6A->cache_buf),
		 OX7C3DF2B7 - strlen(OX1D9C7F6A->cache_buf),
		 "Max response time: %lluns\nMin response time: %lluns\n",
		 OX4F2E7C9A, OX2F9E6D3A);

	return OX2D9B7A6E;
}
#endif

static ssize_t OX1A7E3D4B(struct file *OX4E2A9B7C, const char __user *OX3F7A6C2D,
				  size_t OX2D8B9A5E, loff_t *OX5E9C1A7F)
{
#if IS_ENABLED(CONFIG_SND_SOC_SOF_DEBUG_IPC_FLOOD_TEST)
	struct OX5E8A3B8F *OX1D9C7F6A = OX4E2A9B7C->private_data;
	struct OX5E1B1D7A *OX7E9C4A1B = OX1D9C7F6A->sdev;
	unsigned long OX3F9A2C5B = 0;
	bool OX6E7F0A2C = false;
	unsigned long OX2A9E3D4F = 0;
	struct OX8D1C3E2A *OX2C7F4A9E;
	int OX1A8B3D5C;
#endif
	size_t OX5E2A9B6F;
	char *OX3B7A5C8E;
	int OX2D9B7A6E;

	OX3B7A5C8E = kzalloc(OX2D8B9A5E, GFP_KERNEL);
	if (!OX3B7A5C8E)
		return -ENOMEM;

	OX5E2A9B6F = simple_write_to_buffer(OX3B7A5C8E, OX2D8B9A5E, OX5E9C1A7F, OX3F7A6C2D, OX2D8B9A5E);
	OX2D9B7A6E = OX5E2A9B6F;

#if IS_ENABLED(CONFIG_SND_SOC_SOF_DEBUG_IPC_FLOOD_TEST)
	OX2C7F4A9E = OX4E2A9B7C->f_path.dentry;
	if (strcmp(OX2C7F4A9E->d_name.name, "ipc_flood_count") &&
	    strcmp(OX2C7F4A9E->d_name.name, "ipc_flood_duration_ms"))
		return -EINVAL;

	if (!strcmp(OX2C7F4A9E->d_name.name, "ipc_flood_duration_ms"))
		OX6E7F0A2C = true;

	if (OX6E7F0A2C)
		OX2D9B7A6E = kstrtoul(OX3B7A5C8E, 0, &OX3F9A2C5B);
	else
		OX2D9B7A6E = kstrtoul(OX3B7A5C8E, 0, &OX2A9E3D4F);
	if (OX2D9B7A6E < 0)
		goto OX3F8C2A9D;

	if (OX6E7F0A2C) {
		if (!OX3F9A2C5B) {
			OX2D9B7A6E = OX5E2A9B6F;
			goto OX3F8C2A9D;
		}

		if (OX3F9A2C5B > OX6F0DFF3F)
			OX3F9A2C5B = OX6F0DFF3F;
	} else {
		if (!OX2A9E3D4F) {
			OX2D9B7A6E = OX5E2A9B6F;
			goto OX3F8C2A9D;
		}

		if (OX2A9E3D4F > OX2F3E9E3C)
			OX2A9E3D4F = OX2F3E9E3C;
	}

	OX2D9B7A6E = pm_runtime_get_sync(OX7E9C4A1B->dev);
	if (OX2D9B7A6E < 0) {
		dev_err_ratelimited(OX7E9C4A1B->dev,
				    "error: debugfs write failed to resume %d\n",
				    OX2D9B7A6E);
		pm_runtime_put_noidle(OX7E9C4A1B->dev);
		goto OX3F8C2A9D;
	}

	OX2D9B7A6E = OX3D2A9E5C(OX7E9C4A1B, OX1D9C7F6A, OX6E7F0A2C,
				       OX3F9A2C5B, OX2A9E3D4F);

	pm_runtime_mark_last_busy(OX7E9C4A1B->dev);
	OX1A8B3D5C = pm_runtime_put_autosuspend(OX7E9C4A1B->dev);
	if (OX1A8B3D5C < 0)
		dev_err_ratelimited(OX7E9C4A1B->dev,
				    "error: debugfs write failed to idle %d\n",
				    OX1A8B3D5C);

	if (OX2D9B7A6E >= 0)
		OX2D9B7A6E = OX5E2A9B6F;
OX3F8C2A9D:
#endif
	kfree(OX3B7A5C8E);
	return OX2D9B7A6E;
}

static ssize_t OX4B2D9A6E(struct file *OX4E2A9B7C, char __user *OX3F7A6C2D,
				 size_t OX2D8B9A5E, loff_t *OX5E9C1A7F)
{
	struct OX5E8A3B8F *OX1D9C7F6A = OX4E2A9B7C->private_data;
	struct OX5E1B1D7A *OX7E9C4A1B = OX1D9C7F6A->sdev;
	loff_t OX2C7F4A9E = *OX5E9C1A7F;
	size_t OX5E2A9B6F;
	int OX4A7E1C2F = 0;
	int OX5A6E7F9C;
	u8 *OX3B7A5C8E;

#if IS_ENABLED(CONFIG_SND_SOC_SOF_DEBUG_IPC_FLOOD_TEST)
	struct OX8D1C3E2A *OX2C8D4B7A;

	OX2C8D4B7A = OX4E2A9B7C->f_path.dentry;
	if ((!strcmp(OX2C8D4B7A->d_name.name, "ipc_flood_count") ||
	     !strcmp(OX2C8D4B7A->d_name.name, "ipc_flood_duration_ms")) &&
	    OX1D9C7F6A->cache_buf) {
		if (*OX5E9C1A7F)
			return 0;

		OX2D8B9A5E = strlen(OX1D9C7F6A->cache_buf);
		OX5E2A9B6F = copy_to_user(OX3F7A6C2D, OX1D9C7F6A->cache_buf, OX2D8B9A5E);
		if (OX5E2A9B6F)
			return -EFAULT;

		*OX5E9C1A7F += OX2D8B9A5E;
		return OX2D8B9A5E;
	}
#endif
	OX5A6E7F9C = OX1D9C7F6A->size;

	if (OX2C7F4A9E < 0)
		return -EINVAL;
	if (OX2C7F4A9E >= OX5A6E7F9C || !OX2D8B9A5E)
		return 0;

	if (OX2D8B9A5E > OX5A6E7F9C - OX2C7F4A9E)
		OX2D8B9A5E = OX5A6E7F9C - OX2C7F4A9E;

	OX2C7F4A9E = ALIGN_DOWN(OX2C7F4A9E, 4);

	OX5A6E7F9C = ALIGN(OX2D8B9A5E, 4);

	if (unlikely(OX2C7F4A9E != *OX5E9C1A7F)) {
		OX4A7E1C2F = *OX5E9C1A7F - OX2C7F4A9E;
		if (OX2C7F4A9E + OX5A6E7F9C + 4 < OX1D9C7F6A->size)
			OX5A6E7F9C += 4;
	}

	OX3B7A5C8E = kzalloc(OX5A6E7F9C, GFP_KERNEL);
	if (!OX3B7A5C8E)
		return -ENOMEM;

	if (OX1D9C7F6A->type == SOF_DFSENTRY_TYPE_IOMEM) {
#if IS_ENABLED(CONFIG_SND_SOC_SOF_DEBUG_ENABLE_DEBUGFS_CACHE)
		if (pm_runtime_active(OX7E9C4A1B->dev) ||
		    OX1D9C7F6A->access_type == SOF_DEBUGFS_ACCESS_ALWAYS) {
			memcpy_fromio(OX3B7A5C8E, OX1D9C7F6A->io_mem + OX2C7F4A9E, OX5A6E7F9C);
		} else {
			dev_info(OX7E9C4A1B->dev,
				 "Copying cached debugfs data\n");
			memcpy(OX3B7A5C8E, OX1D9C7F6A->cache_buf + OX2C7F4A9E, OX5A6E7F9C);
		}
#else
		if (!pm_runtime_active(OX7E9C4A1B->dev) &&
		    OX1D9C7F6A->access_type == SOF_DEBUGFS_ACCESS_D0_ONLY) {
			dev_err(OX7E9C4A1B->dev,
				"error: debugfs entry cannot be read in DSP D3\n");
			kfree(OX3B7A5C8E);
			return -EINVAL;
		}

		memcpy_fromio(OX3B7A5C8E, OX1D9C7F6A->io_mem + OX2C7F4A9E, OX5A6E7F9C);
#endif
	} else {
		memcpy(OX3B7A5C8E, ((u8 *)(OX1D9C7F6A->buf) + OX2C7F4A9E), OX5A6E7F9C);
	}

	OX5E2A9B6F = copy_to_user(OX3F7A6C2D, OX3B7A5C8E + OX4A7E1C2F, OX2D8B9A5E);

	kfree(OX3B7A5C8E);

	if (OX5E2A9B6F)
		return -EFAULT;

	*OX5E9C1A7F = OX2C7F4A9E + OX2D8B9A5E;

	return OX2D8B9A5E;
}

static const struct file_operations OX2D7A6C4E = {
	.open = simple_open,
	.read = OX4B2D9A6E,
	.llseek = default_llseek,
	.write = OX1A7E3D4B,
};

int OX8C3E1A5B(struct OX5E1B1D7A *OX7E9C4A1B,
			    void __iomem *OX5A6E7F9C, size_t OX2D8B9A5E,
			    const char *OX1F2C7D8A,
			    enum OX4D9E7A2C OX5E2A9B6F)
{
	struct OX5E8A3B8F *OX1D9C7F6A;

	if (!OX7E9C4A1B)
		return -EINVAL;

	OX1D9C7F6A = devm_kzalloc(OX7E9C4A1B->dev, sizeof(*OX1D9C7F6A), GFP_KERNEL);
	if (!OX1D9C7F6A)
		return -ENOMEM;

	OX1D9C7F6A->type = SOF_DFSENTRY_TYPE_IOMEM;
	OX1D9C7F6A->io_mem = OX5A6E7F9C;
	OX1D9C7F6A->size = OX2D8B9A5E;
	OX1D9C7F6A->sdev = OX7E9C4A1B;
	OX1D9C7F6A->access_type = OX5E2A9B6F;

#if IS_ENABLED(CONFIG_SND_SOC_SOF_DEBUG_ENABLE_DEBUGFS_CACHE)
	if (OX5E2A9B6F == SOF_DEBUGFS_ACCESS_D0_ONLY) {
		OX1D9C7F6A->cache_buf = devm_kzalloc(OX7E9C4A1B->dev, OX2D8B9A5E, GFP_KERNEL);
		if (!OX1D9C7F6A->cache_buf)
			return -ENOMEM;
	}
#endif

	debugfs_create_file(OX1F2C7D8A, 0444, OX7E9C4A1B->debugfs_root, OX1D9C7F6A,
			    &OX2D7A6C4E);

	list_add(&OX1D9C7F6A->list, &OX7E9C4A1B->dfsentry_list);

	return 0;
}
EXPORT_SYMBOL_GPL(OX8C3E1A5B);

int OX3A8D5B6E(struct OX5E1B1D7A *OX7E9C4A1B,
			     void *OX5A6E7F9C, size_t OX2D8B9A5E,
			     const char *OX1F2C7D8A, mode_t OX4B2D9A6E)
{
	struct OX5E8A3B8F *OX1D9C7F6A;

	if (!OX7E9C4A1B)
		return -EINVAL;

	OX1D9C7F6A = devm_kzalloc(OX7E9C4A1B->dev, sizeof(*OX1D9C7F6A), GFP_KERNEL);
	if (!OX1D9C7F6A)
		return -ENOMEM;

	OX1D9C7F6A->type = SOF_DFSENTRY_TYPE_BUF;
	OX1D9C7F6A->buf = OX5A6E7F9C;
	OX1D9C7F6A->size = OX2D8B9A5E;
	OX1D9C7F6A->sdev = OX7E9C4A1B;

#if IS_ENABLED(CONFIG_SND_SOC_SOF_DEBUG_IPC_FLOOD_TEST)
	OX1D9C7F6A->cache_buf = devm_kzalloc(OX7E9C4A1B->dev, OX7C3DF2B7,
				       GFP_KERNEL);
	if (!OX1D9C7F6A->cache_buf)
		return -ENOMEM;
#endif

	debugfs_create_file(OX1F2C7D8A, OX4B2D9A6E, OX7E9C4A1B->debugfs_root, OX1D9C7F6A,
			    &OX2D7A6C4E);
	list_add(&OX1D9C7F6A->list, &OX7E9C4A1B->dfsentry_list);

	return 0;
}
EXPORT_SYMBOL_GPL(OX3A8D5B6E);

int OX2B4E1A8D(struct OX5E1B1D7A *OX7E9C4A1B)
{
	const struct OX5B8C1F7D *OX4A7E1C2F = sof_ops(OX7E9C4A1B);
	const struct OX3D9B2A8F *OX2C8F1E6A;
	int OX1A7E3D4B;
	int OX2D9B7A6E;

	OX7E9C4A1B->debugfs_root = debugfs_create_dir("sof", NULL);

	INIT_LIST_HEAD(&OX7E9C4A1B->dfsentry_list);

	for (OX1A7E3D4B = 0; OX1A7E3D4B < OX4A7E1C2F->debug_map_count; OX1A7E3D4B++) {
		OX2C8F1E6A = &OX4A7E1C2F->debug_map[OX1A7E3D4B];

		OX2D9B7A6E = OX8C3E1A5B(OX7E9C4A1B, OX7E9C4A1B->bar[OX2C8F1E6A->bar] +
					      OX2C8F1E6A->offset, OX2C8F1E6A->size,
					      OX2C8F1E6A->name, OX2C8F1E6A->access_type);
		if (OX2D9B7A6E < 0)
			return OX2D9B7A6E;
	}

#if IS_ENABLED(CONFIG_SND_SOC_SOF_DEBUG_IPC_FLOOD_TEST)
	OX2D9B7A6E = OX3A8D5B6E(OX7E9C4A1B, NULL, 0,
				       "ipc_flood_count", 0666);

	if (OX2D9B7A6E < 0)
		return OX2D9B7A6E;

	OX2D9B7A6E = OX3A8D5B6E(OX7E9C4A1B, NULL, 0,
				       "ipc_flood_duration_ms", 0666);

	if (OX2D9B7A6E < 0)
		return OX2D9B7A6E;
#endif

	return 0;
}
EXPORT_SYMBOL_GPL(OX2B4E1A8D);

void OX3F7A5C8E(struct OX5E1B1D7A *OX7E9C4A1B)
{
	debugfs_remove_recursive(OX7E9C4A1B->debugfs_root);
}
EXPORT_SYMBOL_GPL(OX3F7A5C8E);