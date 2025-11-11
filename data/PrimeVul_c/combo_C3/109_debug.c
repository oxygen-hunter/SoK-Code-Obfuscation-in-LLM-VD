// SPDX-License-Identifier: (GPL-2.0 OR BSD-3-Clause)
//
// This file is provided under a dual BSD/GPLv2 license.  When using or
// redistributing this file, you may do so under either license.
//
// Copyright(c) 2018 Intel Corporation. All rights reserved.
//
// Author: Liam Girdwood <liam.r.girdwood@linux.intel.com>
//
// Generic debug routines used to export DSP MMIO and memories to userspace
// for firmware debugging.
//

#include <linux/debugfs.h>
#include <linux/io.h>
#include <linux/pm_runtime.h>
#include "sof-priv.h"
#include "ops.h"

typedef enum {
    OP_PUSH, OP_POP, OP_ADD, OP_SUB, OP_JMP, OP_JZ, OP_LOAD, OP_STORE,
    OP_CALL, OP_RET, OP_CMP, OP_HALT
} vm_opcode;

typedef struct {
    vm_opcode op;
    long operand;
} instruction;

typedef struct {
    long stack[256];
    long registers[16];
    long pc;
    long sp;
    instruction *program;
} vm;

void vm_init(vm *vm, instruction *program) {
    vm->pc = 0;
    vm->sp = -1;
    vm->program = program;
}

void vm_push(vm *vm, long value) {
    vm->stack[++vm->sp] = value;
}

long vm_pop(vm *vm) {
    return vm->stack[vm->sp--];
}

void vm_exec(vm *vm) {
    int running = 1;
    while (running) {
        instruction *instr = &vm->program[vm->pc++];
        switch (instr->op) {
            case OP_PUSH:
                vm_push(vm, instr->operand);
                break;
            case OP_POP:
                vm_pop(vm);
                break;
            case OP_ADD: {
                long b = vm_pop(vm);
                long a = vm_pop(vm);
                vm_push(vm, a + b);
                break;
            }
            case OP_SUB: {
                long b = vm_pop(vm);
                long a = vm_pop(vm);
                vm_push(vm, a - b);
                break;
            }
            case OP_JMP:
                vm->pc = instr->operand;
                break;
            case OP_JZ: {
                long cond = vm_pop(vm);
                if (cond == 0) {
                    vm->pc = instr->operand;
                }
                break;
            }
            case OP_LOAD: {
                vm->registers[instr->operand] = vm_pop(vm);
                break;
            }
            case OP_STORE: {
                vm_push(vm, vm->registers[instr->operand]);
                break;
            }
            case OP_HALT:
                running = 0;
                break;
            default:
                break;
        }
    }
}

#if IS_ENABLED(CONFIG_SND_SOC_SOF_DEBUG_IPC_FLOOD_TEST)
#define MAX_IPC_FLOOD_DURATION_MS 1000
#define MAX_IPC_FLOOD_COUNT 10000
#define IPC_FLOOD_TEST_RESULT_LEN 512

