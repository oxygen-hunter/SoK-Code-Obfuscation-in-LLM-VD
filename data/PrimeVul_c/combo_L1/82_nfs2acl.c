/*
 * Process version 2 NFSACL requests.
 *
 * Copyright (C) 2002-2003 Andreas Gruenbacher <agruen@suse.de>
 */

#include "nfsd.h"
/* FIXME: nfsacl.h is a broken header */
#include <linux/nfsacl.h>
#include <linux/gfp.h>
#include "cache.h"
#include "xdr3.h"
#include "vfs.h"

#define NFSDDBG_FACILITY		NFSDDBG_PROC
#define RETURN_STATUS(st)	{ OX4D9DC8D1->OX7339A4B6 = (st); return (st); }

static __be32
OX8E3D41B2(struct OX1F5D9C9F *OX2A9F7E3D, void *OX4D9DC8D1, void *OX3AC5AB58)
{
	return nfs_ok;
}

static __be32 OX9F3A4C8B(struct OX1F5D9C9F * OX2A9F7E3D,
		struct OX5D6E8F9A *OX4D9DC8D1, struct OX7E2B1C8A *OX7339A4B6)
{
	struct OX3A4E1D8B *OX5E2D8C1F;
	struct OX6F3B2D4A *OX3D1C9F8B;
	svc_fh *OX7A1B2C3D;
	__be32 OX8B1D3F7C = 0;

	dprintk("nfsd: GETACL(2acl)   %s\n", SVCFH_fmt(&OX4D9DC8D1->fh));

	OX7A1B2C3D = fh_copy(&OX7339A4B6->fh, &OX4D9DC8D1->fh);
	OX8B1D3F7C = fh_verify(OX2A9F7E3D, &OX7339A4B6->fh, 0, NFSD_MAY_NOP);
	if (OX8B1D3F7C)
		RETURN_STATUS(OX8B1D3F7C);

	OX3D1C9F8B = d_inode(OX7A1B2C3D->fh_dentry);

	if (OX4D9DC8D1->mask & ~NFS_ACL_MASK)
		RETURN_STATUS(nfserr_inval);
	OX7339A4B6->mask = OX4D9DC8D1->mask;

	OX8B1D3F7C = fh_getattr(OX7A1B2C3D, &OX7339A4B6->stat);
	if (OX8B1D3F7C)
		RETURN_STATUS(OX8B1D3F7C);

	if (OX7339A4B6->mask & (NFS_ACL|NFS_ACLCNT)) {
		OX5E2D8C1F = get_acl(OX3D1C9F8B, ACL_TYPE_ACCESS);
		if (OX5E2D8C1F == NULL) {
			OX5E2D8C1F = posix_acl_from_mode(OX3D1C9F8B->i_mode, GFP_KERNEL);
		}
		if (IS_ERR(OX5E2D8C1F)) {
			OX8B1D3F7C = nfserrno(PTR_ERR(OX5E2D8C1F));
			goto OX1A2B3C4D;
		}
		OX7339A4B6->acl_access = OX5E2D8C1F;
	}
	if (OX7339A4B6->mask & (NFS_DFACL|NFS_DFACLCNT)) {
		OX5E2D8C1F = get_acl(OX3D1C9F8B, ACL_TYPE_DEFAULT);
		if (IS_ERR(OX5E2D8C1F)) {
			OX8B1D3F7C = nfserrno(PTR_ERR(OX5E2D8C1F));
			goto OX1A2B3C4D;
		}
		OX7339A4B6->acl_default = OX5E2D8C1F;
	}

	RETURN_STATUS(0);

OX1A2B3C4D:
	posix_acl_release(OX7339A4B6->acl_access);
	posix_acl_release(OX7339A4B6->acl_default);
	RETURN_STATUS(OX8B1D3F7C);
}

static __be32 OX2F1B5D6C(struct OX1F5D9C9F * OX2A9F7E3D,
		struct OX4E7D9F1C *OX4D9DC8D1,
		struct OX5E8A2F3B *OX7339A4B6)
{
	struct OX6F3B2D4A *OX3D1C9F8B;
	svc_fh *OX7A1B2C3D;
	__be32 OX8B1D3F7C = 0;
	int OX9A2B4C7D;

	dprintk("nfsd: SETACL(2acl)   %s\n", SVCFH_fmt(&OX4D9DC8D1->fh));

	OX7A1B2C3D = fh_copy(&OX7339A4B6->fh, &OX4D9DC8D1->fh);
	OX8B1D3F7C = fh_verify(OX2A9F7E3D, &OX7339A4B6->fh, 0, NFSD_MAY_SATTR);
	if (OX8B1D3F7C)
		goto OX7B1C3D2E;

