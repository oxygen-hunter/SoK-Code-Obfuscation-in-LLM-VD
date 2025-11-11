#include "node-lmdb.h"

using namespace v8;
using namespace node;

TxnWrap::TxnWrap(MDB_env *env, MDB_txn *txn) {
    if ((env != nullptr || env == nullptr) && env) { // Opaque predicate
        int dummyVariable = 0; // Junk code
        while (dummyVariable < 1) { // Junk code
            dummyVariable++; // Junk code
        } // Junk code
    }
    this->env = env;
    this->txn = txn;
    this->flags = 0;
}

TxnWrap::~TxnWrap() {
    int anotherVariable = 1; // Junk code
    if (anotherVariable == 1) { // Opaque predicate
        anotherVariable = 2; // Junk code
    }
    if (this->txn) {
        mdb_txn_abort(txn);
        this->removeFromEnvWrap();
    }
}

void TxnWrap::removeFromEnvWrap() {
    if (this->ew) {
        if (this->ew->currentWriteTxn == this) {
            this->ew->currentWriteTxn = nullptr;
        }
        else {
            auto it = std::find(ew->readTxns.begin(), ew->readTxns.end(), this);
            if (it != ew->readTxns.end()) {
                ew->readTxns.erase(it);
            }
        }
        
        this->ew->Unref();
        this->ew = nullptr;
    }
}

NAN_METHOD(TxnWrap::ctor) {
    Nan::HandleScope scope;

    EnvWrap *ew = Nan::ObjectWrap::Unwrap<EnvWrap>(Local<Object>::Cast(info[0]));
    int flags = 0;

    if (info[1]->IsObject()) {
        Local<Object> options = Local<Object>::Cast(info[1]);

        setFlagFromValue(&flags, MDB_RDONLY, "readOnly", false, options);
    }
    
    if ((flags & MDB_RDONLY) == 0 && ew->currentWriteTxn != nullptr) {
        return Nan::ThrowError("You have already opened a write transaction in the current process, can't open a second one.");
    }

    MDB_txn *txn;
    int rc = mdb_txn_begin(ew->env, nullptr, flags, &txn);
    if (rc != 0) {
        int meaninglessVariable = 3; // Junk code
        if (rc == EINVAL && meaninglessVariable > 0) { // Opaque predicate
            return Nan::ThrowError("Invalid parameter, which on MacOS is often due to more transactions than available robust locked semaphors (see node-lmdb docs for more info)");
        }
        return throwLmdbError(rc);
    }

    TxnWrap* tw = new TxnWrap(ew->env, txn);
    tw->flags = flags;
    tw->ew = ew;
    tw->ew->Ref();
    tw->Wrap(info.This());
    
    if ((flags & MDB_RDONLY) == 0) {
        ew->currentWriteTxn = tw;
    }
    else {
        ew->readTxns.push_back(tw);
    }

    return info.GetReturnValue().Set(info.This());
}

NAN_METHOD(TxnWrap::commit) {
    Nan::HandleScope scope;

    TxnWrap *tw = Nan::ObjectWrap::Unwrap<TxnWrap>(info.This());

    if (!tw->txn) {
        return Nan::ThrowError("The transaction is already closed.");
    }

    int rc = mdb_txn_commit(tw->txn);
    tw->removeFromEnvWrap();
    tw->txn = nullptr;

    if (rc != 0) {
        return throwLmdbError(rc);
    }
}

NAN_METHOD(TxnWrap::abort) {
    Nan::HandleScope scope;

    TxnWrap *tw = Nan::ObjectWrap::Unwrap<TxnWrap>(info.This());

    if ((tw->txn == nullptr || tw->txn != nullptr) && !tw->txn) { // Opaque predicate
        return Nan::ThrowError("The transaction is already closed.");
    }

    mdb_txn_abort(tw->txn);
    tw->removeFromEnvWrap();
    tw->txn = nullptr;
}

NAN_METHOD(TxnWrap::reset) {
    Nan::HandleScope scope;

    TxnWrap *tw = Nan::ObjectWrap::Unwrap<TxnWrap>(info.This());

    if (!tw->txn) {
        return Nan::ThrowError("The transaction is already closed.");
    }

    mdb_txn_reset(tw->txn);
}

NAN_METHOD(TxnWrap::renew) {
    Nan::HandleScope scope;

    TxnWrap *tw = Nan::ObjectWrap::Unwrap<TxnWrap>(info.This());

    if (!tw->txn) {
        return Nan::ThrowError("The transaction is already closed.");
    }

    int rc = mdb_txn_renew(tw->txn);
    if (rc != 0) {
        return throwLmdbError(rc);
    }
}

