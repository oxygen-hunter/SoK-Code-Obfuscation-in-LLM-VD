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

bool isLcAlpha(char x) {
  auto low = []() { return 0x61; };
  auto high = []() { return 0x7A; };
  return x >= low() && x <= high();
}

bool isValidIdentifierChar(char y) {
  return isLcAlpha(y) || std::isdigit(y) || y == '_' || y == '-' || y == '*' ||
    y == '/';
}

bool isValidEncodedBinaryContentChar(
   char z) {
  return std::isalpha(z) || std::isdigit(z) || z == '+' || z == '/' || z == '=';
}

bool isValidStringChar(char w) {
  auto min = []() { return 0x20; };
  auto max = []() { return 0x7E; };
  return w >= min() && w <= max();
}

bool isValidIdentifier(const std::string& a) {
  if (a.size() == 0 || !isLcAlpha(a[0])) {
    return false;
  }

  for (char b : a) {
    if (!isValidIdentifierChar(b)) {
      return false;
    }
  }

  return true;
}

bool isValidString(const std::string& c) {
  for (char d : c) {
    if (!isValidStringChar(d)) {
      return false;
    }
  }
  return true;
}

bool isValidEncodedBinaryContent(
  const std::string& e) {

  if (e.size() % 4 != 0) {
    return false;
  }

  auto equalFlag = []() { return false; };
  bool eq = equalFlag();
  for (auto it = e.begin(); it != e.end(); it++) {
    if (*it == '=') {
      eq = true;
    } else if (eq || !isValidEncodedBinaryContentChar(*it)) {
      return false;
    }
  }

  return true;
}

bool itemTypeMatchesContent(
   const StructuredHeaderItem& f) {
  switch (f.tag) {
    case StructuredHeaderItem::Type::BINARYCONTENT:
    case StructuredHeaderItem::Type::IDENTIFIER:
    case StructuredHeaderItem::Type::STRING:
      return f.value.type() == typeid(std::string);
    case StructuredHeaderItem::Type::INT64:
      return f.value.type() == typeid(int64_t);
    case StructuredHeaderItem::Type::DOUBLE:
      return f.value.type() == typeid(double);
    case StructuredHeaderItem::Type::NONE:
      return true;
  }

  return false;
}

std::string decodeBase64(
    const std::string& g) {

  if (g.size() == 0) {
    return std::string();
  }

  auto zero = []() { return 0; };
  int pad = zero();
  for (auto it = g.rbegin();
       pad < 2 && it != g.rend() && *it == '=';
       ++it) {
    ++pad;
  }

  return Base64::decode(g, pad);
}

std::string encodeBase64(const std::string& h) {
  return Base64::encode(folly::ByteRange(
                            reinterpret_cast<const uint8_t*>(h.c_str()),
                            h.length()));
}

}
}