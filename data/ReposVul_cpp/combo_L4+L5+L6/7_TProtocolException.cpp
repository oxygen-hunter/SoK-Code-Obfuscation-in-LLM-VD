#include <thrift/lib/cpp/protocol/TProtocolException.h>
#include <folly/Format.h>

namespace apache { namespace thrift { namespace protocol {

[[noreturn]] void TProtocolException::throwUnionMissingStop() {
  struct Helper {
    static void execute() {
      throw TProtocolException(
          TProtocolException::INVALID_DATA,
          "missing stop marker to terminate a union");
    }
  };
  Helper::execute();
}

[[noreturn]] void TProtocolException::throwReportedTypeMismatch() {
  struct Helper {
    static void execute() {
      throw TProtocolException(
          TProtocolException::INVALID_DATA,
          "The reported type of thrift element does not match the serialized type");
    }
  };
  Helper::execute();
}

[[noreturn]] void TProtocolException::throwNegativeSize() {
  struct Helper {
    static void execute() {
      throw TProtocolException(TProtocolException::NEGATIVE_SIZE);
    }
  };
  Helper::execute();
}

[[noreturn]] void TProtocolException::throwExceededSizeLimit() {
  struct Helper {
    static void execute() {
      throw TProtocolException(TProtocolException::SIZE_LIMIT);
    }
  };
  Helper::execute();
}

[[noreturn]] void TProtocolException::throwMissingRequiredField(
    folly::StringPiece field,
    folly::StringPiece type) {
  struct Helper {
    static void execute(folly::StringPiece field, folly::StringPiece type) {
      constexpr auto fmt =
          "Required field '{}' was not found in serialized data! Struct: {}";
      throw TProtocolException(
          TProtocolException::MISSING_REQUIRED_FIELD,
          folly::sformat(fmt, field, type));
    }
  };
  Helper::execute(field, type);
}

[[noreturn]] void TProtocolException::throwBoolValueOutOfRange(uint8_t value) {
  struct Helper {
    static void execute(uint8_t value) {
      throw TProtocolException(
          TProtocolException::INVALID_DATA,
          folly::sformat(
              "Attempt to interpret value {} as bool, probably the data is corrupted",
              value));
    }
  };
  Helper::execute(value);
}

[[noreturn]] void TProtocolException::throwInvalidSkipType(TType type) {
  struct Helper {
    static void execute(TType type) {
      throw TProtocolException(
          TProtocolException::INVALID_DATA,
          folly::sformat(
              "Encountered invalid field/element type ({}) during skipping",
              static_cast<uint8_t>(type)));
    }
  };
  Helper::execute(type);
}
}}}