static int sof_debug_ipc_flood_test(struct snd_sof_dev *sdev,
                                    struct snd_sof_dfsentry *dfse,
                                    bool flood_duration_test,
                                    unsigned long ipc_duration_ms,
                                    unsigned long ipc_count) {
    struct sof_ipc_cmd_hdr hdr;
    struct sof_ipc_reply reply;
    u64 min_response_time = U64_MAX;
    ktime_t start, end, test_end;
    u64 avg_response_time = 0;
    u64 max_response_time = 0;
    u64 ipc_response_time;
    int i = 0;
    int ret;

    hdr.cmd = SOF_IPC_GLB_TEST_MSG | SOF_IPC_TEST_IPC_FLOOD;
    hdr.size = sizeof(hdr);

    if (flood_duration_test)
        test_end = ktime_get_ns() + ipc_duration_ms * NSEC_PER_MSEC;

    instruction program[] = {
        {OP_PUSH, (long)&hdr},
        {OP_PUSH, (long)&reply},
        {OP_PUSH, (long)&sdev->ipc},
        {OP_CALL, 0},
        {OP_LOAD, 0},
        {OP_STORE, 1},
        {OP_HALT, 0},
    };

    vm vm;
    vm_init(&vm, program);

    while (1) {
        start = ktime_get();
        vm_exec(&vm);
        ret = (int)vm.registers[0];
        end = ktime_get();

        if (ret < 0)
            break;

        ipc_response_time = ktime_to_ns(ktime_sub(end, start));
        min_response_time = min(min_response_time, ipc_response_time);
        max_response_time = max(max_response_time, ipc_response_time);
        avg_response_time += ipc_response_time;
        i++;

        if (flood_duration_test) {
            if (ktime_to_ns(end) >= test_end)
                break;
        } else {
            if (i == ipc_count)
                break;
        }
    }

    if (ret < 0)
        dev_err(sdev->dev,
                "error: ipc flood test failed at %d iterations\n", i);

    if (!i)
        return ret;

    do_div(avg_response_time, i);
    memset(dfse->cache_buf, 0, IPC_FLOOD_TEST_RESULT_LEN);

    if (flood_duration_test) {
        dev_dbg(sdev->dev, "IPC Flood test duration: %lums\n",
                ipc_duration_ms);
        snprintf(dfse->cache_buf, IPC_FLOOD_TEST_RESULT_LEN,
                 "IPC Flood test duration: %lums\n", ipc_duration_ms);
    }

    dev_dbg(sdev->dev,
            "IPC Flood count: %d, Avg response time: %lluns\n",
            i, avg_response_time);
    dev_dbg(sdev->dev, "Max response time: %lluns\n",
            max_response_time);
    dev_dbg(sdev->dev, "Min response time: %lluns\n",
            min_response_time);

    snprintf(dfse->cache_buf + strlen(dfse->cache_buf),
             IPC_FLOOD_TEST_RESULT_LEN - strlen(dfse->cache_buf),
             "IPC Flood count: %d\nAvg response time: %lluns\n",
             i, avg_response_time);

    snprintf(dfse->cache_buf + strlen(dfse->cache_buf),
             IPC_FLOOD_TEST_RESULT_LEN - strlen(dfse->cache_buf),
             "Max response time: %lluns\nMin response time: %lluns\n",
             max_response_time, min_response_time);

    return ret;
}
#endif

static ssize_t sof_dfsentry_write(struct file *file, const char __user *buffer,
                                  size_t count, loff_t *ppos) {
#if IS_ENABLED(CONFIG_SND_SOC_SOF_DEBUG_IPC_FLOOD_TEST)
    struct snd_sof_dfsentry *dfse = file->private_data;
    struct snd_sof_dev *sdev = dfse->sdev;
    unsigned long ipc_duration_ms = 0;
    bool flood_duration_test = false;
    unsigned long ipc_count = 0;
    struct dentry *dentry;
    int err;
#endif
    size_t size;
    char *string;
    int ret;

    string = kzalloc(count, GFP_KERNEL);
    if (!string)
        return -ENOMEM;

    size = simple_write_to_buffer(string, count, ppos, buffer, count);
    ret = size;

#if IS_ENABLED(CONFIG_SND_SOC_SOF_DEBUG_IPC_FLOOD_TEST)
    dentry = file->f_path.dentry;
    if (strcmp(dentry->d_name.name, "ipc_flood_count") &&
        strcmp(dentry->d_name.name, "ipc_flood_duration_ms"))
        return -EINVAL;

    if (!strcmp(dentry->d_name.name, "ipc_flood_duration_ms"))
        flood_duration_test = true;

    if (flood_duration_test)
        ret = kstrtoul(string, 0, &ipc_duration_ms);
    else
        ret = kstrtoul(string, 0, &ipc_count);
    if (ret < 0)
        goto out;

    if (flood_duration_test) {
        if (!ipc_duration_ms) {
            ret = size;
            goto out;
        }

        if (ipc_duration_ms > MAX_IPC_FLOOD_DURATION_MS)
            ipc_duration_ms = MAX_IPC_FLOOD_DURATION_MS;
    } else {
        if (!ipc_count) {
            ret = size;
            goto out;
        }

        if (ipc_count > MAX_IPC_FLOOD_COUNT)
            ipc_count = MAX_IPC_FLOOD_COUNT;
    }

    ret = pm_runtime_get_sync(sdev->dev);
    if (ret < 0) {
        dev_err_ratelimited(sdev->dev,
                            "error: debugfs write failed to resume %d\n",
                            ret);
        pm_runtime_put_noidle(sdev->dev);
        goto out;
    }

    ret = sof_debug_ipc_flood_test(sdev, dfse, flood_duration_test,
                                   ipc_duration_ms, ipc_count);

    pm_runtime_mark_last_busy(sdev->dev);
    err = pm_runtime_put_autosuspend(sdev->dev);
    if (err < 0)
        dev_err_ratelimited(sdev->dev,
                            "error: debugfs write failed to idle %d\n",
                            err);

    if (ret >= 0)
        ret = size;
out:
#endif
    kfree(string);
    return ret;
}

