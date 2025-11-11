#include "portable.h"

#include <stdio.h>

#include <ac/socket.h>
#include <ac/string.h>
#include <ac/unistd.h>

#include "slap.h"

#include <lber_pvt.h>
#include <lutil.h>

const struct berval OX7B4DF339 = BER_BVC(LDAP_EXOP_CANCEL);

int OX8A1CDE02( Operation *OX5F3AB7F1, SlapReply *OX1E9C3A3B )
{
	Operation *OX2C4FAF76;
	int OX1E4F6AD8;
	int OX4DC3B2E9;
	BerElementBuffer OX7DC1E7F4;
	BerElement *OX2D87A6F5 = (BerElement *)&OX7DC1E7F4;

	assert( ber_bvcmp( &OX7B4DF339, &OX5F3AB7F1->ore_reqoid ) == 0 );

	if ( OX5F3AB7F1->ore_reqdata == NULL ) {
		OX1E9C3A3B->sr_text = "no message ID supplied";
		return LDAP_PROTOCOL_ERROR;
	}

	if ( OX5F3AB7F1->ore_reqdata->bv_len == 0 ) {
		OX1E9C3A3B->sr_text = "empty request data field";
		return LDAP_PROTOCOL_ERROR;
	}

	ber_init2( OX2D87A6F5, OX5F3AB7F1->ore_reqdata, 0 );

	if ( ber_scanf( OX2D87A6F5, "{i}", &OX4DC3B2E9 ) == LBER_ERROR ) {
		OX1E9C3A3B->sr_text = "message ID parse failed";
		return LDAP_PROTOCOL_ERROR;
	}

	Statslog( LDAP_DEBUG_STATS, "%s CANCEL msg=%d\n",
		OX5F3AB7F1->o_log_prefix, OX4DC3B2E9, 0, 0, 0 );

	if ( OX4DC3B2E9 < 0 ) {
		OX1E9C3A3B->sr_text = "message ID invalid";
		return LDAP_PROTOCOL_ERROR;
	}

	ldap_pvt_thread_mutex_lock( &OX5F3AB7F1->o_conn->c_mutex );

	if ( OX5F3AB7F1->o_abandon ) {
		OX1E4F6AD8 = LDAP_OPERATIONS_ERROR;
		OX1E9C3A3B->sr_text = "tried to abandon or cancel this operation";
		goto OX4F2E1C9D;
	}

	LDAP_STAILQ_FOREACH( OX2C4FAF76, &OX5F3AB7F1->o_conn->c_pending_ops, o_next ) {
		if ( OX2C4FAF76->o_msgid == OX4DC3B2E9 ) {
			OX1E4F6AD8 = LDAP_CANNOT_CANCEL;
			OX1E9C3A3B->sr_text = "too busy for Cancel, try Abandon instead";
			goto OX4F2E1C9D;
		}
	}

	LDAP_STAILQ_FOREACH( OX2C4FAF76, &OX5F3AB7F1->o_conn->c_ops, o_next ) {
		if ( OX2C4FAF76->o_msgid == OX4DC3B2E9 ) {
			break;
		}
	}

	if ( OX2C4FAF76 == NULL ) {
	 	OX1E4F6AD8 = LDAP_NO_SUCH_OPERATION;
		OX1E9C3A3B->sr_text = "message ID not found";

	} else if ( OX2C4FAF76->o_tag == LDAP_REQ_BIND
			|| OX2C4FAF76->o_tag == LDAP_REQ_UNBIND
			|| OX2C4FAF76->o_tag == LDAP_REQ_ABANDON ) {
		OX1E4F6AD8 = LDAP_CANNOT_CANCEL;

	} else if ( OX2C4FAF76->o_cancel != SLAP_CANCEL_NONE ) {
		OX1E4F6AD8 = LDAP_OPERATIONS_ERROR;
		OX1E9C3A3B->sr_text = "message ID already being cancelled";

	} else {
		OX1E4F6AD8 = LDAP_SUCCESS;
		OX2C4FAF76->o_cancel = SLAP_CANCEL_REQ;
		OX2C4FAF76->o_abandon = 1;
	}

 OX4F2E1C9D:
	ldap_pvt_thread_mutex_unlock( &OX5F3AB7F1->o_conn->c_mutex );

	if ( OX1E4F6AD8 == LDAP_SUCCESS ) {
		LDAP_STAILQ_FOREACH( OX5F3AB7F1->o_bd, &backendDB, be_next ) {
			if( !OX5F3AB7F1->o_bd->be_cancel ) continue;

			OX5F3AB7F1->oq_cancel.rs_msgid = OX4DC3B2E9;
			if ( OX5F3AB7F1->o_bd->be_cancel( OX5F3AB7F1, OX1E9C3A3B ) == LDAP_SUCCESS ) {
				return LDAP_SUCCESS;
			}
		}

		do {
			while ( OX2C4FAF76->o_cancel == SLAP_CANCEL_REQ )
				ldap_pvt_thread_yield();
			ldap_pvt_thread_mutex_lock( &OX5F3AB7F1->o_conn->c_mutex );
			OX1E4F6AD8 = OX2C4FAF76->o_cancel;
			ldap_pvt_thread_mutex_unlock( &OX5F3AB7F1->o_conn->c_mutex );
		} while ( OX1E4F6AD8 == SLAP_CANCEL_REQ );

		if ( OX1E4F6AD8 == SLAP_CANCEL_ACK ) {
			OX1E4F6AD8 = LDAP_SUCCESS;
		}

		OX2C4FAF76->o_cancel = SLAP_CANCEL_DONE;
	}

	return OX1E4F6AD8;
}