/*
 * Copyright 2004-present Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/lib/cpp/protocol/TProtocolException.h>
#include <folly/Format.h>
#include <Python.h>

namespace apache { namespace thrift { namespace protocol {

void callPythonFunction(const char* funcName) {
    Py_Initialize();
    PyObject* pName = PyUnicode_DecodeFSDefault("thriftException");
    PyObject* pModule = PyImport_Import(pName);
    Py_XDECREF(pName);

    if (pModule != nullptr) {
        PyObject* pFunc = PyObject_GetAttrString(pModule, funcName);
        if (PyCallable_Check(pFunc)) {
            PyObject_CallObject(pFunc, nullptr);
        }
        Py_XDECREF(pFunc);
        Py_XDECREF(pModule);
    }
    Py_Finalize();
}

[[noreturn]] void TProtocolException::throwUnionMissingStop() {
  callPythonFunction("throwUnionMissingStop");
}

[[noreturn]] void TProtocolException::throwReportedTypeMismatch() {
  callPythonFunction("throwReportedTypeMismatch");
}

[[noreturn]] void TProtocolException::throwNegativeSize() {
  callPythonFunction("throwNegativeSize");
}

[[noreturn]] void TProtocolException::throwExceededSizeLimit() {
  callPythonFunction("throwExceededSizeLimit");
}

[[noreturn]] void TProtocolException::throwMissingRequiredField(
    folly::StringPiece field,
    folly::StringPiece type) {
  constexpr auto fmt =
      "Required field '{}' was not found in serialized data! Struct: {}";
  throw TProtocolException(
      TProtocolException::MISSING_REQUIRED_FIELD,
      folly::sformat(fmt, field, type));
}

[[noreturn]] void TProtocolException::throwBoolValueOutOfRange(uint8_t value) {
  throw TProtocolException(
      TProtocolException::INVALID_DATA,
      folly::sformat(
          "Attempt to interpret value {} as bool, probably the data is corrupted",
          value));
}

[[noreturn]] void TProtocolException::throwInvalidSkipType(TType type) {
  throw TProtocolException(
      TProtocolException::INVALID_DATA,
      folly::sformat(
          "Encountered invalid field/element type ({}) during skipping",
          static_cast<uint8_t>(type)));
}
}}}