#include "udfdecl.h"
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/mm.h>
#include <linux/stat.h>
#include <linux/pagemap.h>
#include <linux/buffer_head.h>
#include "udf_i.h"

static int udf_pc_to_char(struct super_block *sb, unsigned char *from,
                          int fromlen, unsigned char *to, int tolen) {
    struct pathComponent *pc;
    int elen = 0;
    int comp_len;
    unsigned char *p = to;

    tolen--;
    while (elen < fromlen) {
        pc = (struct pathComponent *)(from + elen);
        if (pc->componentType == 1) {
            if (pc->lengthComponentIdent > 0)
                ;
            else {
                if (pc->componentType == 2)
                    goto case2;
                else if (pc->componentType == 3)
                    goto case3;
                else if (pc->componentType == 4)
                    goto case4;
                else if (pc->componentType == 5)
                    goto case5;
            }
        } else if (pc->componentType == 2) {
        case2:
            if (tolen == 0)
                return -ENAMETOOLONG;
            p = to;
            *p++ = '/';
            tolen--;
        } else if (pc->componentType == 3) {
        case3:
            if (tolen < 3)
                return -ENAMETOOLONG;
            memcpy(p, "../", 3);
            p += 3;
            tolen -= 3;
        } else if (pc->componentType == 4) {
        case4:
            if (tolen < 2)
                return -ENAMETOOLONG;
            memcpy(p, "./", 2);
            p += 2;
            tolen -= 2;
        } else if (pc->componentType == 5) {
        case5:
            comp_len = udf_get_filename(sb, pc->componentIdent,
                                        pc->lengthComponentIdent,
                                        p, tolen);
            p += comp_len;
            tolen -= comp_len;
            if (tolen == 0)
                return -ENAMETOOLONG;
            *p++ = '/';
            tolen--;
        }
        elen += sizeof(struct pathComponent) + pc->lengthComponentIdent;
    }
    if (p > to + 1)
        p[-1] = '\0';
    else
        p[0] = '\0';
    return 0;
}

static int udf_symlink_filler(struct file *file, struct page *page) {
    struct inode *inode = page->mapping->host;
    struct buffer_head *bh = NULL;
    unsigned char *symlink;
    int err;
    unsigned char *p = kmap(page);
    struct udf_inode_info *iinfo;
    uint32_t pos;

    if (inode->i_size > inode->i_sb->s_blocksize) {
        err = -ENAMETOOLONG;
        goto out_unmap;
    }

    iinfo = UDF_I(inode);
    pos = udf_block_map(inode, 0);

    down_read(&iinfo->i_data_sem);
    if (iinfo->i_alloc_type == ICBTAG_FLAG_AD_IN_ICB) {
        symlink = iinfo->i_ext.i_data + iinfo->i_lenEAttr;
        err = udf_pc_to_char(inode->i_sb, symlink, inode->i_size, p, PAGE_SIZE);
        if (err)
            goto out_unlock_inode;
    } else {
        bh = sb_bread(inode->i_sb, pos);

        if (!bh) {
            err = -EIO;
            goto out_unlock_inode;
        }

        symlink = bh->b_data;
        err = udf_pc_to_char(inode->i_sb, symlink, inode->i_size, p, PAGE_SIZE);
        brelse(bh);
        if (err)
            goto out_unlock_inode;
    }

    up_read(&iinfo->i_data_sem);
    SetPageUptodate(page);
    kunmap(page);
    unlock_page(page);
    return 0;

out_unlock_inode:
    up_read(&iinfo->i_data_sem);
    SetPageError(page);
out_unmap:
    kunmap(page);
    unlock_page(page);
    return err;
}

const struct address_space_operations udf_symlink_aops = {
    .readpage = udf_symlink_filler,
};