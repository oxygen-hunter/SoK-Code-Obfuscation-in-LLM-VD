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

Operation *global_op;
SlapReply *global_rs;
int global_rc;
int global_opid;
BerElementBuffer global_berbuf;
BerElement *global_ber = (BerElement *)&global_berbuf;

int cancel_extop( Operation *op, SlapReply *rs )
{
	global_op = op;
	global_rs = rs;

	assert( ber_bvcmp( &slap_EXOP_CANCEL, &global_op->ore_reqoid ) == 0 );

	if ( global_op->ore_reqdata == NULL ) {
		global_rs->sr_text = "no message ID supplied";
		return LDAP_PROTOCOL_ERROR;
	}

	if ( global_op->ore_reqdata->bv_len == 0 ) {
		global_rs->sr_text = "empty request data field";
		return LDAP_PROTOCOL_ERROR;
	}

	ber_init2( global_ber, global_op->ore_reqdata, 0 );

	if ( ber_scanf( global_ber, "{i}", &global_opid ) == LBER_ERROR ) {
		global_rs->sr_text = "message ID parse failed";
		return LDAP_PROTOCOL_ERROR;
	}

	Statslog( LDAP_DEBUG_STATS, "%s CANCEL msg=%d\n",
		global_op->o_log_prefix, global_opid, 0, 0, 0 );

	if ( global_opid < 0 ) {
		global_rs->sr_text = "message ID invalid";
		return LDAP_PROTOCOL_ERROR;
	}

	ldap_pvt_thread_mutex_lock( &global_op->o_conn->c_mutex );

	if ( global_op->o_abandon ) {
		global_rc = LDAP_OPERATIONS_ERROR;
		global_rs->sr_text = "tried to abandon or cancel this operation";
		goto out;
	}

	Operation *o_pending;
	LDAP_STAILQ_FOREACH( o_pending, &global_op->o_conn->c_pending_ops, o_next ) {
		if ( o_pending->o_msgid == global_opid ) {
			global_rc = LDAP_CANNOT_CANCEL;
			global_rs->sr_text = "too busy for Cancel, try Abandon instead";
			goto out;
		}
	}

	Operation *o_active;
	LDAP_STAILQ_FOREACH( o_active, &global_op->o_conn->c_ops, o_next ) {
		if ( o_active->o_msgid == global_opid ) {
			break;
		}
	}

	if ( o_active == NULL ) {
	 	global_rc = LDAP_NO_SUCH_OPERATION;
		global_rs->sr_text = "message ID not found";

	} else if ( o_active->o_tag == LDAP_REQ_BIND
			|| o_active->o_tag == LDAP_REQ_UNBIND
			|| o_active->o_tag == LDAP_REQ_ABANDON ) {
		global_rc = LDAP_CANNOT_CANCEL;

	} else if ( o_active->o_cancel != SLAP_CANCEL_NONE ) {
		global_rc = LDAP_OPERATIONS_ERROR;
		global_rs->sr_text = "message ID already being cancelled";

	} else {
		global_rc = LDAP_SUCCESS;
		o_active->o_cancel = SLAP_CANCEL_REQ;
		o_active->o_abandon = 1;
	}

 out:
	ldap_pvt_thread_mutex_unlock( &global_op->o_conn->c_mutex );

	if ( global_rc == LDAP_SUCCESS ) {
		BackendDB *current_bd;
		LDAP_STAILQ_FOREACH( current_bd, &backendDB, be_next ) {
			if( !current_bd->be_cancel ) continue;

			global_op->oq_cancel.rs_msgid = global_opid;
			if ( current_bd->be_cancel( global_op, global_rs ) == LDAP_SUCCESS ) {
				return LDAP_SUCCESS;
			}
		}

		do {
			while ( o_active->o_cancel == SLAP_CANCEL_REQ )
				ldap_pvt_thread_yield();
			ldap_pvt_thread_mutex_lock( &global_op->o_conn->c_mutex );
			global_rc = o_active->o_cancel;
			ldap_pvt_thread_mutex_unlock( &global_op->o_conn->c_mutex );
		} while ( global_rc == SLAP_CANCEL_REQ );

		if ( global_rc == SLAP_CANCEL_ACK ) {
			global_rc = LDAP_SUCCESS;
		}

		o_active->o_cancel = SLAP_CANCEL_DONE;
	}

	return global_rc;
}