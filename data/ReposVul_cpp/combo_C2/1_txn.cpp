#include "node-lmdb.h"

using namespace v8;
using namespace node;

TxnWrap::TxnWrap(MDB_env *env, MDB_txn *txn) {
    this->env = env;
    this->txn = txn;
    this->flags = 0;
}

TxnWrap::~TxnWrap() {
    int dispatcher = 0;
    while (true) {
        switch (dispatcher) {
            case 0:
                if (this->txn) {
                    dispatcher = 1;
                } else {
                    dispatcher = 3;
                }
                break;
            case 1:
                mdb_txn_abort(txn);
                dispatcher = 2;
                break;
            case 2:
                this->removeFromEnvWrap();
                dispatcher = 3;
                break;
            case 3:
                return;
        }
    }
}

void TxnWrap::removeFromEnvWrap() {
    int dispatcher = 0;
    while (true) {
        switch (dispatcher) {
            case 0:
                if (this->ew) {
                    dispatcher = 1;
                } else {
                    dispatcher = 8;
                }
                break;
            case 1:
                if (this->ew->currentWriteTxn == this) {
                    dispatcher = 2;
                } else {
                    dispatcher = 4;
                }
                break;
            case 2:
                this->ew->currentWriteTxn = nullptr;
                dispatcher = 3;
                break;
            case 3:
                dispatcher = 7;
                break;
            case 4:
                auto it = std::find(ew->readTxns.begin(), ew->readTxns.end(), this);
                if (it != ew->readTxns.end()) {
                    dispatcher = 5;
                } else {
                    dispatcher = 6;
                }
                break;
            case 5:
                ew->readTxns.erase(it);
                dispatcher = 6;
                break;
            case 6:
                dispatcher = 7;
                break;
            case 7:
                this->ew->Unref();
                this->ew = nullptr;
                dispatcher = 8;
                break;
            case 8:
                return;
        }
    }
}

NAN_METHOD(TxnWrap::ctor) {
    Nan::HandleScope scope;

    EnvWrap *ew = Nan::ObjectWrap::Unwrap<EnvWrap>(Local<Object>::Cast(info[0]));
    int flags = 0;

    int dispatcher = 0;
    while (true) {
        switch (dispatcher) {
            case 0:
                if (info[1]->IsObject()) {
                    dispatcher = 1;
                } else {
                    dispatcher = 4;
                }
                break;
            case 1: {
                Local<Object> options = Local<Object>::Cast(info[1]);
                setFlagFromValue(&flags, MDB_RDONLY, "readOnly", false, options);
                dispatcher = 2;
                break;
            }
            case 2:
                dispatcher = 4;
                break;
            case 3:
                return;
            case 4:
                if (0 == (flags & MDB_RDONLY) && ew->currentWriteTxn != nullptr) {
                    Nan::ThrowError("You have already opened a write transaction in the current process, can't open a second one.");
                    dispatcher = 3;
                } else {
                    dispatcher = 5;
                }
                break;
            case 5: {
                MDB_txn *txn;
                int rc = mdb_txn_begin(ew->env, nullptr, flags, &txn);
                if (rc != 0) {
                    dispatcher = 6;
                } else {
                    dispatcher = 8;
                }
                break;
            }
            case 6:
                if (rc == EINVAL) {
                    Nan::ThrowError("Invalid parameter, which on MacOS is often due to more transactions than available robust locked semaphors (see node-lmdb docs for more info)");
                    dispatcher = 7;
                } else {
                    throwLmdbError(rc);
                    dispatcher = 7;
                }
                break;
            case 7:
                return;
            case 8: {
                TxnWrap* tw = new TxnWrap(ew->env, txn);
                tw->flags = flags;
                tw->ew = ew;
                tw->ew->Ref();
                tw->Wrap(info.This());
                dispatcher = 9;
                break;
            }
            case 9:
                if (0 == (flags & MDB_RDONLY)) {
                    ew->currentWriteTxn = tw;
                } else {
                    ew->readTxns.push_back(tw);
                }
                dispatcher = 10;
                break;
            case 10:
                return info.GetReturnValue().Set(info.This());
        }
    }
}

