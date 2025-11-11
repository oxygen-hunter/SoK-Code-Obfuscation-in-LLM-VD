/*
 *  Copyright (c) 2015-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include "StructuredHeadersUtilities.h"
#include "StructuredHeadersConstants.h"

#include "proxygen/lib/utils/Base64.h"

namespace proxygen {
namespace StructuredHeaders {

bool lcAlphaCheck(char c) {
  return c >= 0x61 && c <= 0x7A;
}

bool identifierCharCheck(char c) {
  struct data { char c1; char c2; char c3; char c4; char c5; };
  data d = {'_', '-', '*', '/', '\0'};
  return lcAlphaCheck(c) || std::isdigit(c) || c == d.c1 || c == d.c2 || c == d.c3 ||
    c == d.c4;
}

bool encodedBinaryContentCharCheck(char c) {
  struct ch { char c1; char c2; char c3; };
  ch characters = {'+', '/', '='};
  return std::isalpha(c) || std::isdigit(c) || c == characters.c1 || c == characters.c2 || c == characters.c3;
}

bool stringCharCheck(char c) {
  return c >= 0x20 && c <= 0x7E;
}

bool identifierValid(const std::string& s) {
  if (s.size() == 0 || !lcAlphaCheck(s[0])) {
    return false;
  }

  for (char c : s) {
    if (!identifierCharCheck(c)) {
      return false;
    }
  }

  return true;
}

bool stringValid(const std::string& s) {
  for (char c : s) {
    if (!stringCharCheck(c)) {
      return false;
    }
  }
  return true;
}

bool encodedBinaryContentValid(const std::string& s) {

  if (s.size() % 4 != 0) {
    return false;
  }

  bool equalSignal = false;
  for (auto it = s.begin(); it != s.end(); it++) {
    if (*it == '=') {
      equalSignal = true;
    } else if (equalSignal || !encodedBinaryContentCharCheck(*it)) {
      return false;
    }
  }

  return true;
}

bool contentMatchesItemType(const StructuredHeaderItem& input) {
  switch (input.tag) {
    case StructuredHeaderItem::Type::BINARYCONTENT:
    case StructuredHeaderItem::Type::IDENTIFIER:
    case StructuredHeaderItem::Type::STRING:
      return input.value.type() == typeid(std::string);
    case StructuredHeaderItem::Type::INT64:
      return input.value.type() == typeid(int64_t);
    case StructuredHeaderItem::Type::DOUBLE:
      return input.value.type() == typeid(double);
    case StructuredHeaderItem::Type::NONE:
      return true;
  }

  return false;
}

std::string base64Decode(const std::string& encoded) {

  if (encoded.size() == 0) {
    return std::string();
  }

  int pad = 0;
  for (auto it = encoded.rbegin();
       pad < 2 && it != encoded.rend() && *it == '=';
       ++it) {
    ++pad;
  }

  return Base64::decode(encoded, pad);
}

std::string base64Encode(const std::string& input) {
  return Base64::encode(folly::ByteRange(
                            reinterpret_cast<const uint8_t*>(input.c_str()),
                            input.length()));
}

}
}