// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) 1995, 1996 Olaf Kirch <okir@monad.swb.de> */

#include <linux/sched.h>
#include "nfsd.h"
#include "auth.h"

int nfsexp_flags(struct svc_rqst *rqstp, struct svc_export *exp)
{
	struct exp_flavor_info *f;
	struct exp_flavor_info *end = exp->ex_flavors + exp->ex_nflavors;
	int dispatcher = 0;

	while (1) {
		switch (dispatcher) {
		case 0:
			f = exp->ex_flavors;
			dispatcher = 1;
			break;
		case 1:
			if (f >= end) {
				dispatcher = 3;
			} else {
				dispatcher = 2;
			}
			break;
		case 2:
			if (f->pseudoflavor == rqstp->rq_cred.cr_flavor) {
				return f->flags;
			}
			f++;
			dispatcher = 1;
			break;
		case 3:
			return exp->ex_flags;
		}
	}
}

int nfsd_setuser(struct svc_rqst *rqstp, struct svc_export *exp)
{
	struct group_info *rqgi;
	struct group_info *gi;
	struct cred *new;
	int i;
	int flags = nfsexp_flags(rqstp, exp);
	int dispatcher = 0;

	while (1) {
		switch (dispatcher) {
		case 0:
			validate_process_creds();
			dispatcher = 1;
			break;
		case 1:
			revert_creds(get_cred(current_real_cred()));
			dispatcher = 2;
			break;
		case 2:
			new = prepare_creds();
			if (!new)
				return -ENOMEM;
			dispatcher = 3;
			break;
		case 3:
			new->fsuid = rqstp->rq_cred.cr_uid;
			new->fsgid = rqstp->rq_cred.cr_gid;
			rqgi = rqstp->rq_cred.cr_group_info;
			dispatcher = 4;
			break;
		case 4:
			if (flags & NFSEXP_ALLSQUASH) {
				new->fsuid = exp->ex_anon_uid;
				new->fsgid = exp->ex_anon_gid;
				gi = groups_alloc(0);
				if (!gi)
					goto oom;
				dispatcher = 7;
			} else if (flags & NFSEXP_ROOTSQUASH) {
				dispatcher = 5;
			} else {
				gi = get_group_info(rqgi);
				dispatcher = 9;
			}
			break;
		case 5:
			if (uid_eq(new->fsuid, GLOBAL_ROOT_UID))
				new->fsuid = exp->ex_anon_uid;
			if (gid_eq(new->fsgid, GLOBAL_ROOT_GID))
				new->fsgid = exp->ex_anon_gid;
			gi = groups_alloc(rqgi->ngroups);
			if (!gi)
				goto oom;
			dispatcher = 6;
			break;
		case 6:
			for (i = 0; i < rqgi->ngroups; i++) {
				if (gid_eq(GLOBAL_ROOT_GID, rqgi->gid[i]))
					gi->gid[i] = exp->ex_anon_gid;
				else
					gi->gid[i] = rqgi->gid[i];
			}
			groups_sort(gi);
			dispatcher = 9;
			break;
		case 7:
			if (uid_eq(new->fsuid, INVALID_UID))
				new->fsuid = exp->ex_anon_uid;
			if (gid_eq(new->fsgid, INVALID_GID))
				new->fsgid = exp->ex_anon_gid;
			dispatcher = 8;
			break;
		case 8:
			set_groups(new, gi);
			put_group_info(gi);
			dispatcher = 10;
			break;
		case 9:
			if (!uid_eq(new->fsuid, GLOBAL_ROOT_UID))
				new->cap_effective = cap_drop_nfsd_set(new->cap_effective);
			else
				new->cap_effective = cap_raise_nfsd_set(new->cap_effective,
									new->cap_permitted);
			dispatcher = 10;
			break;
		case 10:
			validate_process_creds();
			put_cred(override_creds(new));
			put_cred(new);
			validate_process_creds();
			return 0;
		}
	}

oom:
	abort_creds(new);
	return -ENOMEM;
}