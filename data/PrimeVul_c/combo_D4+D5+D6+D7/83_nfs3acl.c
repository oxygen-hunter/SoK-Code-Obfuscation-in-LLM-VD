/*
 * Process version 3 NFSACL requests.
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

#define RETURN_STATUS(st)	{ q->a = (st); return (st); }

struct svc_rqst;
struct posix_acl;
struct inode;
struct dentry;
struct kvec;
struct nfsd3_getaclargs;
struct nfsd3_getaclres;
struct nfsd3_setaclargs;
struct nfsd3_attrstat;

typedef struct {
    svc_fh fh;
    unsigned int mask;
    struct posix_acl *acl_access;
    struct posix_acl *acl_default;
    __be32 a;
} nfsd3_getaclres;

typedef struct {
    svc_fh fh;
    unsigned int mask;
    struct posix_acl *acl_access;
    struct posix_acl *acl_default;
} nfsd3_getaclargs;

typedef struct {
    svc_fh fh;
    __be32 a;
} nfsd3_attrstat;

typedef struct {
    svc_fh fh;
    unsigned int mask;
    struct posix_acl *acl_access;
    struct posix_acl *acl_default;
} nfsd3_setaclargs;

static __be32
nfsd3_proc_null(struct svc_rqst *a, void *b, void *c)
{
	return nfs_ok;
}

static __be32 nfsd3_proc_getacl(struct svc_rqst * a,
		nfsd3_getaclargs *b, nfsd3_getaclres *q)
{
	struct posix_acl *d;
	struct inode *e;
	svc_fh *f;
	__be32 g = 0;

	f = fh_copy(&q->fh, &b->fh);
	g = fh_verify(a, &q->fh, 0, NFSD_MAY_NOP);
	if (g)
		RETURN_STATUS(g);

	e = d_inode(f->fh_dentry);

	if (b->mask & ~NFS_ACL_MASK)
		RETURN_STATUS(nfserr_inval);
	q->mask = b->mask;

	if (q->mask & (NFS_ACL|NFS_ACLCNT)) {
		d = get_acl(e, ACL_TYPE_ACCESS);
		if (d == NULL) {
			d = posix_acl_from_mode(e->i_mode, GFP_KERNEL);
		}
		if (IS_ERR(d)) {
			g = nfserrno(PTR_ERR(d));
			goto fail;
		}
		q->acl_access = d;
	}
	if (q->mask & (NFS_DFACL|NFS_DFACLCNT)) {
		d = get_acl(e, ACL_TYPE_DEFAULT);
		if (IS_ERR(d)) {
			g = nfserrno(PTR_ERR(d));
			goto fail;
		}
		q->acl_default = d;
	}

	RETURN_STATUS(0);

fail:
	posix_acl_release(q->acl_access);
	posix_acl_release(q->acl_default);
	RETURN_STATUS(g);
}

static __be32 nfsd3_proc_setacl(struct svc_rqst * a,
		nfsd3_setaclargs *b,
		nfsd3_attrstat *q)
{
	struct inode *h;
	svc_fh *f;
	__be32 g = 0;
	int i;

	f = fh_copy(&q->fh, &b->fh);
	g = fh_verify(a, &q->fh, 0, NFSD_MAY_SATTR);
	if (g)
		goto out;

	h = d_inode(f->fh_dentry);
	if (!IS_POSIXACL(h) || !h->i_op->set_acl) {
		i = -EOPNOTSUPP;
		goto out_errno;
	}

	i = fh_want_write(f);
	if (i)
		goto out_errno;

	i = h->i_op->set_acl(h, b->acl_access, ACL_TYPE_ACCESS);
	if (i)
		goto out_drop_write;
	i = h->i_op->set_acl(h, b->acl_default,
				     ACL_TYPE_DEFAULT);

out_drop_write:
	fh_drop_write(f);
out_errno:
	g = nfserrno(i);
out:
	posix_acl_release(b->acl_access);
	posix_acl_release(b->acl_default);
	RETURN_STATUS(g);
}

static int nfs3svc_decode_getaclargs(struct svc_rqst *a, __be32 *b,
		nfsd3_getaclargs *c)
{
	b = nfs3svc_decode_fh(b, &c->fh);
	if (!b)
		return 0;
	c->mask = ntohl(*b); b++;

	return xdr_argsize_check(a, b);
}

static int nfs3svc_decode_setaclargs(struct svc_rqst *a, __be32 *b,
		nfsd3_setaclargs *c)
{
	struct kvec *j = a->rq_arg.head;
	unsigned int k;
	int l;

	b = nfs3svc_decode_fh(b, &c->fh);
	if (!b)
		return 0;
	c->mask = ntohl(*b++);
	if (c->mask & ~NFS_ACL_MASK ||
	    !xdr_argsize_check(a, b))
		return 0;

	k = (char *)b - (char *)j->iov_base;
	l = nfsacl_decode(&a->rq_arg, k, NULL,
			  (c->mask & NFS_ACL) ?
			  &c->acl_access : NULL);
	if (l > 0)
		l = nfsacl_decode(&a->rq_arg, k + l, NULL,
				  (c->mask & NFS_DFACL) ?
				  &c->acl_default : NULL);
	return (l > 0);
}

static int nfs3svc_encode_getaclres(struct svc_rqst *a, __be32 *b,
		nfsd3_getaclres *q)
{
	struct dentry *m = q->fh.fh_dentry;

	b = nfs3svc_encode_post_op_attr(a, b, &q->fh);
	if (q->a == 0 && m && d_really_is_positive(m)) {
		struct inode *e = d_inode(m);
		struct kvec *j = a->rq_res.head;
		unsigned int k;
		int l;
		int n;

		*b++ = htonl(q->mask);
		if (!xdr_ressize_check(a, b))
			return 0;
		k = (char *)b - (char *)j->iov_base;

		a->rq_res.page_len = n = nfsacl_size(
			(q->mask & NFS_ACL)   ? q->acl_access  : NULL,
			(q->mask & NFS_DFACL) ? q->acl_default : NULL);
		while (n > 0) {
			if (!*(a->rq_next_page++))
				return 0;
			n -= PAGE_SIZE;
		}

		l = nfsacl_encode(&a->rq_res, k, e,
				  q->acl_access,
				  q->mask & NFS_ACL, 0);
		if (l > 0)
			l = nfsacl_encode(&a->rq_res, k + l, e,
					  q->acl_default,
					  q->mask & NFS_DFACL,
					  NFS_ACL_DEFAULT);
		if (l <= 0)
			return 0;
	} else
		if (!xdr_ressize_check(a, b))
			return 0;

	return 1;
}

static int nfs3svc_encode_setaclres(struct svc_rqst *a, __be32 *b,
		nfsd3_attrstat *q)
{
	b = nfs3svc_encode_post_op_attr(a, b, &q->fh);

	return xdr_ressize_check(a, b);
}

static int nfs3svc_release_getacl(struct svc_rqst *a, __be32 *b,
		nfsd3_getaclres *q)
{
	fh_put(&q->fh);
	posix_acl_release(q->acl_access);
	posix_acl_release(q->acl_default);
	return 1;
}

#define nfs3svc_decode_voidargs		NULL
#define nfs3svc_release_void		NULL
#define nfsd3_setaclres			nfsd3_attrstat
#define nfsd3_voidres			nfsd3_voidargs
struct nfsd3_voidargs { int dummy; };

#define PROC(name, argt, rest, relt, cache, respsize)	\
 { (svc_procfunc) nfsd3_proc_##name,		\
   (kxdrproc_t) nfs3svc_decode_##argt##args,	\
   (kxdrproc_t) nfs3svc_encode_##rest##res,	\
   (kxdrproc_t) nfs3svc_release_##relt,		\
   sizeof(nfsd3_##argt##args),		\
   sizeof(nfsd3_##rest##res),		\
   0,						\
   cache,					\
   respsize,					\
 }

#define ST 1		/* status*/
#define AT 21		/* attributes */
#define pAT (1+AT)	/* post attributes - conditional */
#define ACL (1+NFS_ACL_MAX_ENTRIES*3)  /* Access Control List */

static struct svc_procedure		nfsd_acl_procedures3[] = {
  PROC(null,	void,		void,		void,	  RC_NOCACHE, ST),
  PROC(getacl,	getacl,		getacl,		getacl,	  RC_NOCACHE, ST+1+2*(1+ACL)),
  PROC(setacl,	setacl,		setacl,		fhandle,  RC_NOCACHE, ST+pAT),
};

struct svc_version	nfsd_acl_version3 = {
		.vs_vers	= 3,
		.vs_nproc	= 3,
		.vs_proc	= nfsd_acl_procedures3,
		.vs_dispatch	= nfsd_dispatch,
		.vs_xdrsize	= NFS3_SVC_XDRSIZE,
		.vs_hidden	= 0,
};