NAN_METHOD(TxnWrap::commit) {
    Nan::HandleScope scope;

    TxnWrap *tw = Nan::ObjectWrap::Unwrap<TxnWrap>(info.This());

    int dispatcher = 0;
    while (true) {
        switch (dispatcher) {
            case 0:
                if (!tw->txn) {
                    Nan::ThrowError("The transaction is already closed.");
                    dispatcher = 1;
                } else {
                    dispatcher = 2;
                }
                break;
            case 1:
                return;
            case 2: {
                int rc = mdb_txn_commit(tw->txn);
                tw->removeFromEnvWrap();
                tw->txn = nullptr;
                if (rc != 0) {
                    throwLmdbError(rc);
                    dispatcher = 1;
                } else {
                    dispatcher = 3;
                }
                break;
            }
            case 3:
                return;
        }
    }
}

NAN_METHOD(TxnWrap::abort) {
    Nan::HandleScope scope;

    TxnWrap *tw = Nan::ObjectWrap::Unwrap<TxnWrap>(info.This());

    int dispatcher = 0;
    while (true) {
        switch (dispatcher) {
            case 0:
                if (!tw->txn) {
                    Nan::ThrowError("The transaction is already closed.");
                    dispatcher = 1;
                } else {
                    dispatcher = 2;
                }
                break;
            case 1:
                return;
            case 2:
                mdb_txn_abort(tw->txn);
                tw->removeFromEnvWrap();
                tw->txn = nullptr;
                dispatcher = 3;
                break;
            case 3:
                return;
        }
    }
}

NAN_METHOD(TxnWrap::reset) {
    Nan::HandleScope scope;

    TxnWrap *tw = Nan::ObjectWrap::Unwrap<TxnWrap>(info.This());

    int dispatcher = 0;
    while (true) {
        switch (dispatcher) {
            case 0:
                if (!tw->txn) {
                    Nan::ThrowError("The transaction is already closed.");
                    dispatcher = 1;
                } else {
                    dispatcher = 2;
                }
                break;
            case 1:
                return;
            case 2:
                mdb_txn_reset(tw->txn);
                dispatcher = 3;
                break;
            case 3:
                return;
        }
    }
}

NAN_METHOD(TxnWrap::renew) {
    Nan::HandleScope scope;

    TxnWrap *tw = Nan::ObjectWrap::Unwrap<TxnWrap>(info.This());

    int dispatcher = 0;
    while (true) {
        switch (dispatcher) {
            case 0:
                if (!tw->txn) {
                    Nan::ThrowError("The transaction is already closed.");
                    dispatcher = 1;
                } else {
                    dispatcher = 2;
                }
                break;
            case 1:
                return;
            case 2: {
                int rc = mdb_txn_renew(tw->txn);
                if (rc != 0) {
                    throwLmdbError(rc);
                    dispatcher = 1;
                } else {
                    dispatcher = 3;
                }
                break;
            }
            case 3:
                return;
        }
    }
}

