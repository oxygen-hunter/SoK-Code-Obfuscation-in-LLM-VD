#include "node-lmdb.h"

using namespace v8;
using namespace node;

TxnWrap::TxnWrap(MDB_env *env, MDB_txn *txn) {
    this->env = env;
    this->txn = txn;
    this->flags = getInitialFlags();
}

TxnWrap::~TxnWrap() {
    if (isTxnOpen()) {
        closeTxn();
        this->removeFromEnvWrap();
    }
}

void TxnWrap::removeFromEnvWrap() {
    if (isEnvWrapActive()) {
        if (isCurrentWriteTxn()) {
            clearCurrentWriteTxn();
        }
        else {
            auto it = findReadTxn();
            if (isReadTxnFound(it)) {
                eraseReadTxn(it);
            }
        }
        
        unrefEnvWrap();
        clearEnvWrap();
    }
}

NAN_METHOD(TxnWrap::ctor) {
    Nan::HandleScope scope;

    EnvWrap *ew = getEnvWrap(info);
    int flags = getDefaultFlags();

    if (isOptionsObject(info)) {
        Local<Object> options = getOptions(info);
        setFlagsFromOptions(&flags, options);
    }
    
    if (hasCurrentWriteTxn(flags, ew)) {
        return Nan::ThrowError(getWriteTxnErrorMessage());
    }

    MDB_txn *txn;
    int rc = beginTxn(ew, flags, &txn);
    if (isTxnBeginError(rc)) {
        return handleTxnBeginError(rc);
    }

    TxnWrap* tw = createTxnWrap(ew, txn, flags);
    tw->ew = ew;
    refEnvWrap(ew);
    wrapTxn(tw, info);
    
    if (isWriteTxn(flags)) {
        setCurrentWriteTxn(ew, tw);
    }
    else {
        addReadTxn(ew, tw);
    }

    return info.GetReturnValue().Set(getReturnValue(info));
}

NAN_METHOD(TxnWrap::commit) {
    Nan::HandleScope scope;

    TxnWrap *tw = unwrapTxn(info);

    if (isTxnClosed(tw)) {
        return Nan::ThrowError(getTxnClosedErrorMessage());
    }

    int rc = commitTxn(tw);
    tw->removeFromEnvWrap();
    closeTxn(tw);

    if (isCommitError(rc)) {
        return throwLmdbError(rc);
    }
}

NAN_METHOD(TxnWrap::abort) {
    Nan::HandleScope scope;

    TxnWrap *tw = unwrapTxn(info);

    if (isTxnClosed(tw)) {
        return Nan::ThrowError(getTxnClosedErrorMessage());
    }

    abortTxn(tw);
    tw->removeFromEnvWrap();
    closeTxn(tw);
}

NAN_METHOD(TxnWrap::reset) {
    Nan::HandleScope scope;

    TxnWrap *tw = unwrapTxn(info);

    if (isTxnClosed(tw)) {
        return Nan::ThrowError(getTxnClosedErrorMessage());
    }

    resetTxn(tw);
}

NAN_METHOD(TxnWrap::renew) {
    Nan::HandleScope scope;

    TxnWrap *tw = unwrapTxn(info);

    if (isTxnClosed(tw)) {
        return Nan::ThrowError(getTxnClosedErrorMessage());
    }

    int rc = renewTxn(tw);
    if (isRenewError(rc)) {
        return throwLmdbError(rc);
    }
}

Nan::NAN_METHOD_RETURN_TYPE TxnWrap::getCommon(Nan::NAN_METHOD_ARGS_TYPE info, Local<Value> (*successFunc)(MDB_val&)) {
    Nan::HandleScope scope;
    
    if (isInvalidArgumentCount(info)) {
        return Nan::ThrowError(getInvalidArgCountErrorMessage());
    }

    TxnWrap *tw = unwrapTxn(info);
    DbiWrap *dw = unwrapDbi(info);

    if (isTxnClosed(tw)) {
        return Nan::ThrowError(getTxnClosedErrorMessage());
    }

    MDB_val key, oldkey, data;
    bool keyIsValid;
    auto keyType = validateKeyType(info, dw, keyIsValid);
    if (!keyIsValid) {
        return;
    }
    auto freeKey = convertArgToKey(info, key, keyType, keyIsValid);
    if (!keyIsValid) {
        return;
    }

    saveOldKeyState(key, oldkey);

    int rc = getTxnData(tw, dw, key, data);
    
    if (freeKey) {
        freeKey(oldkey);
    }

    if (isDataNotFound(rc)) {
        return info.GetReturnValue().Set(Nan::Null());
    }
    else if (isDataFetchError(rc)) {
        return throwLmdbError(rc);
    }
    else {
      return info.GetReturnValue().Set(successFunc(data));
    }
}

NAN_METHOD(TxnWrap::getString) {
    return getCommon(info, valToString);
}

