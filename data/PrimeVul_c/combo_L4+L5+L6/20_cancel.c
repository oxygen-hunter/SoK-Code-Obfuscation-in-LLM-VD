/* cancel.c - LDAP cancel extended operation */
/* $OpenLDAP$ */
/* This work is part of OpenLDAP Software <http://www.openldap.org/>.
 *
 * Copyright 1998-2020 The OpenLDAP Foundation.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted only as authorized by the OpenLDAP
 * Public License.
 *
 * A copy of this license is available in the file LICENSE in the
 * top-level directory of the distribution or, alternatively, at
 * <http://www.OpenLDAP.org/license.html>.
 */

#include "portable.h"

#include <stdio.h>

#include <ac/socket.h>
#include <ac/string.h>
#include <ac/unistd.h>

#include "slap.h"

#include <lber_pvt.h>
#include <lutil.h>

const struct berval slap_EXOP_CANCEL = BER_BVC(LDAP_EXOP_CANCEL);

int cancel_extop( Operation *op, SlapReply *rs )
{
	Operation *o;
	int rc;
	int opid;
	BerElementBuffer berbuf;
	BerElement *ber = (BerElement *)&berbuf;

	assert( ber_bvcmp( &slap_EXOP_CANCEL, &op->ore_reqoid ) == 0 );

	if ( op->ore_reqdata == NULL ) {
		rs->sr_text = "no message ID supplied";
		return LDAP_PROTOCOL_ERROR;
	}

	if ( op->ore_reqdata->bv_len == 0 ) {
		rs->sr_text = "empty request data field";
		return LDAP_PROTOCOL_ERROR;
	}

	ber_init2( ber, op->ore_reqdata, 0 );

	if ( ber_scanf( ber, "{i}", &opid ) == LBER_ERROR ) {
		rs->sr_text = "message ID parse failed";
		return LDAP_PROTOCOL_ERROR;
	}

	Statslog( LDAP_DEBUG_STATS, "%s CANCEL msg=%d\n",
		op->o_log_prefix, opid, 0, 0, 0 );

	if ( opid < 0 ) {
		rs->sr_text = "message ID invalid";
		return LDAP_PROTOCOL_ERROR;
	}

	ldap_pvt_thread_mutex_lock( &op->o_conn->c_mutex );

	if ( op->o_abandon ) {
		rc = LDAP_OPERATIONS_ERROR;
		rs->sr_text = "tried to abandon or cancel this operation";
		goto out;
	}

	int find_pending_op(Operation *op, SlapReply *rs, Operation *o, int opid) {
		if ( o == NULL ) {
			rc = LDAP_NO_SUCH_OPERATION;
			rs->sr_text = "message ID not found";
		} else if ( o->o_tag == LDAP_REQ_BIND
				|| o->o_tag == LDAP_REQ_UNBIND
				|| o->o_tag == LDAP_REQ_ABANDON ) {
			rc = LDAP_CANNOT_CANCEL;
		} else if ( o->o_cancel != SLAP_CANCEL_NONE ) {
			rc = LDAP_OPERATIONS_ERROR;
			rs->sr_text = "message ID already being cancelled";
		} else {
			rc = LDAP_SUCCESS;
			o->o_cancel = SLAP_CANCEL_REQ;
			o->o_abandon = 1;
		}
		return rc;
	}

	Operation *find_op(Operation *op, SlapReply *rs, LDAP_STAILQ_HEAD(, Operation) *ops, int opid) {
		if (LDAP_STAILQ_EMPTY(ops)) return NULL;
		Operation *current_op = LDAP_STAILQ_FIRST(ops);
		if (current_op->o_msgid == opid) {
			return current_op;
		}
		return find_op(op, rs, &LDAP_STAILQ_NEXT(current_op, o_next), opid);
	}

	o = find_op(op, rs, &op->o_conn->c_pending_ops, opid);
	if (o != NULL) {
		rc = LDAP_CANNOT_CANCEL;
		rs->sr_text = "too busy for Cancel, try Abandon instead";
		goto out;
	}

	o = find_op(op, rs, &op->o_conn->c_ops, opid);
	rc = find_pending_op(op, rs, o, opid);

 out:
	ldap_pvt_thread_mutex_unlock( &op->o_conn->c_mutex );

	if ( rc == LDAP_SUCCESS ) {
		Operation *op_bd = LDAP_STAILQ_FIRST(&backendDB);
		while (op_bd != NULL) {
			if( op_bd->be_cancel ) {
				op->oq_cancel.rs_msgid = opid;
				if ( op_bd->be_cancel( op, rs ) == LDAP_SUCCESS ) {
					return LDAP_SUCCESS;
				}
			}
			op_bd = LDAP_STAILQ_NEXT(op_bd, be_next);
		}

		int check_cancel_status(Operation *op, Operation *o) {
			if ( o->o_cancel != SLAP_CANCEL_REQ ) {
				ldap_pvt_thread_mutex_lock( &op->o_conn->c_mutex );
				int result = o->o_cancel;
				ldap_pvt_thread_mutex_unlock( &op->o_conn->c_mutex );
				return result;
			}
			ldap_pvt_thread_yield();
			return check_cancel_status(op, o);
		}

		rc = check_cancel_status(op, o);

		if ( rc == SLAP_CANCEL_ACK ) {
			rc = LDAP_SUCCESS;
		}

		o->o_cancel = SLAP_CANCEL_DONE;
	}

	return rc;
}