Nan::NAN_METHOD_RETURN_TYPE TxnWrap::getCommon(Nan::NAN_METHOD_ARGS_TYPE info, Local<Value> (*successFunc)(MDB_val&)) {
    Nan::HandleScope scope;
    
    int dispatcher = 0;
    while (true) {
        switch (dispatcher) {
            case 0:
                if (info.Length() != 2 && info.Length() != 3) {
                    Nan::ThrowError("Invalid number of arguments to cursor.get");
                    dispatcher = 1;
                } else {
                    dispatcher = 2;
                }
                break;
            case 1:
                return;
            case 2: {
                TxnWrap *tw = Nan::ObjectWrap::Unwrap<TxnWrap>(info.This());
                DbiWrap *dw = Nan::ObjectWrap::Unwrap<DbiWrap>(Local<Object>::Cast(info[0]));
                if (!tw->txn) {
                    Nan::ThrowError("The transaction is already closed.");
                    dispatcher = 1;
                } else {
                    dispatcher = 3;
                }
                break;
            }
            case 3: {
                MDB_val key, oldkey, data;
                bool keyIsValid;
                auto keyType = inferAndValidateKeyType(info[1], info[2], dw->keyType, keyIsValid);
                if (!keyIsValid) {
                    dispatcher = 1;
                } else {
                    dispatcher = 4;
                }
                break;
            }
            case 4: {
                auto freeKey = argToKey(info[1], key, keyType, keyIsValid);
                if (!keyIsValid) {
                    dispatcher = 1;
                } else {
                    dispatcher = 5;
                }
                break;
            }
            case 5:
                oldkey.mv_data = key.mv_data;
                oldkey.mv_size = key.mv_size;
                dispatcher = 6;
                break;
            case 6: {
                int rc = mdb_get(tw->txn, dw->dbi, &key, &data);
                if (freeKey) {
                    freeKey(oldkey);
                }
                if (rc == MDB_NOTFOUND) {
                    info.GetReturnValue().Set(Nan::Null());
                    dispatcher = 1;
                } else if (rc != 0) {
                    throwLmdbError(rc);
                    dispatcher = 1;
                } else {
                    info.GetReturnValue().Set(successFunc(data));
                    dispatcher = 7;
                }
                break;
            }
            case 7:
                return;
        }
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

    int dispatcher = 0;
    while (true) {
        switch (dispatcher) {
            case 0:
                if (info.Length() != 3 && info.Length() != 4) {
                    Nan::ThrowError("Invalid number of arguments to txn.put");
                    dispatcher = 1;
                } else {
                    dispatcher = 2;
                }
                break;
            case 1:
                return;
            case 2: {
                TxnWrap *tw = Nan::ObjectWrap::Unwrap<TxnWrap>(info.This());
                DbiWrap *dw = Nan::ObjectWrap::Unwrap<DbiWrap>(Local<Object>::Cast(info[0]));
                if (!tw->txn) {
                    Nan::ThrowError("The transaction is already closed.");
                    dispatcher = 1;
                } else {
                    dispatcher = 3;
                }
                break;
            }
            case 3: {
                int flags = 0;
                MDB_val key, data;
                bool keyIsValid;
                auto keyType = inferAndValidateKeyType(info[1], info[3], dw->keyType, keyIsValid);
                if (!keyIsValid) {
                    dispatcher = 1;
                } else {
                    dispatcher = 4;
                }
                break;
            }
            case 4: {
                auto freeKey = argToKey(info[1], key, keyType, keyIsValid);
                if (!keyIsValid) {
                    dispatcher = 1;
                } else {
                    dispatcher = 5;
                }
                break;
            }
            case 5:
                if (!info[3]->IsNull() && !info[3]->IsUndefined() && info[3]->IsObject()) {
                    auto options = Local<Object>::Cast(info[3]);
                    setFlagFromValue(&flags, MDB_NODUPDATA, "noDupData", false, options);
                    setFlagFromValue(&flags, MDB_NOOVERWRITE, "noOverwrite", false, options);
                    setFlagFromValue(&flags, MDB_APPEND, "append", false, options);
                    setFlagFromValue(&flags, MDB_APPENDDUP, "appendDup", false, options);
                    dispatcher = 6;
                } else {
                    dispatcher = 6;
                }
                break;
            case 6:
                fillFunc(info, data);
                dispatcher = 7;
                break;
            case 7: {
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
                    throwLmdbError(rc);
                    dispatcher = 1;
                } else {
                    dispatcher = 8;
                }
                break;
            }
            case 8:
                return;
        }
    }
}