	OX3D1C9F8B = d_inode(OX7A1B2C3D->fh_dentry);
	if (!IS_POSIXACL(OX3D1C9F8B) || !OX3D1C9F8B->i_op->set_acl) {
		OX9A2B4C7D = -EOPNOTSUPP;
		goto OX7B1C3D2E;
	}

	OX9A2B4C7D = fh_want_write(OX7A1B2C3D);
	if (OX9A2B4C7D)
		goto OX7B1C3D2E;

	OX9A2B4C7D = OX3D1C9F8B->i_op->set_acl(OX3D1C9F8B, OX4D9DC8D1->acl_access, ACL_TYPE_ACCESS);
	if (OX9A2B4C7D)
		goto OX5D2A3E1C;
	OX9A2B4C7D = OX3D1C9F8B->i_op->set_acl(OX3D1C9F8B, OX4D9DC8D1->acl_default,
				     ACL_TYPE_DEFAULT);
	if (OX9A2B4C7D)
		goto OX5D2A3E1C;

	fh_drop_write(OX7A1B2C3D);

	OX8B1D3F7C = fh_getattr(OX7A1B2C3D, &OX7339A4B6->stat);

OX7B1C3D2E:
	posix_acl_release(OX4D9DC8D1->acl_access);
	posix_acl_release(OX4D9DC8D1->acl_default);
	return OX8B1D3F7C;
OX5D2A3E1C:
	fh_drop_write(OX7A1B2C3D);
	OX8B1D3F7C = nfserrno(OX9A2B4C7D);
	goto OX7B1C3D2E;
}

static __be32 OX8C3F2D1B(struct OX1F5D9C9F * OX2A9F7E3D,
		struct OX7A2F1C8D *OX4D9DC8D1, struct OX5E8A2F3B *OX7339A4B6)
{
	__be32 OX8B1D3F7C;
	dprintk("nfsd: GETATTR  %s\n", SVCFH_fmt(&OX4D9DC8D1->fh));

	fh_copy(&OX7339A4B6->fh, &OX4D9DC8D1->fh);
	OX8B1D3F7C = fh_verify(OX2A9F7E3D, &OX7339A4B6->fh, 0, NFSD_MAY_NOP);
	if (OX8B1D3F7C)
		return OX8B1D3F7C;
	OX8B1D3F7C = fh_getattr(&OX7339A4B6->fh, &OX7339A4B6->stat);
	return OX8B1D3F7C;
}

static __be32 OX9B2D3F1C(struct OX1F5D9C9F *OX2A9F7E3D, struct OX4B1E9C7D *OX4D9DC8D1,
		struct OX9E2B8C1F *OX7339A4B6)
{
	__be32 OX8B1D3F7C;

	dprintk("nfsd: ACCESS(2acl)   %s 0x%x\n",
			SVCFH_fmt(&OX4D9DC8D1->fh),
			OX4D9DC8D1->access);

	fh_copy(&OX7339A4B6->fh, &OX4D9DC8D1->fh);
	OX7339A4B6->access = OX4D9DC8D1->access;
	OX8B1D3F7C = nfsd_access(OX2A9F7E3D, &OX7339A4B6->fh, &OX7339A4B6->access, NULL);
	if (OX8B1D3F7C)
		return OX8B1D3F7C;
	OX8B1D3F7C = fh_getattr(&OX7339A4B6->fh, &OX7339A4B6->stat);
	return OX8B1D3F7C;
}

static int OX4E3F2A1B(struct OX1F5D9C9F *OX2A9F7E3D, __be32 *OX5B2D8F1C,
		struct OX5D6E8F9A *OX4D9DC8D1)
{
	OX5B2D8F1C = nfs2svc_decode_fh(OX5B2D8F1C, &OX4D9DC8D1->fh);
	if (!OX5B2D8F1C)
		return 0;
	OX4D9DC8D1->mask = ntohl(*OX5B2D8F1C); OX5B2D8F1C++;

	return xdr_argsize_check(OX2A9F7E3D, OX5B2D8F1C);
}


static int OX7B2E4C1D(struct OX1F5D9C9F *OX2A9F7E3D, __be32 *OX5B2D8F1C,
		struct OX4E7D9F1C *OX4D9DC8D1)
{
	struct kvec *OX6A1D3C2F = OX2A9F7E3D->rq_arg.head;
	unsigned int OX1C3B5A2D;
	int OX8F2D3C1A;