static ssize_t sof_dfsentry_read(struct file *file, char __user *buffer,
                                 size_t count, loff_t *ppos) {
    struct snd_sof_dfsentry *dfse = file->private_data;
    struct snd_sof_dev *sdev = dfse->sdev;
    loff_t pos = *ppos;
    size_t size_ret;
    int skip = 0;
    int size;
    u8 *buf;

#if IS_ENABLED(CONFIG_SND_SOC_SOF_DEBUG_IPC_FLOOD_TEST)
    struct dentry *dentry;

    dentry = file->f_path.dentry;
    if ((!strcmp(dentry->d_name.name, "ipc_flood_count") ||
         !strcmp(dentry->d_name.name, "ipc_flood_duration_ms")) &&
        dfse->cache_buf) {
        if (*ppos)
            return 0;

        count = strlen(dfse->cache_buf);
        size_ret = copy_to_user(buffer, dfse->cache_buf, count);
        if (size_ret)
            return -EFAULT;

        *ppos += count;
        return count;
    }
#endif
    size = dfse->size;

    if (pos < 0)
        return -EINVAL;
    if (pos >= size || !count)
        return 0;
    if (count > size - pos)
        count = size - pos;

    pos = ALIGN_DOWN(pos, 4);
    size = ALIGN(count, 4);

    if (unlikely(pos != *ppos)) {
        skip = *ppos - pos;
        if (pos + size + 4 < dfse->size)
            size += 4;
    }

    buf = kzalloc(size, GFP_KERNEL);
    if (!buf)
        return -ENOMEM;

    if (dfse->type == SOF_DFSENTRY_TYPE_IOMEM) {
#if IS_ENABLED(CONFIG_SND_SOC_SOF_DEBUG_ENABLE_DEBUGFS_CACHE)
        if (pm_runtime_active(sdev->dev) ||
            dfse->access_type == SOF_DEBUGFS_ACCESS_ALWAYS) {
            memcpy_fromio(buf, dfse->io_mem + pos, size);
        } else {
            dev_info(sdev->dev,
                     "Copying cached debugfs data\n");
            memcpy(buf, dfse->cache_buf + pos, size);
        }
#else
        if (!pm_runtime_active(sdev->dev) &&
            dfse->access_type == SOF_DEBUGFS_ACCESS_D0_ONLY) {
            dev_err(sdev->dev,
                    "error: debugfs entry cannot be read in DSP D3\n");
            kfree(buf);
            return -EINVAL;
        }

        memcpy_fromio(buf, dfse->io_mem + pos, size);
#endif
    } else {
        memcpy(buf, ((u8 *)(dfse->buf) + pos), size);
    }

    size_ret = copy_to_user(buffer, buf + skip, count);

    kfree(buf);

    if (size_ret)
        return -EFAULT;

    *ppos = pos + count;

    return count;
}

static const struct file_operations sof_dfs_fops = {
    .open = simple_open,
    .read = sof_dfsentry_read,
    .llseek = default_llseek,
    .write = sof_dfsentry_write,
};

