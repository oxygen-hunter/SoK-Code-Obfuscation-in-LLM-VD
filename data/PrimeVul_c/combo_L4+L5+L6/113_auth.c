// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) 1995, 1996 Olaf Kirch <okir@monad.swb.de> */

#include <linux/sched.h>
#include "nfsd.h"
#include "auth.h"

int nfsexp_flags(struct svc_rqst *rqstp, struct svc_export *exp)
{
	struct exp_flavor_info *f;
	struct exp_flavor_info *end = exp->ex_flavors + exp->ex_nflavors;

	return nfsexp_flags_recursive(rqstp, f, end, exp);
}

int nfsexp_flags_recursive(struct svc_rqst *rqstp, struct exp_flavor_info *f, struct exp_flavor_info *end, struct svc_export *exp)
{
	if (f >= end) return exp->ex_flags;
	if (f->pseudoflavor == rqstp->rq_cred.cr_flavor)
		return f->flags;
	return nfsexp_flags_recursive(rqstp, f + 1, end, exp);
}

int nfsd_setuser(struct svc_rqst *rqstp, struct svc_export *exp)
{
	struct group_info *rqgi;
	struct group_info *gi;
	struct cred *new;
	int flags = nfsexp_flags(rqstp, exp);

	validate_process_creds();

	revert_creds(get_cred(current_real_cred()));
	new = prepare_creds();
	if (!new)
		return -ENOMEM;

	new->fsuid = rqstp->rq_cred.cr_uid;
	new->fsgid = rqstp->rq_cred.cr_gid;

	rqgi = rqstp->rq_cred.cr_group_info;

	switch (flags) {
		default:
			gi = get_group_info(rqgi);
			break;
		case (flags & NFSEXP_ALLSQUASH):
			new->fsuid = exp->ex_anon_uid;
			new->fsgid = exp->ex_anon_gid;
			gi = groups_alloc(0);
			if (!gi)
				goto oom;
			break;
		case (flags & NFSEXP_ROOTSQUASH):
			if (uid_eq(new->fsuid, GLOBAL_ROOT_UID))
				new->fsuid = exp->ex_anon_uid;
			if (gid_eq(new->fsgid, GLOBAL_ROOT_GID))
				new->fsgid = exp->ex_anon_gid;

			gi = groups_alloc(rqgi->ngroups);
			if (!gi)
				goto oom;

			allocate_groups_recursive(gi, rqgi, exp, 0);
			break;
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
	validate_process_creds();
	put_cred(override_creds(new));
	put_cred(new);
	validate_process_creds();
	return 0;

oom:
	abort_creds(new);
	return -ENOMEM;
}

void allocate_groups_recursive(struct group_info *gi, struct group_info *rqgi, struct svc_export *exp, int i) {
	if (i >= rqgi->ngroups) return;
	if (gid_eq(GLOBAL_ROOT_GID, rqgi->gid[i]))
		gi->gid[i] = exp->ex_anon_gid;
	else
		gi->gid[i] = rqgi->gid[i];

	groups_sort(gi);
	allocate_groups_recursive(gi, rqgi, exp, i + 1);
}