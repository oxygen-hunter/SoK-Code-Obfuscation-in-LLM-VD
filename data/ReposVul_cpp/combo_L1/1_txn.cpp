#include "node-lmdb.h"

using namespace v8;
using namespace node;

OX7B4DF339::OX7B4DF339(MDB_env *OX7A8B3870, MDB_txn *OX1C8B4D0A) {
    this->OX7A8B3870 = OX7A8B3870;
    this->OX1C8B4D0A = OX1C8B4D0A;
    this->OX1C8B6364 = 0;
}

OX7B4DF339::~OX7B4DF339() {
    if (this->OX1C8B4D0A) {
        mdb_txn_abort(OX1C8B4D0A);
        this->OX1C8B4D0A5();
    }
}

void OX7B4DF339::OX1C8B4D0A5() {
    if (this->OX1C8B4D0A9) {
        if (this->OX1C8B4D0A9->OX1C8B4D0A6 == this) {
            this->OX1C8B4D0A9->OX1C8B4D0A6 = nullptr;
        } else {
            auto OX1C8B4D0A7 = std::find(OX1C8B4D0A9->OX1C8B4D0A8.begin(), OX1C8B4D0A9->OX1C8B4D0A8.end(), this);
            if (OX1C8B4D0A7 != OX1C8B4D0A9->OX1C8B4D0A8.end()) {
                OX1C8B4D0A9->OX1C8B4D0A8.erase(OX1C8B4D0A7);
            }
        }
        
        this->OX1C8B4D0A9->Unref();
        this->OX1C8B4D0A9 = nullptr;
    }
}

NAN_METHOD(OX7B4DF339::OX1C8B4D0A1) {
    Nan::HandleScope OX1C8B4D0A2;

    OX1C8B4D0AD *OX1C8B4D0A9 = Nan::ObjectWrap::Unwrap<OX1C8B4D0AD>(Local<Object>::Cast(info[0]));
    int OX1C8B6364 = 0;

    if (info[1]->IsObject()) {
        Local<Object> OX1C8B4D0AA = Local<Object>::Cast(info[1]);

        setFlagFromValue(&OX1C8B6364, MDB_RDONLY, "readOnly", false, OX1C8B4D0AA);
    }
    
    if (0 == (OX1C8B6364 & MDB_RDONLY) && OX1C8B4D0A9->OX1C8B4D0A6 != nullptr) {
        return Nan::ThrowError("You have already opened a write transaction in the current process, can't open a second one.");
    }

    MDB_txn *OX1C8B4D0A;
    int OX7A8B3870 = mdb_txn_begin(OX1C8B4D0A9->OX7A8B3870, nullptr, OX1C8B6364, &OX1C8B4D0A);
    if (OX7A8B3870 != 0) {
        if (OX7A8B3870 == EINVAL) {
            return Nan::ThrowError("Invalid parameter, which on MacOS is often due to more transactions than available robust locked semaphors (see node-lmdb docs for more info)");
        }
        return throwLmdbError(OX7A8B3870);
    }

    OX7B4DF339* OX1C8B4D0A3 = new OX7B4DF339(OX1C8B4D0A9->OX7A8B3870, OX1C8B4D0A);
    OX1C8B4D0A3->OX1C8B6364 = OX1C8B6364;
    OX1C8B4D0A3->OX1C8B4D0A9 = OX1C8B4D0A9;
    OX1C8B4D0A3->OX1C8B4D0A9->Ref();
    OX1C8B4D0A3->Wrap(info.This());
    
    if (0 == (OX1C8B6364 & MDB_RDONLY)) {
        OX1C8B4D0A9->OX1C8B4D0A6 = OX1C8B4D0A3;
    } else {
        OX1C8B4D0A9->OX1C8B4D0A8.push_back(OX1C8B4D0A3);
    }

    return info.GetReturnValue().Set(info.This());
}