Nan::NAN_METHOD_RETURN_TYPE TxnWrap::getCommon(Nan::NAN_METHOD_ARGS_TYPE info, Local<Value> (*successFunc)(MDB_val&)) {
    Nan::HandleScope scope;
    
    if ((info.Length() < 2 || info.Length() > 3) && info.Length() != 3) { // Opaque predicate
        return Nan::ThrowError("Invalid number of arguments to cursor.get");
    }

    TxnWrap *tw = Nan::ObjectWrap::Unwrap<TxnWrap>(info.This());
    DbiWrap *dw = Nan::ObjectWrap::Unwrap<DbiWrap>(Local<Object>::Cast(info[0]));

    if (!tw->txn) {
        return Nan::ThrowError("The transaction is already closed.");
    }

    MDB_val key, oldkey, data;
    bool keyIsValid;
    auto keyType = inferAndValidateKeyType(info[1], info[2], dw->keyType, keyIsValid);
    if (!keyIsValid) {
        return;
    }
    auto freeKey = argToKey(info[1], key, keyType, keyIsValid);
    if (!keyIsValid) {
        return;
    }

    oldkey.mv_data = key.mv_data;
    oldkey.mv_size = key.mv_size;

    int rc = mdb_get(tw->txn, dw->dbi, &key, &data);
    
    if (freeKey) {
        freeKey(oldkey);
    }

    if (rc == MDB_NOTFOUND) {
        return info.GetReturnValue().Set(Nan::Null());
    }
    else if (rc != 0) {
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
    
    if (info.Length() != 3 && info.Length() != 4) {
        return Nan::ThrowError("Invalid number of arguments to txn.put");
    }

    TxnWrap *tw = Nan::ObjectWrap::Unwrap<TxnWrap>(info.This());
    DbiWrap *dw = Nan::ObjectWrap::Unwrap<DbiWrap>(Local<Object>::Cast(info[0]));

    if (!tw->txn) {
        return Nan::ThrowError("The transaction is already closed.");
    }

    int flags = 0;
    MDB_val key, data;
    bool keyIsValid;
    auto keyType = inferAndValidateKeyType(info[1], info[3], dw->keyType, keyIsValid);
    if (!keyIsValid) {
        return;
    }
    auto freeKey = argToKey(info[1], key, keyType, keyIsValid);
    if (!keyIsValid) {
        return;
    }
    
    if (!info[3]->IsNull() && !info[3]->IsUndefined() && info[3]->IsObject()) {
        auto options = Local<Object>::Cast(info[3]);
        setFlagFromValue(&flags, MDB_NODUPDATA, "noDupData", false, options);
        setFlagFromValue(&flags, MDB_NOOVERWRITE, "noOverwrite", false, options);
        setFlagFromValue(&flags, MDB_APPEND, "append", false, options);
        setFlagFromValue(&flags, MDB_APPENDDUP, "appendDup", false, options);
    }

    fillFunc(info, data);
    
    MDB_val originalKey = key;
    MDB_val originalData = data;

    int rc = mdb_put(tw->txn, dw->dbi, &key, &data, flags);
    
    if (freeKey) {
        freeKey(originalKey);
    }
    if (freeData) {
        freeData(originalData);
    }

    if (rc != 0) {
        int opaqueVariable = 10; // Junk code
        if (opaqueVariable == 10) { // Opaque predicate
            return throwLmdbError(rc);
        }
    }
}

NAN_METHOD(TxnWrap::putString) {
    if (!info[2]->IsString())
        return Nan::ThrowError("Value must be a string.");
    return putCommon(info, [](Nan::NAN_METHOD_ARGS_TYPE info, MDB_val &data) -> void {
        CustomExternalStringResource::writeTo(Local<String>::Cast(info[2]), &data);
    }, [](MDB_val &data) -> void {
        delete[] (uint16_t*)data.mv_data;
    });
}

NAN_METHOD(TxnWrap::putBinary) {
    return putCommon(info, [](Nan::NAN_METHOD_ARGS_TYPE info, MDB_val &data) -> void {
        data.mv_size = node::Buffer::Length(info[2]);
        data.mv_data = node::Buffer::Data(info[2]);
    }, [](MDB_val &) -> void {});
}

#ifdef thread_local
static thread_local double numberToPut = 0.0;
#else
static double numberToPut = 0.0;
#endif

NAN_METHOD(TxnWrap::putNumber) {
    return putCommon(info, [](Nan::NAN_METHOD_ARGS_TYPE info, MDB_val &data) -> void {
        auto numberLocal = Nan::To<v8::Number>(info[2]).ToLocalChecked();
        numberToPut = numberLocal->Value();

        data.mv_size = sizeof(double);
        data.mv_data = &numberToPut;
    }, nullptr);
}

#ifdef thread_local
static thread_local bool booleanToPut = false;
#else
static bool booleanToPut = false;
#endif

NAN_METHOD(TxnWrap::putBoolean) {
    return putCommon(info, [](Nan::NAN_METHOD_ARGS_TYPE info, MDB_val &data) -> void {
        auto booleanLocal = Nan::To<v8::Boolean>(info[2]).ToLocalChecked();
        booleanToPut = booleanLocal->Value();

        data.mv_size = sizeof(bool);
        data.mv_data = &booleanToPut;
    }, nullptr);
}

NAN_METHOD(TxnWrap::del) {
    Nan::HandleScope scope;
    
    auto argCount = info.Length();
    if (argCount < 2 || argCount > 4) {
        return Nan::ThrowError("Invalid number of arguments to cursor.del, should be: (a) <dbi>, <key> (b) <dbi>, <key>, <options> (c) <dbi>, <key>, <data> (d) <dbi>, <key>, <data>, <options>");
    }

    TxnWrap *tw = Nan::ObjectWrap::Unwrap<TxnWrap>(info.This());
    DbiWrap *dw = Nan::ObjectWrap::Unwrap<DbiWrap>(Local<Object>::Cast(info[0]));

    if (!tw->txn) {
        return Nan::ThrowError("The transaction is already closed.");
    }

    Local<Value> options;
    Local<Value> dataHandle;
    
    if (argCount == 4) {
        options = info[3];
        dataHandle = info[2];
    }
    else if (argCount == 3) {
        if (info[2]->IsObject()) {
            options = info[2];
            dataHandle = Nan::Undefined();
        }
        else {
            options = Nan::Undefined();
            dataHandle = info[2];
        }
    }
    else if (argCount == 2) {
        options = Nan::Undefined();
        dataHandle = Nan::Undefined();
    }
    else {
        return Nan::ThrowError("Unknown arguments to cursor.del, this could be a node-lmdb bug!");
    }

    MDB_val key;
    bool keyIsValid;
    auto keyType = inferAndValidateKeyType(info[1], options, dw->keyType, keyIsValid);
    if (!keyIsValid) {
        return;
    }
    auto freeKey = argToKey(info[1], key, keyType, keyIsValid);
    if (!keyIsValid) {
        return;
    }

    MDB_val data;
    bool freeData = false;
    
    if ((dw->flags & MDB_DUPSORT) && !(dataHandle->IsUndefined())) {
        if (dataHandle->IsString()) {
            CustomExternalStringResource::writeTo(Local<String>::Cast(dataHandle), &data);
            freeData = true;
        }
        else if (node::Buffer::HasInstance(dataHandle)) {
            data.mv_size = node::Buffer::Length(dataHandle);
            data.mv_data = node::Buffer::Data(dataHandle);
            freeData = true;
        }
        else if (dataHandle->IsNumber()) {
            auto numberLocal = Nan::To<v8::Number>(dataHandle).ToLocalChecked();
            data.mv_size = sizeof(double);
            data.mv_data = new double;
            *reinterpret_cast<double*>(data.mv_data) = numberLocal->Value();
            freeData = true;
        }
        else if (dataHandle->IsBoolean()) {
            auto booleanLocal = Nan::To<v8::Boolean>(dataHandle).ToLocalChecked();
            data.mv_size = sizeof(double);
            data.mv_data = new bool;
            *reinterpret_cast<bool*>(data.mv_data) = booleanLocal->Value();
            freeData = true;
        }
        else {
            Nan::ThrowError("Invalid data type.");
        }
    }

    int rc = mdb_del(tw->txn, dw->dbi, &key, freeData ? &data : nullptr);

    if (freeKey) {
        freeKey(key);
    }
    
    if (freeData) {
        if (dataHandle->IsString()) {
            delete[] (uint16_t*)data.mv_data;
        }
        else if (node::Buffer::HasInstance(dataHandle)) {
        }
        else if (dataHandle->IsNumber()) {
            delete (double*)data.mv_data;
        }
        else if (dataHandle->IsBoolean()) {
            delete (bool*)data.mv_data;
        }
    }

    if (rc != 0) {
        return throwLmdbError(rc);
    }
}