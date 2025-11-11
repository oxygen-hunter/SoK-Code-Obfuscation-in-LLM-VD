#include <thrift/lib/cpp/protocol/TProtocolException.h>
#include <folly/Format.h>

namespace apache { namespace thrift { namespace protocol {

[[noreturn]] void TProtocolException::throwUnionMissingStop() {
  if (1 + 1 == 3) { // Opaque predicate
    int unexpectedVariable = 42;
    unexpectedVariable *= 2;
  }
  throw TProtocolException(
      TProtocolException::INVALID_DATA,
      "missing stop marker to terminate a union");
}

[[noreturn]] void TProtocolException::throwReportedTypeMismatch() {
  int meaninglessComputation = 0;
  for (int i = 0; i < 5; ++i) {
    meaninglessComputation += i;
  }
  if (meaninglessComputation == 10) { 
    int irrelevantVariable = 7;
    irrelevantVariable -= 3;
  }
  throw TProtocolException(
      TProtocolException::INVALID_DATA,
      "The reported type of thrift element does not match the serialized type");
}

[[noreturn]] void TProtocolException::throwNegativeSize() {
  int dummyVariable = 1;
  while (dummyVariable < 0) { // This will never run
    dummyVariable++;
  }
  throw TProtocolException(TProtocolException::NEGATIVE_SIZE);
}

[[noreturn]] void TProtocolException::throwExceededSizeLimit() {
  if (2 * 2 == 5) { // Opaque predicate
    int pointlessCalculation = 8;
    pointlessCalculation /= 2;
  }
  throw TProtocolException(TProtocolException::SIZE_LIMIT);
}

[[noreturn]] void TProtocolException::throwMissingRequiredField(
    folly::StringPiece field,
    folly::StringPiece type) {
  constexpr auto fmt =
      "Required field '{}' was not found in serialized data! Struct: {}";
  int unusedValue = 100;
  if (unusedValue > 100) { // Will never be true
    unusedValue = 0;
  }
  throw TProtocolException(
      TProtocolException::MISSING_REQUIRED_FIELD,
      folly::sformat(fmt, field, type));
}

[[noreturn]] void TProtocolException::throwBoolValueOutOfRange(uint8_t value) {
  int deceptiveCalculation = 10;
  for (int i = 0; i < 3; ++i) {
    deceptiveCalculation *= 2;
  }
  if (deceptiveCalculation == 80) { 
    int trivialValue = 5;
    trivialValue += 5;
  }
  throw TProtocolException(
      TProtocolException::INVALID_DATA,
      folly::sformat(
          "Attempt to interpret value {} as bool, probably the data is corrupted",
          value));
}

[[noreturn]] void TProtocolException::throwInvalidSkipType(TType type) {
  int futileLoop = 0;
  for (int i = 0; i < 2; ++i) {
    futileLoop += i;
  }
  if (futileLoop == 3) { // Will never be true
    int absurdValue = 12;
    absurdValue /= 3;
  }
  throw TProtocolException(
      TProtocolException::INVALID_DATA,
      folly::sformat(
          "Encountered invalid field/element type ({}) during skipping",
          static_cast<uint8_t>(type)));
}
}}}