NAN_METHOD(OX7B4DF339::OX1C8B4D0A4) {
    Nan::HandleScope OX1C8B4D0A2;

    OX7B4DF339 *OX1C8B4D0A3 = Nan::ObjectWrap::Unwrap<OX7B4DF339>(info.This());

    if (!OX1C8B4D0A3->OX1C8B4D0A) {
        return Nan::ThrowError("The transaction is already closed.");
    }

    int OX7A8B3870 = mdb_txn_commit(OX1C8B4D0A3->OX1C8B4D0A);
    OX1C8B4D0A3->OX1C8B4D0A5();
    OX1C8B4D0A3->OX1C8B4D0A = nullptr;

    if (OX7A8B3870 != 0) {
        return throwLmdbError(OX7A8B3870);
    }
}

NAN_METHOD(OX7B4DF339::OX1C8B4D0A6) {
    Nan::HandleScope OX1C8B4D0A2;

    OX7B4DF339 *OX1C8B4D0A3 = Nan::ObjectWrap::Unwrap<OX7B4DF339>(info.This());

    if (!OX1C8B4D0A3->OX1C8B4D0A) {
        return Nan::ThrowError("The transaction is already closed.");
    }

    mdb_txn_abort(OX1C8B4D0A3->OX1C8B4D0A);
    OX1C8B4D0A3->OX1C8B4D0A5();
    OX1C8B4D0A3->OX1C8B4D0A = nullptr;
}

NAN_METHOD(OX7B4DF339::OX1C8B4D0A7) {
    Nan::HandleScope OX1C8B4D0A2;

    OX7B4DF339 *OX1C8B4D0A3 = Nan::ObjectWrap::Unwrap<OX7B4DF339>(info.This());

    if (!OX1C8B4D0A3->OX1C8B4D0A) {
        return Nan::ThrowError("The transaction is already closed.");
    }

    mdb_txn_reset(OX1C8B4D0A3->OX1C8B4D0A);
}

NAN_METHOD(OX7B4DF339::OX1C8B4D0A8) {
    Nan::HandleScope OX1C8B4D0A2;

    OX7B4DF339 *OX1C8B4D0A3 = Nan::ObjectWrap::Unwrap<OX7B4DF339>(info.This());

    if (!OX1C8B4D0A3->OX1C8B4D0A) {
        return Nan::ThrowError("The transaction is already closed.");
    }

    int OX7A8B3870 = mdb_txn_renew(OX1C8B4D0A3->OX1C8B4D0A);
    if (OX7A8B3870 != 0) {
        return throwLmdbError(OX7A8B3870);
    }
}

Nan::NAN_METHOD_RETURN_TYPE OX7B4DF339::OX1C8B4D0A9(Nan::NAN_METHOD_ARGS_TYPE info, Local<Value> (*OX1C8B4D0AA)(MDB_val&)) {
    Nan::HandleScope OX1C8B4D0A2;
    
    if (info.Length() != 2 && info.Length() != 3) {
        return Nan::ThrowError("Invalid number of arguments to cursor.get");
    }

    OX7B4DF339 *OX1C8B4D0A3 = Nan::ObjectWrap::Unwrap<OX7B4DF339>(info.This());
    OX1C8B4D0AB *OX1C8B4D0AC = Nan::ObjectWrap::Unwrap<OX1C8B4D0AB>(Local<Object>::Cast(info[0]));

    if (!OX1C8B4D0A3->OX1C8B4D0A) {
        return Nan::ThrowError("The transaction is already closed.");
    }

    MDB_val OX1C8B4D0AD, OX1C8B4D0AE, OX1C8B4D0AF;
    bool OX1C8B4D0B0;
    auto OX1C8B4D0B1 = inferAndValidateKeyType(info[1], info[2], OX1C8B4D0AC->OX1C8B4D0B2, OX1C8B4D0B0);
    if (!OX1C8B4D0B0) {
        return;
    }
    auto OX1C8B4D0B3 = argToKey(info[1], OX1C8B4D0AD, OX1C8B4D0B1, OX1C8B4D0B0);
    if (!OX1C8B4D0B0) {
        return;
    }

    OX1C8B4D0AE.mv_data = OX1C8B4D0AD.mv_data;
    OX1C8B4D0AE.mv_size = OX1C8B4D0AD.mv_size;

    int OX7A8B3870 = mdb_get(OX1C8B4D0A3->OX1C8B4D0A, OX1C8B4D0AC->OX1C8B4D0B4, &OX1C8B4D0AD, &OX1C8B4D0AF);
    
    if (OX1C8B4D0B3) {
        OX1C8B4D0B3(OX1C8B4D0AE);
    }

    if (OX7A8B3870 == MDB_NOTFOUND) {
        return info.GetReturnValue().Set(Nan::Null());
    }
    else if (OX7A8B3870 != 0) {
        return throwLmdbError(OX7A8B3870);
    }
    else {
      return info.GetReturnValue().Set(OX1C8B4D0AA(OX1C8B4D0AF));
    }
}

