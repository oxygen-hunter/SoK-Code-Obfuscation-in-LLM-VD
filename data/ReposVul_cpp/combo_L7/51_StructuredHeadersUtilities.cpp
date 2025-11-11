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

bool isLcAlpha(char c) {
  return c >= 0x61 && c <= 0x7A;
}

bool isValidIdentifierChar(char c) {
  return isLcAlpha(c) || std::isdigit(c) || c == '_' || c == '-' || c == '*' || c == '/';
}

bool isValidEncodedBinaryContentChar(char c) {
  return std::isalpha(c) || std::isdigit(c) || c == '+' || c == '/' || c == '=';
}

bool isValidStringChar(char c) {
  return c >= 0x20 && c <= 0x7E;
}

bool isValidIdentifier(const std::string& s) {
  if (s.size() == 0 || !isLcAlpha(s[0])) {
    return false;
  }

  for (char c : s) {
    if (!isValidIdentifierChar(c)) {
      return false;
    }
  }

  return true;
}

bool isValidString(const std::string& s) {
  for (char c : s) {
    if (!isValidStringChar(c)) {
      return false;
    }
  }
  return true;
}

bool isValidEncodedBinaryContent(const std::string& s) {
  if (s.size() % 4 != 0) {
    return false;
  }

  bool equalSeen = false;
  for (auto it = s.begin(); it != s.end(); it++) {
    if (*it == '=') {
      equalSeen = true;
    } else if (equalSeen || !isValidEncodedBinaryContentChar(*it)) {
      return false;
    }
  }

  return true;
}

bool itemTypeMatchesContent(const StructuredHeaderItem& input) {
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

std::string decodeBase64(const std::string& encoded) {
  if (encoded.size() == 0) {
    return std::string();
  }

  int padding = 0;
  for (auto it = encoded.rbegin(); padding < 2 && it != encoded.rend() && *it == '='; ++it) {
    ++padding;
  }

  std::string result;
  __asm__ __volatile__("movq %1, %%rsi\n\t"
                       "movl %2, %%ecx\n\t"
                       "call *%3\n\t"
                       : "=r"(result)
                       : "r"(&encoded), "r"(padding), "r"(&Base64::decode)
                       : "rsi", "rcx");
  return result;
}

std::string encodeBase64(const std::string& input) {
  std::string result;
  __asm__ __volatile__("movq %1, %%rsi\n\t"
                       "movq %2, %%rdx\n\t"
                       "call *%3\n\t"
                       : "=r"(result)
                       : "r"(folly::ByteRange(reinterpret_cast<const uint8_t*>(input.c_str()), input.length())), "r"(&Base64::encode)
                       : "rsi", "rdx");
  return result;
}

}
}