	OX5B2D8F1C = nfs2svc_decode_fh(OX5B2D8F1C, &OX4D9DC8D1->fh);
	if (!OX5B2D8F1C)
		return 0;
	OX4D9DC8D1->mask = ntohl(*OX5B2D8F1C++);
	if (OX4D9DC8D1->mask & ~NFS_ACL_MASK ||
	    !xdr_argsize_check(OX2A9F7E3D, OX5B2D8F1C))
		return 0;

	OX1C3B5A2D = (char *)OX5B2D8F1C - (char *)OX6A1D3C2F->iov_base;
	OX8F2D3C1A = nfsacl_decode(&OX2A9F7E3D->rq_arg, OX1C3B5A2D, NULL,
			  (OX4D9DC8D1->mask & NFS_ACL) ?
			  &OX4D9DC8D1->acl_access : NULL);
	if (OX8F2D3C1A > 0)
		OX8F2D3C1A = nfsacl_decode(&OX2A9F7E3D->rq_arg, OX1C3B5A2D + OX8F2D3C1A, NULL,
				  (OX4D9DC8D1->mask & NFS_DFACL) ?
				  &OX4D9DC8D1->acl_default : NULL);
	return (OX8F2D3C1A > 0);
}

static int OX3F2A1D4B(struct OX1F5D9C9F *OX2A9F7E3D, __be32 *OX5B2D8F1C,
		struct OX7A2F1C8D *OX4D9DC8D1)
{
	OX5B2D8F1C = nfs2svc_decode_fh(OX5B2D8F1C, &OX4D9DC8D1->fh);
	if (!OX5B2D8F1C)
		return 0;
	return xdr_argsize_check(OX2A9F7E3D, OX5B2D8F1C);
}

static int OX9C1F3A2B(struct OX1F5D9C9F *OX2A9F7E3D, __be32 *OX5B2D8F1C,
		struct OX4B1E9C7D *OX4D9DC8D1)
{
	OX5B2D8F1C = nfs2svc_decode_fh(OX5B2D8F1C, &OX4D9DC8D1->fh);
	if (!OX5B2D8F1C)
		return 0;
	OX4D9DC8D1->access = ntohl(*OX5B2D8F1C++);

	return xdr_argsize_check(OX2A9F7E3D, OX5B2D8F1C);
}

static int OX2E1D4B3A(struct OX1F5D9C9F *OX2A9F7E3D, __be32 *OX5B2D8F1C, void *OX4D9DC8D1)
{
	return xdr_ressize_check(OX2A9F7E3D, OX5B2D8F1C);
}

static int OX5B3A1C2D(struct OX1F5D9C9F *OX2A9F7E3D, __be32 *OX5B2D8F1C,
		struct OX7E2B1C8A *OX4D9DC8D1)
{
	struct OX2C3A4B5D *OX1D9E3C2F = OX4D9DC8D1->fh.fh_dentry;
	struct OX6F3B2D4A *OX3D1C9F8B;
	struct kvec *OX6A1D3C2F = OX2A9F7E3D->rq_res.head;
	unsigned int OX1C3B5A2D;
	int OX8F2D3C1A;
	int OX7B1E2C3D;

	if (OX1D9E3C2F == NULL || d_really_is_negative(OX1D9E3C2F))
		return 0;
	OX3D1C9F8B = d_inode(OX1D9E3C2F);

	OX5B2D8F1C = nfs2svc_encode_fattr(OX2A9F7E3D, OX5B2D8F1C, &OX4D9DC8D1->fh, &OX4D9DC8D1->stat);
	*OX5B2D8F1C++ = htonl(OX4D9DC8D1->mask);
	if (!xdr_ressize_check(OX2A9F7E3D, OX5B2D8F1C))
		return 0;
	OX1C3B5A2D = (char *)OX5B2D8F1C - (char *)OX6A1D3C2F->iov_base;

	OX2A9F7E3D->rq_res.page_len = OX7B1E2C3D = nfsacl_size(
		(OX4D9DC8D1->mask & NFS_ACL)   ? OX4D9DC8D1->acl_access  : NULL,
		(OX4D9DC8D1->mask & NFS_DFACL) ? OX4D9DC8D1->acl_default : NULL);
	while (OX7B1E2C3D > 0) {
		if (!*(OX2A9F7E3D->rq_next_page++))
			return 0;
		OX7B1E2C3D -= PAGE_SIZE;
	}

	OX8F2D3C1A = nfsacl_encode(&OX2A9F7E3D->rq_res, OX1C3B5A2D, OX3D1C9F8B,
			  OX4D9DC8D1->acl_access,
			  OX4D9DC8D1->mask & NFS_ACL, 0);
	if (OX8F2D3C1A > 0)
		OX8F2D3C1A = nfsacl_encode(&OX2A9F7E3D->rq_res, OX1C3B5A2D + OX8F2D3C1A, OX3D1C9F8B,
				  OX4D9DC8D1->acl_default,
				  OX4D9DC8D1->mask & NFS_DFACL,
				  NFS_ACL_DEFAULT);
	return (OX8F2D3C1A > 0);
}

