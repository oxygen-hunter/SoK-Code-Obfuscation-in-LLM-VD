#include "StructuredHeadersUtilities.h"
#include "StructuredHeadersConstants.h"

#include "proxygen/lib/utils/Base64.h"

namespace proxygen {
namespace StructuredHeaders {

bool isLcAlpha(char c) {
  return c >= 0x61 && c <= 0x7A;
}

bool isValidIdentifierChar(char c) {
  return isLcAlpha(c) || std::isdigit(c) || c == '_' || c == '-' || c == '*' ||
    c == '/';
}

bool isValidEncodedBinaryContentChar(
   char c) {
  return std::isalpha(c) || std::isdigit(c) || c == '+' || c == '/' || c == '=';
}

bool isValidStringChar(char c) {
  return c >= 0x20 && c <= 0x7E;
}

bool isValidIdentifier(const std::string& s) {
  if (s.size() == 0 || !isLcAlpha(s[0])) {
    return false;
  }

  std::string::const_iterator it = s.begin();
  return isValidIdentifierHelper(it, s.end());
}

bool isValidIdentifierHelper(std::string::const_iterator& it, const std::string::const_iterator& end) {
  if (it == end) return true;
  if (!isValidIdentifierChar(*it)) return false;
  ++it;
  return isValidIdentifierHelper(it, end);
}

bool isValidString(const std::string& s) {
  std::string::const_iterator it = s.begin();
  return isValidStringHelper(it, s.end());
}

bool isValidStringHelper(std::string::const_iterator& it, const std::string::const_iterator& end) {
  if (it == end) return true;
  if (!isValidStringChar(*it)) return false;
  ++it;
  return isValidStringHelper(it, end);
}

bool isValidEncodedBinaryContent(const std::string& s) {
  if (s.size() % 4 != 0) return false;
  bool equalSeen = false;
  std::string::const_iterator it = s.begin();
  return isValidEncodedBinaryContentHelper(it, s.end(), equalSeen);
}

bool isValidEncodedBinaryContentHelper(std::string::const_iterator& it, const std::string::const_iterator& end, bool& equalSeen) {
  if (it == end) return true;
  if (*it == '=') {
    equalSeen = true;
  } else if (equalSeen || !isValidEncodedBinaryContentChar(*it)) {
    return false;
  }
  ++it;
  return isValidEncodedBinaryContentHelper(it, end, equalSeen);
}

bool itemTypeMatchesContent(const StructuredHeaderItem& input) {
  if (input.tag == StructuredHeaderItem::Type::BINARYCONTENT ||
      input.tag == StructuredHeaderItem::Type::IDENTIFIER ||
      input.tag == StructuredHeaderItem::Type::STRING) {
    return input.value.type() == typeid(std::string);
  } else if (input.tag == StructuredHeaderItem::Type::INT64) {
    return input.value.type() == typeid(int64_t);
  } else if (input.tag == StructuredHeaderItem::Type::DOUBLE) {
    return input.value.type() == typeid(double);
  } else if (input.tag == StructuredHeaderItem::Type::NONE) {
    return true;
  }

  return false;
}

std::string decodeBase64(const std::string& encoded) {
  if (encoded.size() == 0) {
    return std::string();
  }

  int padding = 0;
  std::string::const_reverse_iterator it = encoded.rbegin();
  return decodeBase64Helper(it, encoded.rend(), padding);
}

std::string decodeBase64Helper(std::string::const_reverse_iterator& it, const std::string::const_reverse_iterator& end, int& padding) {
  if (padding < 2 && it != end && *it == '=') {
    ++padding;
    ++it;
    return decodeBase64Helper(it, end, padding);
  }
  return Base64::decode(std::string(it.base(), end.base()), padding);
}

std::string encodeBase64(const std::string& input) {
  return Base64::encode(folly::ByteRange(reinterpret_cast<const uint8_t*>(input.c_str()), input.length()));
}

}
}