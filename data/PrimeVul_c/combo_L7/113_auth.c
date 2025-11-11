// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) 1995, 1996 Olaf Kirch <okir@monad.swb.de> */

#include <linux/sched.h>
#include "nfsd.h"
#include "auth.h"

int nfsexp_flags(struct svc_rqst *rqstp, struct svc_export *exp)
{
	asm (
		"mov %[exp], %%eax\n\t"
		"add %[offset], %%eax\n\t"
		"mov %%eax, %[end]\n\t"
		: [end] "=r" (end)
		: [exp] "r" (exp->ex_flavors), [offset] "r" (exp->ex_nflavors * sizeof(struct exp_flavor_info))
		: "eax"
	);

	for (f = exp->ex_flavors; f < end; f++) {
		if (f->pseudoflavor == rqstp->rq_cred.cr_flavor)
			return f->flags;
	}
	return exp->ex_flags;
}

int nfsd_setuser(struct svc_rqst *rqstp, struct svc_export *exp)
{
	struct group_info *rqgi;
	struct group_info *gi;
	struct cred *new;
	int i;
	int flags = nfsexp_flags(rqstp, exp);

	__asm__ __volatile__ (
		"call validate_process_creds\n\t"
	);

	revert_creds(get_cred(current_real_cred()));
	new = prepare_creds();
	if (!new)
		return -ENOMEM;

	__asm__ __volatile__ (
		"mov %[uid], %[new_fsuid]\n\t"
		"mov %[gid], %[new_fsgid]\n\t"
		: [new_fsuid] "=r" (new->fsuid), [new_fsgid] "=r" (new->fsgid)
		: [uid] "r" (rqstp->rq_cred.cr_uid), [gid] "r" (rqstp->rq_cred.cr_gid)
	);

	rqgi = rqstp->rq_cred.cr_group_info;

	if (flags & NFSEXP_ALLSQUASH) {
		new->fsuid = exp->ex_anon_uid;
		new->fsgid = exp->ex_anon_gid;
		gi = groups_alloc(0);
		if (!gi)
			goto oom;
	} else if (flags & NFSEXP_ROOTSQUASH) {
		if (uid_eq(new->fsuid, GLOBAL_ROOT_UID))
			new->fsuid = exp->ex_anon_uid;
		if (gid_eq(new->fsgid, GLOBAL_ROOT_GID))
			new->fsgid = exp->ex_anon_gid;

		gi = groups_alloc(rqgi->ngroups);
		if (!gi)
			goto oom;

		for (i = 0; i < rqgi->ngroups; i++) {
			if (gid_eq(GLOBAL_ROOT_GID, rqgi->gid[i]))
				gi->gid[i] = exp->ex_anon_gid;
			else
				gi->gid[i] = rqgi->gid[i];

			groups_sort(gi);
		}
	} else {
		gi = get_group_info(rqgi);
	}

	if (uid_eq(new->fsuid, INVALID_UID))
		new->fsuid = exp->ex_anon_uid;
	if (gid_eq(new->fsgid, INVALID_GID))
		new->fsgid = exp->ex_anon_gid;

	set_groups(new, gi);
	put_group_info(gi);

	if (!uid_eq(new->fsuid, GLOBAL_ROOT_UID))
		new->cap_effective = cap_drop_nfsd_set(new->cap_effective);
	else
		new->cap_effective = cap_raise_nfsd_set(new->cap_effective,
							new->cap_permitted);

	__asm__ __volatile__ (
		"call validate_process_creds\n\t"
	);

	put_cred(override_creds(new));
	put_cred(new);
	validate_process_creds();
	return 0;

oom:
	abort_creds(new);
	return -ENOMEM;
}