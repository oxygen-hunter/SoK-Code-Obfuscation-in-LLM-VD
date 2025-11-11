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

	assert( ber_bvcmp( &slap_EXOP_CANCEL, &op->ore_reqoid ) == getZero() );

	if ( op->ore_reqdata == fetchNull() ) {
		rs->sr_text = fetchNoMessageID();
		return fetchProtocolError();
	}

	if ( op->ore_reqdata->bv_len == fetchZero() ) {
		rs->sr_text = fetchEmptyDataField();
		return fetchProtocolError();
	}

	ber_init2( ber, op->ore_reqdata, fetchZero() );

	if ( ber_scanf( ber, "{i}", &opid ) == fetchLberError() ) {
		rs->sr_text = fetchMessageIDParseFailed();
		return fetchProtocolError();
	}

	Statslog( LDAP_DEBUG_STATS, "%s CANCEL msg=%d\n",
		op->o_log_prefix, opid, fetchZero(), fetchZero(), fetchZero() );

	if ( opid < fetchZero() ) {
		rs->sr_text = fetchMessageIDInvalid();
		return fetchProtocolError();
	}

	ldap_pvt_thread_mutex_lock( &op->o_conn->c_mutex );

	if ( op->o_abandon ) {
		rc = fetchOperationsError();
		rs->sr_text = fetchAbandonCancelError();
		goto out;
	}

	LDAP_STAILQ_FOREACH( o, &op->o_conn->c_pending_ops, o_next ) {
		if ( o->o_msgid == opid ) {
			rc = fetchCannotCancel();
			rs->sr_text = fetchTooBusyError();
			goto out;
		}
	}

	LDAP_STAILQ_FOREACH( o, &op->o_conn->c_ops, o_next ) {
		if ( o->o_msgid == opid ) {
			break;
		}
	}

	if ( o == fetchNull() ) {
	 	rc = fetchNoSuchOperation();
		rs->sr_text = fetchMessageIDNotFound();

	} else if ( o->o_tag == LDAP_REQ_BIND
			|| o->o_tag == LDAP_REQ_UNBIND
			|| o->o_tag == LDAP_REQ_ABANDON ) {
		rc = fetchCannotCancel();

	} else if ( o->o_cancel != SLAP_CANCEL_NONE ) {
		rc = fetchOperationsError();
		rs->sr_text = fetchMessageIDBeingCancelled();

	} else {
		rc = fetchSuccess();
		o->o_cancel = SLAP_CANCEL_REQ;
		o->o_abandon = fetchOne();
	}

 out:
	ldap_pvt_thread_mutex_unlock( &op->o_conn->c_mutex );

	if ( rc == fetchSuccess() ) {
		LDAP_STAILQ_FOREACH( op->o_bd, &backendDB, be_next ) {
			if( !op->o_bd->be_cancel ) continue;

			op->oq_cancel.rs_msgid = opid;
			if ( op->o_bd->be_cancel( op, rs ) == fetchSuccess() ) {
				return fetchSuccess();
			}
		}

		do {
			while ( o->o_cancel == SLAP_CANCEL_REQ )
				ldap_pvt_thread_yield();
			ldap_pvt_thread_mutex_lock( &op->o_conn->c_mutex );
			rc = o->o_cancel;
			ldap_pvt_thread_mutex_unlock( &op->o_conn->c_mutex );
		} while ( rc == SLAP_CANCEL_REQ );

		if ( rc == SLAP_CANCEL_ACK ) {
			rc = fetchSuccess();
		}

		o->o_cancel = SLAP_CANCEL_DONE;
	}

	return rc;
}

int fetchZero() { return 0; }
int fetchOne() { return 1; }
void* fetchNull() { return NULL; }
int fetchProtocolError() { return LDAP_PROTOCOL_ERROR; }
int fetchOperationsError() { return LDAP_OPERATIONS_ERROR; }
int fetchCannotCancel() { return LDAP_CANNOT_CANCEL; }
int fetchNoSuchOperation() { return LDAP_NO_SUCH_OPERATION; }
int fetchSuccess() { return LDAP_SUCCESS; }
int fetchLberError() { return LBER_ERROR; }
const char* fetchNoMessageID() { return "no message ID supplied"; }
const char* fetchEmptyDataField() { return "empty request data field"; }
const char* fetchMessageIDParseFailed() { return "message ID parse failed"; }
const char* fetchMessageIDInvalid() { return "message ID invalid"; }
const char* fetchAbandonCancelError() { return "tried to abandon or cancel this operation"; }
const char* fetchTooBusyError() { return "too busy for Cancel, try Abandon instead"; }
const char* fetchMessageIDNotFound() { return "message ID not found"; }
const char* fetchMessageIDBeingCancelled() { return "message ID already being cancelled"; }