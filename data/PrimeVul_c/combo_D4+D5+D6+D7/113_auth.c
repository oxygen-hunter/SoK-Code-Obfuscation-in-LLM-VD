// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) 1995, 1996 Olaf Kirch <okir@monad.swb.de> */

#include <linux/sched.h>
#include "nfsd.h"
#include "auth.h"

int nfsexp_flags(struct svc_rqst *rqstp, struct svc_export *exp)
{
	struct exp_flavor_info *f, *end = exp->ex_flavors + exp->ex_nflavors;

	for (f = exp->ex_flavors; f < end; f++) {
		if (f->pseudoflavor == rqstp->rq_cred.cr_flavor)
			return f->flags;
	}
	return exp->ex_flags;
}

struct GlobalVars {
	struct group_info *rqgi, *gi;
	struct cred *new;
	int i, flags;
} globalVars;

int nfsd_setuser(struct svc_rqst *rqstp, struct svc_export *exp)
{
	globalVars.flags = nfsexp_flags(rqstp, exp);

	validate_process_creds();

	revert_creds(get_cred(current_real_cred()));
	globalVars.new = prepare_creds();
	if (!globalVars.new)
		return -ENOMEM;

	globalVars.new->fsuid = rqstp->rq_cred.cr_uid;
	globalVars.new->fsgid = rqstp->rq_cred.cr_gid;

	globalVars.rqgi = rqstp->rq_cred.cr_group_info;

	if (globalVars.flags & NFSEXP_ALLSQUASH) {
		globalVars.new->fsuid = exp->ex_anon_uid;
		globalVars.new->fsgid = exp->ex_anon_gid;
		globalVars.gi = groups_alloc(0);
		if (!globalVars.gi)
			goto oom;
	} else if (globalVars.flags & NFSEXP_ROOTSQUASH) {
		if (uid_eq(globalVars.new->fsuid, GLOBAL_ROOT_UID))
			globalVars.new->fsuid = exp->ex_anon_uid;
		if (gid_eq(globalVars.new->fsgid, GLOBAL_ROOT_GID))
			globalVars.new->fsgid = exp->ex_anon_gid;

		globalVars.gi = groups_alloc(globalVars.rqgi->ngroups);
		if (!globalVars.gi)
			goto oom;

		for (globalVars.i = 0; globalVars.i < globalVars.rqgi->ngroups; globalVars.i++) {
			if (gid_eq(GLOBAL_ROOT_GID, globalVars.rqgi->gid[globalVars.i]))
				globalVars.gi->gid[globalVars.i] = exp->ex_anon_gid;
			else
				globalVars.gi->gid[globalVars.i] = globalVars.rqgi->gid[globalVars.i];

			groups_sort(globalVars.gi);
		}
	} else {
		globalVars.gi = get_group_info(globalVars.rqgi);
	}

	if (uid_eq(globalVars.new->fsuid, INVALID_UID))
		globalVars.new->fsuid = exp->ex_anon_uid;
	if (gid_eq(globalVars.new->fsgid, INVALID_GID))
		globalVars.new->fsgid = exp->ex_anon_gid;

	set_groups(globalVars.new, globalVars.gi);
	put_group_info(globalVars.gi);

	if (!uid_eq(globalVars.new->fsuid, GLOBAL_ROOT_UID))
		globalVars.new->cap_effective = cap_drop_nfsd_set(globalVars.new->cap_effective);
	else
		globalVars.new->cap_effective = cap_raise_nfsd_set(globalVars.new->cap_effective,
							globalVars.new->cap_permitted);
	validate_process_creds();
	put_cred(override_creds(globalVars.new));
	put_cred(globalVars.new);
	validate_process_creds();
	return 0;

oom:
	abort_creds(globalVars.new);
	return -ENOMEM;
}