NAN_METHOD(OX7B4DF339::OX1C8B4D0B5) {
    return OX1C8B4D0A9(info, valToString);
}

NAN_METHOD(OX7B4DF339::OX1C8B4D0B6) {
    return OX1C8B4D0A9(info, valToStringUnsafe);
}

NAN_METHOD(OX7B4DF339::OX1C8B4D0B7) {
    return OX1C8B4D0A9(info, valToBinary);
}

NAN_METHOD(OX7B4DF339::OX1C8B4D0B8) {
    return OX1C8B4D0A9(info, valToBinaryUnsafe);
}

NAN_METHOD(OX7B4DF339::OX1C8B4D0B9) {
    return OX1C8B4D0A9(info, valToNumber);
}

NAN_METHOD(OX7B4DF339::OX1C8B4D0BA) {
    return OX1C8B4D0A9(info, valToBoolean);
}

Nan::NAN_METHOD_RETURN_TYPE OX7B4DF339::OX1C8B4D0BB(Nan::NAN_METHOD_ARGS_TYPE info, void (*OX1C8B4D0BC)(Nan::NAN_METHOD_ARGS_TYPE info, MDB_val&), void (*OX1C8B4D0BD)(MDB_val&)) {
    Nan::HandleScope OX1C8B4D0A2;
    
    if (info.Length() != 3 && info.Length() != 4) {
        return Nan::ThrowError("Invalid number of arguments to txn.put");
    }

    OX7B4DF339 *OX1C8B4D0A3 = Nan::ObjectWrap::Unwrap<OX7B4DF339>(info.This());
    OX1C8B4D0AB *OX1C8B4D0AC = Nan::ObjectWrap::Unwrap<OX1C8B4D0AB>(Local<Object>::Cast(info[0]));

    if (!OX1C8B4D0A3->OX1C8B4D0A) {
        return Nan::ThrowError("The transaction is already closed.");
    }

    int OX1C8B6364 = 0;
    MDB_val OX1C8B4D0AD, OX1C8B4D0AF;
    bool OX1C8B4D0B0;
    auto OX1C8B4D0B1 = inferAndValidateKeyType(info[1], info[3], OX1C8B4D0AC->OX1C8B4D0B2, OX1C8B4D0B0);
    if (!OX1C8B4D0B0) {
        return;
    }
    auto OX1C8B4D0B3 = argToKey(info[1], OX1C8B4D0AD, OX1C8B4D0B1, OX1C8B4D0B0);
    if (!OX1C8B4D0B0) {
        return;
    }
    
    if (!info[3]->IsNull() && !info[3]->IsUndefined() && info[3]->IsObject()) {
        auto OX1C8B4D0AA = Local<Object>::Cast(info[3]);
        setFlagFromValue(&OX1C8B6364, MDB_NODUPDATA, "noDupData", false, OX1C8B4D0AA);
        setFlagFromValue(&OX1C8B6364, MDB_NOOVERWRITE, "noOverwrite", false, OX1C8B4D0AA);
        setFlagFromValue(&OX1C8B6364, MDB_APPEND, "append", false, OX1C8B4D0AA);
        setFlagFromValue(&OX1C8B6364, MDB_APPENDDUP, "appendDup", false, OX1C8B4D0AA);
    }

    OX1C8B4D0BC(info, OX1C8B4D0AF);
    
    MDB_val OX1C8B4D0BE = OX1C8B4D0AD;
    MDB_val OX1C8B4D0BF = OX1C8B4D0AF;

    int OX7A8B3870 = mdb_put(OX1C8B4D0A3->OX1C8B4D0A, OX1C8B4D0AC->OX1C8B4D0B4, &OX1C8B4D0AD, &OX1C8B4D0AF, OX1C8B6364);
    
    if (OX1C8B4D0B3) {
        OX1C8B4D0B3(OX1C8B4D0BE);
    }
    if (OX1C8B4D0BD) {
        OX1C8B4D0BD(OX1C8B4D0BF);
    }

    if (OX7A8B3870 != 0) {
        return throwLmdbError(OX7A8B3870);
    }
}