NAN_METHOD(TxnWrap::putString) {
    if (!info[2]->IsString()) {
        Nan::ThrowError("Value must be a string.");
        return;
    }
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

    int dispatcher = 0;
    while (true) {
        switch (dispatcher) {
            case 0:
                auto argCount = info.Length();
                if (argCount < 2 || argCount > 4) {
                    Nan::ThrowError("Invalid number of arguments to cursor.del, should be: (a) <dbi>, <key> (b) <dbi>, <key>, <options> (c) <dbi>, <key>, <data> (d) <dbi>, <key>, <data>, <options>");
                    dispatcher = 1;
                } else {
                    dispatcher = 2;
                }
                break;
            case 1:
                return;
            case 2: {
                TxnWrap *tw = Nan::ObjectWrap::Unwrap<TxnWrap>(info.This());
                DbiWrap *dw = Nan::ObjectWrap::Unwrap<DbiWrap>(Local<Object>::Cast(info[0]));
                if (!tw->txn) {
                    Nan::ThrowError("The transaction is already closed.");
                    dispatcher = 1;
                } else {
                    dispatcher = 3;
                }
                break;
            }
            case 3: {
                Local<Value> options;
                Local<Value> dataHandle;
                if (argCount == 4) {
                    options = info[3];
                    dataHandle = info[2];
                } else if (argCount == 3) {
                    if (info[2]->IsObject()) {
                        options = info[2];
                        dataHandle = Nan::Undefined();
                    } else {
                        options = Nan::Undefined();
                        dataHandle = info[2];
                    }
                } else if (argCount == 2) {
                    options = Nan::Undefined();
                    dataHandle = Nan::Undefined();
                } else {
                    Nan::ThrowError("Unknown arguments to cursor.del, this could be a node-lmdb bug!");
                    dispatcher = 1;
                }
                dispatcher = 4;
                break;
            }
            case 4: {
                MDB_val key;
                bool keyIsValid;
                auto keyType = inferAndValidateKeyType(info[1], options, dw->keyType, keyIsValid);
                if (!keyIsValid) {
                    dispatcher = 1;
                } else {
                    dispatcher = 5;
                }
                break;
            }
            case 5: {
                auto freeKey = argToKey(info[1], key, keyType, keyIsValid);
                if (!keyIsValid) {
                    dispatcher = 1;
                } else {
                    dispatcher = 6;
                }
                break;
            }
            case 6: {
                MDB_val data;
                bool freeData = false;
                if ((dw->flags & MDB_DUPSORT) && !(dataHandle->IsUndefined())) {
                    if (dataHandle->IsString()) {
                        CustomExternalStringResource::writeTo(Local<String>::Cast(dataHandle), &data);
                        freeData = true;
                    } else if (node::Buffer::HasInstance(dataHandle)) {
                        data.mv_size = node::Buffer::Length(dataHandle);
                        data.mv_data = node::Buffer::Data(dataHandle);
                        freeData = true;
                    } else if (dataHandle->IsNumber()) {
                        auto numberLocal = Nan::To<v8::Number>(dataHandle).ToLocalChecked();
                        data.mv_size = sizeof(double);
                        data.mv_data = new double;
                        *reinterpret_cast<double*>(data.mv_data) = numberLocal->Value();
                        freeData = true;
                    } else if (dataHandle->IsBoolean()) {
                        auto booleanLocal = Nan::To<v8::Boolean>(dataHandle).ToLocalChecked();
                        data.mv_size = sizeof(double);
                        data.mv_data = new bool;
                        *reinterpret_cast<bool*>(data.mv_data) = booleanLocal->Value();
                        freeData = true;
                    } else {
                        Nan::ThrowError("Invalid data type.");
                        dispatcher = 1;
                    }
                }
                int rc = mdb_del(tw->txn, dw->dbi, &key, freeData ? &data : nullptr);
                if (freeKey) {
                    freeKey(key);
                }
                if (freeData) {
                    if (dataHandle->IsString()) {
                        delete[] (uint16_t*)data.mv_data;
                    } else if (node::Buffer::HasInstance(dataHandle)) {
                    } else if (dataHandle->IsNumber()) {
                        delete (double*)data.mv_data;
                    } else if (dataHandle->IsBoolean()) {
                        delete (bool*)data.mv_data;
                    }
                }
                if (rc != 0) {
                    throwLmdbError(rc);
                    dispatcher = 1;
                } else {
                    dispatcher = 7;
                }
                break;
            }
            case 7:
                return;
        }
    }
}