NAN_METHOD(TxnWrap::getStringUnsafe) {
    return getCommon(info, valToStringUnsafe);
}

NAN_METHOD(TxnWrap::getBinary) {
    return getCommon(info, valToBinary);
}

NAN_METHOD(TxnWrap::getBinaryUnsafe) {
    return getCommon(info, valToBinaryUnsafe);
}

NAN_METHOD(TxnWrap::getNumber) {
    return getCommon(info, valToNumber);
}

NAN_METHOD(TxnWrap::getBoolean) {
    return getCommon(info, valToBoolean);
}

Nan::NAN_METHOD_RETURN_TYPE TxnWrap::putCommon(Nan::NAN_METHOD_ARGS_TYPE info, void (*fillFunc)(Nan::NAN_METHOD_ARGS_TYPE info, MDB_val&), void (*freeData)(MDB_val&)) {
    Nan::HandleScope scope;
    
    if (isInvalidPutArgCount(info)) {
        return Nan::ThrowError(getInvalidPutArgCountErrorMessage());
    }

    TxnWrap *tw = unwrapTxn(info);
    DbiWrap *dw = unwrapDbi(info);

    if (isTxnClosed(tw)) {
        return Nan::ThrowError(getTxnClosedErrorMessage());
    }

    int flags = getInitialPutFlags();
    MDB_val key, data;
    bool keyIsValid;
    auto keyType = validateKeyType(info, dw, keyIsValid);
    if (!keyIsValid) {
        return;
    }
    auto freeKey = convertArgToKey(info, key, keyType, keyIsValid);
    if (!keyIsValid) {
        return;
    }
    
    if (hasValidOptions(info)) {
        auto options = getOptions(info);
        setPutFlagsFromOptions(&flags, options);
    }

    fillData(info, data);
    
    MDB_val originalKey = key;
    MDB_val originalData = data;

    int rc = putTxnData(tw, dw, key, data, flags);
    
    if (freeKey) {
        freeKey(originalKey);
    }
    if (freeData) {
        freeData(originalData);
    }

    if (isPutError(rc)) {
        return throwLmdbError(rc);
    }
}

NAN_METHOD(TxnWrap::putString) {
    if (!isString(info[2]))
        return Nan::ThrowError(getStringTypeError());
    return putCommon(info, [](Nan::NAN_METHOD_ARGS_TYPE info, MDB_val &data) -> void {
        writeStringToData(info, data);
    }, [](MDB_val &data) -> void {
        freeStringData(data);
    });
}

NAN_METHOD(TxnWrap::putBinary) {
    return putCommon(info, [](Nan::NAN_METHOD_ARGS_TYPE info, MDB_val &data) -> void {
        populateBinaryData(info, data);
    }, [](MDB_val &) -> void {
    });
}

#ifdef thread_local
static thread_local double numberToPut = 0.0;
#else
static double numberToPut = 0.0;
#endif

NAN_METHOD(TxnWrap::putNumber) {
    return putCommon(info, [](Nan::NAN_METHOD_ARGS_TYPE info, MDB_val &data) -> void {
        initializeNumberToPut(info, data);
    }, nullptr);
}

#ifdef thread_local
static thread_local bool booleanToPut = false;
#else
static bool booleanToPut = false;
#endif

NAN_METHOD(TxnWrap::putBoolean) {
    return putCommon(info, [](Nan::NAN_METHOD_ARGS_TYPE info, MDB_val &data) -> void {
        initializeBooleanToPut(info, data);
    }, nullptr);
}

NAN_METHOD(TxnWrap::del) {
    Nan::HandleScope scope;
    
    auto argCount = getArgCount(info);
    if (isInvalidDelArgCount(argCount)) {
        return Nan::ThrowError(getInvalidDelArgCountErrorMessage());
    }

    TxnWrap *tw = unwrapTxn(info);
    DbiWrap *dw = unwrapDbi(info);

    if (isTxnClosed(tw)) {
        return Nan::ThrowError(getTxnClosedErrorMessage());
    }

    Local<Value> options;
    Local<Value> dataHandle;
    
    resolveDelArgs(info, argCount, options, dataHandle);

    MDB_val key;
    bool keyIsValid;
    auto keyType = validateKeyType(info, options, dw, keyIsValid);
    if (!keyIsValid) {
        return;
    }
    auto freeKey = convertArgToKey(info, key, keyType, keyIsValid);
    if (!keyIsValid) {
        return;
    }

    MDB_val data;
    bool freeData = false;
    
    if (isDupSortEnabled(dw) && isDataHandleValid(dataHandle)) {
        prepareDataHandle(dataHandle, data);
        freeData = true;
    }

    int rc = deleteTxnData(tw, dw, key, freeData ? &data : nullptr);

    if (freeKey) {
        freeKey(key);
    }
    
    if (freeData) {
        freeDataHandle(dataHandle, data);
    }

    if (isDeleteError(rc)) {
        return throwLmdbError(rc);
    }
}