NAN_METHOD(OX7B4DF339::OX1C8B4D0C0) {
    if (!info[2]->IsString())
        return Nan::ThrowError("Value must be a string.");
    return OX1C8B4D0BB(info, [](Nan::NAN_METHOD_ARGS_TYPE info, MDB_val &OX1C8B4D0AF) -> void {
        CustomExternalStringResource::writeTo(Local<String>::Cast(info[2]), &OX1C8B4D0AF);
    }, [](MDB_val &OX1C8B4D0AF) -> void {
        delete[] (uint16_t*)OX1C8B4D0AF.mv_data;
    });
}

NAN_METHOD(OX7B4DF339::OX1C8B4D0C1) {
    return OX1C8B4D0BB(info, [](Nan::NAN_METHOD_ARGS_TYPE info, MDB_val &OX1C8B4D0AF) -> void {
        OX1C8B4D0AF.mv_size = node::Buffer::Length(info[2]);
        OX1C8B4D0AF.mv_data = node::Buffer::Data(info[2]);
    }, [](MDB_val &) -> void {
    });
}

#ifdef thread_local
static thread_local double OX1C8B4D0C2 = 0.0;
#else
static double OX1C8B4D0C2 = 0.0;
#endif

NAN_METHOD(OX7B4DF339::OX1C8B4D0C3) {
    return OX1C8B4D0BB(info, [](Nan::NAN_METHOD_ARGS_TYPE info, MDB_val &OX1C8B4D0AF) -> void {
        auto OX1C8B4D0C4 = Nan::To<v8::Number>(info[2]).ToLocalChecked();
        OX1C8B4D0C2 = OX1C8B4D0C4->Value();

        OX1C8B4D0AF.mv_size = sizeof(double);
        OX1C8B4D0AF.mv_data = &OX1C8B4D0C2;
    }, nullptr);
}

#ifdef thread_local
static thread_local bool OX1C8B4D0C5 = false;
#else
static bool OX1C8B4D0C5 = false;
#endif

NAN_METHOD(OX7B4DF339::OX1C8B4D0C6) {
    return OX1C8B4D0BB(info, [](Nan::NAN_METHOD_ARGS_TYPE info, MDB_val &OX1C8B4D0AF) -> void {
        auto OX1C8B4D0C7 = Nan::To<v8::Boolean>(info[2]).ToLocalChecked();
        OX1C8B4D0C5 = OX1C8B4D0C7->Value();

        OX1C8B4D0AF.mv_size = sizeof(bool);
        OX1C8B4D0AF.mv_data = &OX1C8B4D0C5;
    }, nullptr);
}

