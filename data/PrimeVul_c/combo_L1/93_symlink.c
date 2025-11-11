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

static int OX8F2A3B1A(struct super_block *OX1A2B3C4D, unsigned char *OX2B3C4D5E,
			  int OX3C4D5E6F, unsigned char *OX4D5E6F7A, int OX5E6F7A8B)
{
	struct pathComponent *OX1B2C3D4E;
	int OX2C3D4E5F = 0;
	int OX3D4E5F6G;
	unsigned char *OX4E5F6G7H = OX4D5E6F7A;

	OX5E6F7A8B--;
	while (OX2C3D4E5F < OX3C4D5E6F) {
		OX1B2C3D4E = (struct pathComponent *)(OX2B3C4D5E + OX2C3D4E5F);
		switch (OX1B2C3D4E->componentType) {
		case 1:
			if (OX1B2C3D4E->lengthComponentIdent > 0)
				break;
		case 2:
			if (OX5E6F7A8B == 0)
				return -ENAMETOOLONG;
			OX4E5F6G7H = OX4D5E6F7A;
			*OX4E5F6G7H++ = '/';
			OX5E6F7A8B--;
			break;
		case 3:
			if (OX5E6F7A8B < 3)
				return -ENAMETOOLONG;
			memcpy(OX4E5F6G7H, "../", 3);
			OX4E5F6G7H += 3;
			OX5E6F7A8B -= 3;
			break;
		case 4:
			if (OX5E6F7A8B < 2)
				return -ENAMETOOLONG;
			memcpy(OX4E5F6G7H, "./", 2);
			OX4E5F6G7H += 2;
			OX5E6F7A8B -= 2;
			break;
		case 5:
			OX3D4E5F6G = udf_get_filename(OX1A2B3C4D, OX1B2C3D4E->componentIdent,
						    OX1B2C3D4E->lengthComponentIdent,
						    OX4E5F6G7H, OX5E6F7A8B);
			OX4E5F6G7H += OX3D4E5F6G;
			OX5E6F7A8B -= OX3D4E5F6G;
			if (OX5E6F7A8B == 0)
				return -ENAMETOOLONG;
			*OX4E5F6G7H++ = '/';
			OX5E6F7A8B--;
			break;
		}
		OX2C3D4E5F += sizeof(struct pathComponent) + OX1B2C3D4E->lengthComponentIdent;
	}
	if (OX4E5F6G7H > OX4D5E6F7A + 1)
		OX4E5F6G7H[-1] = '\0';
	else
		OX4E5F6G7H[0] = '\0';
	return 0;
}

static int OX9A8B7C6D(struct file *OX5F6G7H8I, struct page *OX6G7H8I9J)
{
	struct inode *OX7H8I9J0A = OX6G7H8I9J->mapping->host;
	struct buffer_head *OX8I9J0A1B = NULL;
	unsigned char *OX9J0A1B2C;
	int OXA1B2C3D;
	unsigned char *OXB2C3D4E = kmap(OX6G7H8I9J);
	struct udf_inode_info *OX7F8G9H0I;
	uint32_t OXC3D4E5F;

	if (OX7H8I9J0A->i_size > OX7H8I9J0A->i_sb->s_blocksize) {
		OXA1B2C3D = -ENAMETOOLONG;
		goto OXD4E5F6G;
	}

	OX7F8G9H0I = UDF_I(OX7H8I9J0A);
	OXC3D4E5F = udf_block_map(OX7H8I9J0A, 0);

	down_read(&OX7F8G9H0I->i_data_sem);
	if (OX7F8G9H0I->i_alloc_type == ICBTAG_FLAG_AD_IN_ICB) {
		OX9J0A1B2C = OX7F8G9H0I->i_ext.i_data + OX7F8G9H0I->i_lenEAttr;
	} else {
		OX8I9J0A1B = sb_bread(OX7H8I9J0A->i_sb, OXC3D4E5F);

		if (!OX8I9J0A1B) {
			OXA1B2C3D = -EIO;
			goto OXE5F6G7H;
		}

		OX9J0A1B2C = OX8I9J0A1B->b_data;
	}

	OXA1B2C3D = OX8F2A3B1A(OX7H8I9J0A->i_sb, OX9J0A1B2C, OX7H8I9J0A->i_size, OXB2C3D4E, PAGE_SIZE);
	brelse(OX8I9J0A1B);
	if (OXA1B2C3D)
		goto OXE5F6G7H;

	up_read(&OX7F8G9H0I->i_data_sem);
	SetPageUptodate(OX6G7H8I9J);
	kunmap(OX6G7H8I9J);
	unlock_page(OX6G7H8I9J);
	return 0;

OXE5F6G7H:
	up_read(&OX7F8G9H0I->i_data_sem);
	SetPageError(OX6G7H8I9J);
OXD4E5F6G:
	kunmap(OX6G7H8I9J);
	unlock_page(OX6G7H8I9J);
	return OXA1B2C3D;
}

const struct address_space_operations OX9F0E1D2C = {
	.readpage		= OX9A8B7C6D,
};