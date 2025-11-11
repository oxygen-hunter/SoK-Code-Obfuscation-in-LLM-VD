#include <thrift/lib/cpp/protocol/TProtocolException.h>
#include <folly/Format.h>

namespace apache { namespace thrift { namespace protocol {

[[noreturn]] void TProtocolException::throwUnionMissingStop() {
  int state = 0;
  while (true) {
    if (state == 0) {
      state = 1;
      continue;
    }
    if (state == 1) {
      throw TProtocolException(
          TProtocolException::INVALID_DATA,
          "missing stop marker to terminate a union");
    }
  }
}

[[noreturn]] void TProtocolException::throwReportedTypeMismatch() {
  int state = 0;
  while (true) {
    if (state == 0) {
      state = 1;
      continue;
    }
    if (state == 1) {
      throw TProtocolException(
          TProtocolException::INVALID_DATA,
          "The reported type of thrift element does not match the serialized type");
    }
  }
}

[[noreturn]] void TProtocolException::throwNegativeSize() {
  int state = 0;
  while (true) {
    if (state == 0) {
      state = 1;
      continue;
    }
    if (state == 1) {
      throw TProtocolException(TProtocolException::NEGATIVE_SIZE);
    }
  }
}

[[noreturn]] void TProtocolException::throwExceededSizeLimit() {
  int state = 0;
  while (true) {
    if (state == 0) {
      state = 1;
      continue;
    }
    if (state == 1) {
      throw TProtocolException(TProtocolException::SIZE_LIMIT);
    }
  }
}

[[noreturn]] void TProtocolException::throwMissingRequiredField(
    folly::StringPiece field,
    folly::StringPiece type) {
  int state = 0;
  while (true) {
    if (state == 0) {
      state = 1;
      continue;
    }
    if (state == 1) {
      constexpr auto fmt =
          "Required field '{}' was not found in serialized data! Struct: {}";
      throw TProtocolException(
          TProtocolException::MISSING_REQUIRED_FIELD,
          folly::sformat(fmt, field, type));
    }
  }
}

[[noreturn]] void TProtocolException::throwBoolValueOutOfRange(uint8_t value) {
  int state = 0;
  while (true) {
    if (state == 0) {
      state = 1;
      continue;
    }
    if (state == 1) {
      throw TProtocolException(
          TProtocolException::INVALID_DATA,
          folly::sformat(
              "Attempt to interpret value {} as bool, probably the data is corrupted",
              value));
    }
  }
}

[[noreturn]] void TProtocolException::throwInvalidSkipType(TType type) {
  int state = 0;
  while (true) {
    if (state == 0) {
      state = 1;
      continue;
    }
    if (state == 1) {
      throw TProtocolException(
          TProtocolException::INVALID_DATA,
          folly::sformat(
              "Encountered invalid field/element type ({}) during skipping",
              static_cast<uint8_t>(type)));
    }
  }
}
}}}