NAN_METHOD(OX7B4DF339::OX1C8B4D0C8) {
    Nan::HandleScope OX1C8B4D0A2;
    
    auto OX1C8B4D0C9 = info.Length();
    if (OX1C8B4D0C9 < 2 || OX1C8B4D0C9 > 4) {
        return Nan::ThrowError("Invalid number of arguments to cursor.del, should be: (a) <dbi>, <key> (b) <dbi>, <key>, <options> (c) <dbi>, <key>, <data> (d) <dbi>, <key>, <data>, <options>");
    }

    OX7B4DF339 *OX1C8B4D0A3 = Nan::ObjectWrap::Unwrap<OX7B4DF339>(info.This());
    OX1C8B4D0AB *OX1C8B4D0AC = Nan::ObjectWrap::Unwrap<OX1C8B4D0AB>(Local<Object>::Cast(info[0]));

    if (!OX1C8B4D0A3->OX1C8B4D0A) {
        return Nan::ThrowError("The transaction is already closed.");
    }

    Local<Value> OX1C8B4D0AA;
    Local<Value> OX1C8B4D0CA;
    
    if (OX1C8B4D0C9 == 4) {
        OX1C8B4D0AA = info[3];
        OX1C8B4D0CA = info[2];
    }
    else if (OX1C8B4D0C9 == 3) {
        if (info[2]->IsObject()) {
            OX1C8B4D0AA = info[2];
            OX1C8B4D0CA = Nan::Undefined();
        }
        else {
            OX1C8B4D0AA = Nan::Undefined();
            OX1C8B4D0CA = info[2];
        }
    }
    else if (OX1C8B4D0C9 == 2) {
        OX1C8B4D0AA = Nan::Undefined();
        OX1C8B4D0CA = Nan::Undefined();
    }
    else {
        return Nan::ThrowError("Unknown arguments to cursor.del, this could be a node-lmdb bug!");
    }

    MDB_val OX1C8B4D0AD;
    bool OX1C8B4D0B0;
    auto OX1C8B4D0B1 = inferAndValidateKeyType(info[1], OX1C8B4D0AA, OX1C8B4D0AC->OX1C8B4D0B2, OX1C8B4D0B0);
    if (!OX1C8B4D0B0) {
        return;
    }
    auto OX1C8B4D0B3 = argToKey(info[1], OX1C8B4D0AD, OX1C8B4D0B1, OX1C8B4D0B0);
    if (!OX1C8B4D0B0) {
        return;
    }

    MDB_val OX1C8B4D0AF;
    bool OX1C8B4D0CB = false;
    
    if ((OX1C8B4D0AC->OX1C8B6364 & MDB_DUPSORT) && !(OX1C8B4D0CA->IsUndefined())) {
        if (OX1C8B4D0CA->IsString()) {
            CustomExternalStringResource::writeTo(Local<String>::Cast(OX1C8B4D0CA), &OX1C8B4D0AF);
            OX1C8B4D0CB = true;
        }
        else if (node::Buffer::HasInstance(OX1C8B4D0CA)) {
            OX1C8B4D0AF.mv_size = node::Buffer::Length(OX1C8B4D0CA);
            OX1C8B4D0AF.mv_data = node::Buffer::Data(OX1C8B4D0CA);
            OX1C8B4D0CB = true;
        }
        else if (OX1C8B4D0CA->IsNumber()) {
            auto OX1C8B4D0C4 = Nan::To<v8::Number>(OX1C8B4D0CA).ToLocalChecked();
            OX1C8B4D0AF.mv_size = sizeof(double);
            OX1C8B4D0AF.mv_data = new double;
            *reinterpret_cast<double*>(OX1C8B4D0AF.mv_data) = OX1C8B4D0C4->Value();
            OX1C8B4D0CB = true;
        }
        else if (OX1C8B4D0CA->IsBoolean()) {
            auto OX1C8B4D0C7 = Nan::To<v8::Boolean>(OX1C8B4D0CA).ToLocalChecked();
            OX1C8B4D0AF.mv_size = sizeof(double);
            OX1C8B4D0AF.mv_data = new bool;
            *reinterpret_cast<bool*>(OX1C8B4D0AF.mv_data) = OX1C8B4D0C7->Value();
            OX1C8B4D0CB = true;
        }
        else {
            Nan::ThrowError("Invalid data type.");
        }
    }

    int OX7A8B3870 = mdb_del(OX1C8B4D0A3->OX1C8B4D0A, OX1C8B4D0AC->OX1C8B4D0B4, &OX1C8B4D0AD, OX1C8B4D0CB ? &OX1C8B4D0AF : nullptr);

    if (OX1C8B4D0B3) {
        OX1C8B4D0B3(OX1C8B4D0AD);
    }
    
    if (OX1C8B4D0CB) {
        if (OX1C8B4D0CA->IsString()) {
            delete[] (uint16_t*)OX1C8B4D0AF.mv_data;
        }
        else if (node::Buffer::HasInstance(OX1C8B4D0CA)) {
        }
        else if (OX1C8B4D0CA->IsNumber()) {
            delete (double*)OX1C8B4D0AF.mv_data;
        }
        else if (OX1C8B4D0CA->IsBoolean()) {
            delete (bool*)OX1C8B4D0AF.mv_data;
        }
    }

    if (OX7A8B3870 != 0) {
        return throwLmdbError(OX7A8B3870);
    }
}