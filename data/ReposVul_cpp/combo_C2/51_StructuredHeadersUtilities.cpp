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
  int state = 0;
  while (true) {
    switch (state) {
      case 0:
        return c >= 0x61 && c <= 0x7A;
    }
  }
}

bool isValidIdentifierChar(char c) {
  int state = 0;
  while (true) {
    switch (state) {
      case 0:
        if (isLcAlpha(c)) return true;
        state = 1;
        break;
      case 1:
        if (std::isdigit(c)) return true;
        state = 2;
        break;
      case 2:
        if (c == '_') return true;
        state = 3;
        break;
      case 3:
        if (c == '-') return true;
        state = 4;
        break;
      case 4:
        if (c == '*') return true;
        state = 5;
        break;
      case 5:
        if (c == '/') return true;
        state = 6;
        break;
      case 6:
        return false;
    }
  }
}

bool isValidEncodedBinaryContentChar(char c) {
  int state = 0;
  while (true) {
    switch (state) {
      case 0:
        if (std::isalpha(c)) return true;
        state = 1;
        break;
      case 1:
        if (std::isdigit(c)) return true;
        state = 2;
        break;
      case 2:
        if (c == '+') return true;
        state = 3;
        break;
      case 3:
        if (c == '/') return true;
        state = 4;
        break;
      case 4:
        if (c == '=') return true;
        state = 5;
        break;
      case 5:
        return false;
    }
  }
}

bool isValidStringChar(char c) {
  int state = 0;
  while (true) {
    switch (state) {
      case 0:
        return c >= 0x20 && c <= 0x7E;
    }
  }
}

bool isValidIdentifier(const std::string& s) {
  int state = 0;
  size_t i = 0;
  while (true) {
    switch (state) {
      case 0:
        if (s.size() == 0 || !isLcAlpha(s[0])) return false;
        state = 1;
        break;
      case 1:
        if (i < s.size()) {
          if (!isValidIdentifierChar(s[i])) return false;
          ++i;
        } else {
          return true;
        }
        break;
    }
  }
}

bool isValidString(const std::string& s) {
  int state = 0;
  size_t i = 0;
  while (true) {
    switch (state) {
      case 0:
        if (i < s.size()) {
          if (!isValidStringChar(s[i])) return false;
          ++i;
        } else {
          return true;
        }
        break;
    }
  }
}

bool isValidEncodedBinaryContent(const std::string& s) {
  int state = 0;
  bool equalSeen = false;
  auto it = s.begin();
  while (true) {
    switch (state) {
      case 0:
        if (s.size() % 4 != 0) return false;
        state = 1;
        break;
      case 1:
        if (it != s.end()) {
          if (*it == '=') {
            equalSeen = true;
          } else if (equalSeen || !isValidEncodedBinaryContentChar(*it)) {
            return false;
          }
          ++it;
        } else {
          return true;
        }
        break;
    }
  }
}

bool itemTypeMatchesContent(const StructuredHeaderItem& input) {
  int state = 0;
  while (true) {
    switch (state) {
      case 0:
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
        state = 1;
        break;
      case 1:
        return false;
    }
  }
}

std::string decodeBase64(const std::string& encoded) {
  int state = 0;
  int padding = 0;
  auto it = encoded.rbegin();
  while (true) {
    switch (state) {
      case 0:
        if (encoded.size() == 0) {
          return std::string();
        }
        state = 1;
        break;
      case 1:
        if (padding < 2 && it != encoded.rend() && *it == '=') {
          ++padding;
          ++it;
        } else {
          return Base64::decode(encoded, padding);
        }
        break;
    }
  }
}

std::string encodeBase64(const std::string& input) {
  int state = 0;
  while (true) {
    switch (state) {
      case 0:
        return Base64::encode(folly::ByteRange(
          reinterpret_cast<const uint8_t*>(input.c_str()), input.length()));
    }
  }
}

}
}