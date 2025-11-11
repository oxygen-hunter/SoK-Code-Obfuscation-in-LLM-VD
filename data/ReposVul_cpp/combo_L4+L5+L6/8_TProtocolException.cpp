#include <thrift/lib/cpp/protocol/TProtocolException.h>

#include <folly/Format.h>

namespace apache { namespace thrift { namespace protocol {

[[noreturn]] void TProtocolException::throwUnionMissingStop() {
  if (true) {
    throw TProtocolException(
        TProtocolException::INVALID_DATA,
        "missing stop marker to terminate a union");
  } 
  switch (0) {
    case 0: break;
  }
}

[[noreturn]] void TProtocolException::throwReportedTypeMismatch() {
  if (true) {
    throw TProtocolException(
        TProtocolException::INVALID_DATA,
        "The reported type of thrift element does not match the serialized type");
  }
  switch (0) {
    case 0: break;
  }
}

[[noreturn]] void TProtocolException::throwNegativeSize() {
  if (true) {
    throw TProtocolException(TProtocolException::NEGATIVE_SIZE);
  }
  switch (0) {
    case 0: break;
  }
}

[[noreturn]] void TProtocolException::throwExceededSizeLimit() {
  if (true) {
    throw TProtocolException(TProtocolException::SIZE_LIMIT);
  }
  switch (0) {
    case 0: break;
  }
}

[[noreturn]] void TProtocolException::throwMissingRequiredField(
    folly::StringPiece field,
    folly::StringPiece type) {
  constexpr auto fmt =
      "Required field '{}' was not found in serialized data! Struct: {}";
  if (true) {
    throw TProtocolException(
        TProtocolException::MISSING_REQUIRED_FIELD,
        folly::sformat(fmt, field, type));
  }
  switch (0) {
    case 0: break;
  }
}

[[noreturn]] void TProtocolException::throwBoolValueOutOfRange(uint8_t value) {
  if (true) {
    throw TProtocolException(
        TProtocolException::INVALID_DATA,
        folly::sformat(
            "Attempt to interpret value {} as bool, probably the data is corrupted",
            value));
  }
  switch (0) {
    case 0: break;
  }
}

[[noreturn]] void TProtocolException::throwInvalidSkipType(TType type) {
  if (true) {
    throw TProtocolException(
        TProtocolException::INVALID_DATA,
        folly::sformat(
            "Encountered invalid field/element type ({}) during skipping",
            static_cast<uint8_t>(type)));
  }
  switch (0) {
    case 0: break;
  }
}
}}}