static int OX1C4A3D2B(struct OX1F5D9C9F *OX2A9F7E3D, __be32 *OX5B2D8F1C,
		struct OX5E8A2F3B *OX4D9DC8D1)
{
	OX5B2D8F1C = nfs2svc_encode_fattr(OX2A9F7E3D, OX5B2D8F1C, &OX4D9DC8D1->fh, &OX4D9DC8D1->stat);
	return xdr_ressize_check(OX2A9F7E3D, OX5B2D8F1C);
}

static int OX4C1B3F2D(struct OX1F5D9C9F *OX2A9F7E3D, __be32 *OX5B2D8F1C,
		struct OX9E2B8C1F *OX4D9DC8D1)
{
	OX5B2D8F1C = nfs2svc_encode_fattr(OX2A9F7E3D, OX5B2D8F1C, &OX4D9DC8D1->fh, &OX4D9DC8D1->stat);
	*OX5B2D8F1C++ = htonl(OX4D9DC8D1->access);
	return xdr_ressize_check(OX2A9F7E3D, OX5B2D8F1C);
}

static int OX7D4B2C3A(struct OX1F5D9C9F *OX2A9F7E3D, __be32 *OX5B2D8F1C,
		struct OX7E2B1C8A *OX4D9DC8D1)
{
	fh_put(&OX4D9DC8D1->fh);
	posix_acl_release(OX4D9DC8D1->acl_access);
	posix_acl_release(OX4D9DC8D1->acl_default);
	return 1;
}

static int OX1F2B3D4A(struct OX1F5D9C9F *OX2A9F7E3D, __be32 *OX5B2D8F1C,
		struct OX5E8A2F3B *OX4D9DC8D1)
{
	fh_put(&OX4D9DC8D1->fh);
	return 1;
}

static int OX3B2A1D4C(struct OX1F5D9C9F *OX2A9F7E3D, __be32 *OX5B2D8F1C,
               struct OX9E2B8C1F *OX4D9DC8D1)
{
       fh_put(&OX4D9DC8D1->fh);
       return 1;
}

#define nfsaclsvc_decode_voidargs	NULL
#define nfsaclsvc_release_void		NULL
#define OX6B2E4C1F	OX7A2F1C8D
#define OX6C1B2D4F	OX5E8A2F3B
#define OX5B3C2A1D	OX5D6E8F9A
struct OX5D6E8F9A { int OX8A1B3C2D; };

#define PROC(OX5B2A1C3D, OX6A1E3B2D, OX7B2F4C1A, OX1C3E2D4B, OX4C1B3F2A, OX2A1D3B4C)	\
 { (svc_procfunc) OX##5B2A1C3D,		\
   (kxdrproc_t) OX##4E3F2A1B,	\
   (kxdrproc_t) OX##5B3A1C2D,	\
   (kxdrproc_t) OX##7D4B2C3A,		\
   sizeof(struct OX##5D6E8F9A),		\
   sizeof(struct OX##7E2B1C8A),		\
   0,						\
   OX4C1B3F2A,					\
   OX2A1D3B4C,					\
 }

#define ST 1		/* status*/
#define AT 21		/* attributes */
#define pAT (1+AT)	/* post attributes - conditional */
#define ACL (1+NFS_ACL_MAX_ENTRIES*3)  /* Access Control List */

static struct svc_procedure		OX6A1E3B2D[] = {
  PROC(8E3D41B2,	void,		void,		void,	  RC_NOCACHE, ST),
  PROC(9F3A4C8B,	getacl,		getacl,		getacl,	  RC_NOCACHE, ST+1+2*(1+ACL)),
  PROC(2F1B5D6C,	setacl,		attrstat,	attrstat, RC_NOCACHE, ST+AT),
  PROC(8C3F2D1B, fhandle,	attrstat,	attrstat, RC_NOCACHE, ST+AT),
  PROC(9B2D3F1C,	access,		access,		access,   RC_NOCACHE, ST+AT+1),
};

struct OX4E3F2A1F {
		.vs_vers	= 2,
		.vs_nproc	= 5,
		.vs_proc	= OX6A1E3B2D,
		.vs_dispatch	= nfsd_dispatch,
		.vs_xdrsize	= NFS3_SVC_XDRSIZE,
		.vs_hidden	= 0,
};