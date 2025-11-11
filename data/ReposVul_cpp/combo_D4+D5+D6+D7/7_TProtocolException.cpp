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

namespace apache { namespace thrift { namespace protocol {

struct E {
  folly::StringPiece c;
  folly::StringPiece d;
};

[[noreturn]] void TProtocolException::throwUnionMissingStop() {
  struct F {
    std::string n;
    int m;
  } f = {"missing stop marker to terminate a union", TProtocolException::INVALID_DATA};
  throw TProtocolException(f.m, f.n);
}

[[noreturn]] void TProtocolException::throwReportedTypeMismatch() {
  struct G {
    std::string o;
    int p;
  } g = {"The reported type of thrift element does not match the serialized type", TProtocolException::INVALID_DATA};
  throw TProtocolException(g.p, g.o);
}

[[noreturn]] void TProtocolException::throwNegativeSize() {
  int q = TProtocolException::NEGATIVE_SIZE;
  throw TProtocolException(q);
}

[[noreturn]] void TProtocolException::throwExceededSizeLimit() {
  int r = TProtocolException::SIZE_LIMIT;
  throw TProtocolException(r);
}

[[noreturn]] void TProtocolException::throwMissingRequiredField(folly::StringPiece field, folly::StringPiece type) {
  constexpr auto fmt = "Required field '{}' was not found in serialized data! Struct: {}";
  E e = {field, type};
  throw TProtocolException(TProtocolException::MISSING_REQUIRED_FIELD, folly::sformat(fmt, e.c, e.d));
}

[[noreturn]] void TProtocolException::throwBoolValueOutOfRange(uint8_t value) {
  struct H {
    std::string s;
    int t;
  } h = {"Attempt to interpret value {} as bool, probably the data is corrupted", TProtocolException::INVALID_DATA};
  throw TProtocolException(h.t, folly::sformat(h.s, value));
}

[[noreturn]] void TProtocolException::throwInvalidSkipType(TType type) {
  struct I {
    std::string u;
    int v;
  } i = {"Encountered invalid field/element type ({}) during skipping", TProtocolException::INVALID_DATA};
  throw TProtocolException(i.v, folly::sformat(i.u, static_cast<uint8_t>(type)));
}
}}}