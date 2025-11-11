#include <thrift/lib/cpp/protocol/TProtocolException.h>
#include <folly/Format.h>

namespace apache { namespace thrift { namespace protocol {

[[noreturn]] void OX7B4DF339::OXA7E4F4B3() {
  throw OX7B4DF339(
      OX7B4DF339::OX3C42D2F1,
      "missing stop marker to terminate a union");
}

[[noreturn]] void OX7B4DF339::OX9F1D2B67() {
  throw OX7B4DF339(
      OX7B4DF339::OX3C42D2F1,
      "The reported type of thrift element does not match the serialized type");
}

[[noreturn]] void OX7B4DF339::OXA2DF8A76() {
  throw OX7B4DF339(OX7B4DF339::OX6F7A89B8);
}

[[noreturn]] void OX7B4DF339::OX8C3E2A45() {
  throw OX7B4DF339(OX7B4DF339::OXE4C0A7B1);
}

[[noreturn]] void OX7B4DF339::OXD5A4C3E9(
    folly::StringPiece OX4B2D9C73,
    folly::StringPiece OX5A1E3F64) {
  constexpr auto OX2F3A9B87 =
      "Required field '{}' was not found in serialized data! Struct: {}";
  throw OX7B4DF339(
      OX7B4DF339::OX1B2D5F89,
      folly::sformat(OX2F3A9B87, OX4B2D9C73, OX5A1E3F64));
}

[[noreturn]] void OX7B4DF339::OX9E2C4B67(uint8_t OX3F1B8A2D) {
  throw OX7B4DF339(
      OX7B4DF339::OX3C42D2F1,
      folly::sformat(
          "Attempt to interpret value {} as bool, probably the data is corrupted",
          OX3F1B8A2D));
}

[[noreturn]] void OX7B4DF339::OX4C2A9D7E(TType OX6B1E7A4D) {
  throw OX7B4DF339(
      OX7B4DF339::OX3C42D2F1,
      folly::sformat(
          "Encountered invalid field/element type ({}) during skipping",
          static_cast<uint8_t>(OX6B1E7A4D)));
}
}}}