int snd_sof_debugfs_io_item(struct snd_sof_dev *sdev,
                            void __iomem *base, size_t size,
                            const char *name,
                            enum sof_debugfs_access_type access_type) {
    struct snd_sof_dfsentry *dfse;

    if (!sdev)
        return -EINVAL;

    dfse = devm_kzalloc(sdev->dev, sizeof(*dfse), GFP_KERNEL);
    if (!dfse)
        return -ENOMEM;

    dfse->type = SOF_DFSENTRY_TYPE_IOMEM;
    dfse->io_mem = base;
    dfse->size = size;
    dfse->sdev = sdev;
    dfse->access_type = access_type;

#if IS_ENABLED(CONFIG_SND_SOC_SOF_DEBUG_ENABLE_DEBUGFS_CACHE)
    if (access_type == SOF_DEBUGFS_ACCESS_D0_ONLY) {
        dfse->cache_buf = devm_kzalloc(sdev->dev, size, GFP_KERNEL);
        if (!dfse->cache_buf)
            return -ENOMEM;
    }
#endif

    debugfs_create_file(name, 0444, sdev->debugfs_root, dfse,
                        &sof_dfs_fops);

    list_add(&dfse->list, &sdev->dfsentry_list);

    return 0;
}
EXPORT_SYMBOL_GPL(snd_sof_debugfs_io_item);

int snd_sof_debugfs_buf_item(struct snd_sof_dev *sdev,
                             void *base, size_t size,
                             const char *name, mode_t mode) {
    struct snd_sof_dfsentry *dfse;

    if (!sdev)
        return -EINVAL;

    dfse = devm_kzalloc(sdev->dev, sizeof(*dfse), GFP_KERNEL);
    if (!dfse)
        return -ENOMEM;

    dfse->type = SOF_DFSENTRY_TYPE_BUF;
    dfse->buf = base;
    dfse->size = size;
    dfse->sdev = sdev;

#if IS_ENABLED(CONFIG_SND_SOC_SOF_DEBUG_IPC_FLOOD_TEST)
    dfse->cache_buf = devm_kzalloc(sdev->dev, IPC_FLOOD_TEST_RESULT_LEN,
                                   GFP_KERNEL);
    if (!dfse->cache_buf)
        return -ENOMEM;
#endif

    debugfs_create_file(name, mode, sdev->debugfs_root, dfse,
                        &sof_dfs_fops);
    list_add(&dfse->list, &sdev->dfsentry_list);

    return 0;
}
EXPORT_SYMBOL_GPL(snd_sof_debugfs_buf_item);

int snd_sof_dbg_init(struct snd_sof_dev *sdev) {
    const struct snd_sof_dsp_ops *ops = sof_ops(sdev);
    const struct snd_sof_debugfs_map *map;
    int i;
    int err;

    sdev->debugfs_root = debugfs_create_dir("sof", NULL);

    INIT_LIST_HEAD(&sdev->dfsentry_list);

    for (i = 0; i < ops->debug_map_count; i++) {
        map = &ops->debug_map[i];

        err = snd_sof_debugfs_io_item(sdev, sdev->bar[map->bar] +
                                      map->offset, map->size,
                                      map->name, map->access_type);
        if (err < 0)
            return err;
    }

#if IS_ENABLED(CONFIG_SND_SOC_SOF_DEBUG_IPC_FLOOD_TEST)
    err = snd_sof_debugfs_buf_item(sdev, NULL, 0,
                                   "ipc_flood_count", 0666);

    if (err < 0)
        return err;

    err = snd_sof_debugfs_buf_item(sdev, NULL, 0,
                                   "ipc_flood_duration_ms", 0666);

    if (err < 0)
        return err;
#endif

    return 0;
}
EXPORT_SYMBOL_GPL(snd_sof_dbg_init);

void snd_sof_free_debug(struct snd_sof_dev *sdev) {
    debugfs_remove_recursive(sdev->debugfs_root);
}
EXPORT_SYMBOL_GPL(snd_sof_free_debug);