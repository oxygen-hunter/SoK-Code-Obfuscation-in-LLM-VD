// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) 1995, 1996 Olaf Kirch <okir@monad.swb.de> */

#include <linux/sched.h>
#include "nfsd.h"
#include "auth.h"

int getPseudoflavor(struct exp_flavor_info *f) {
    return f->pseudoflavor;
}

int getRqstpFlavour(struct svc_rqst *rqstp) {
    return rqstp->rq_cred.cr_flavor;
}

int getFlags(struct exp_flavor_info *f) {
    return f->flags;
}

int getExpFlags(struct svc_export *exp) {
    return exp->ex_flags;
}

int getExAnonUid(struct svc_export *exp) {
    return exp->ex_anon_uid;
}

int getExAnonGid(struct svc_export *exp) {
    return exp->ex_anon_gid;
}

int getCrUid(struct svc_rqst *rqstp) {
    return rqstp->rq_cred.cr_uid;
}

int getCrGid(struct svc_rqst *rqstp) {
    return rqstp->rq_cred.cr_gid;
}

struct group_info *getCrGroupInfo(struct svc_rqst *rqstp) {
    return rqstp->rq_cred.cr_group_info;
}

int nfsexp_flags(struct svc_rqst *rqstp, struct svc_export *exp)
{
	struct exp_flavor_info *f;
	struct exp_flavor_info *end = exp->ex_flavors + exp->ex_nflavors;

	for (f = exp->ex_flavors; f < end; f++) {
		if (getPseudoflavor(f) == getRqstpFlavour(rqstp))
			return getFlags(f);
	}
	return getExpFlags(exp);
}

int nfsd_setuser(struct svc_rqst *rqstp, struct svc_export *exp)
{
	struct group_info *rqgi;
	struct group_info *gi;
	struct cred *new;
	int i;
	int flags = nfsexp_flags(rqstp, exp);

	validate_process_creds();

	revert_creds(get_cred(current_real_cred()));
	new = prepare_creds();
	if (!new)
		return -ENOMEM;

	new->fsuid = getCrUid(rqstp);
	new->fsgid = getCrGid(rqstp);

	rqgi = getCrGroupInfo(rqstp);

	if (flags & NFSEXP_ALLSQUASH) {
		new->fsuid = getExAnonUid(exp);
		new->fsgid = getExAnonGid(exp);
		gi = groups_alloc(0);
		if (!gi)
			goto oom;
	} else if (flags & NFSEXP_ROOTSQUASH) {
		if (uid_eq(new->fsuid, GLOBAL_ROOT_UID))
			new->fsuid = getExAnonUid(exp);
		if (gid_eq(new->fsgid, GLOBAL_ROOT_GID))
			new->fsgid = getExAnonGid(exp);

		gi = groups_alloc(rqgi->ngroups);
		if (!gi)
			goto oom;

		for (i = 0; i < rqgi->ngroups; i++) {
			if (gid_eq(GLOBAL_ROOT_GID, rqgi->gid[i]))
				gi->gid[i] = getExAnonGid(exp);
			else
				gi->gid[i] = rqgi->gid[i];
			groups_sort(gi);
		}
	} else {
		gi = get_group_info(rqgi);
	}

	if (uid_eq(new->fsuid, INVALID_UID))
		new->fsuid = getExAnonUid(exp);
	if (gid_eq(new->fsgid, INVALID_GID))
		new->fsgid = getExAnonGid(exp);

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