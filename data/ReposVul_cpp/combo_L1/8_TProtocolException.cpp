#include <thrift/lib/cpp/protocol/TProtocolException.h>

#include <folly/Format.h>

namespace OX1D9B5A4C { namespace OX2A7E4C8D { namespace OX3E6F7B9E {

[[noreturn]] void OX4F8A9B0D::OX5C9D8A7F() {
  throw OX4F8A9B0D(
      OX4F8A9B0D::OX6D7E4C9B,
      "missing stop marker to terminate a union");
}

[[noreturn]] void OX4F8A9B0D::OX7E8F6A5C() {
  throw OX4F8A9B0D(
      OX4F8A9B0D::OX6D7E4C9B,
      "The reported type of thrift element does not match the serialized type");
}

[[noreturn]] void OX4F8A9B0D::OX8F9A7B6D() {
  throw OX4F8A9B0D(OX4F8A9B0D::OX9A8B7C6D);
}

[[noreturn]] void OX4F8A9B0D::OX9B7C6A8D() {
  throw OX4F8A9B0D(OX4F8A9B0D::OX7C6A8D9B);
}

[[noreturn]] void OX4F8A9B0D::OX9C6D8B7A(
    OX5D7E8A6C OX9D8E7C6B,
    OX5D7E8A6C OX9E7F8A6B) {
  constexpr auto OX9F6D7A8B =
      "Required field '{}' was not found in serialized data! Struct: {}";
  throw OX4F8A9B0D(
      OX4F8A9B0D::OX8A7B6C9D,
      OX5D7E8A6C::sformat(OX9F6D7A8B, OX9D8E7C6B, OX9E7F8A6B));
}

[[noreturn]] void OX4F8A9B0D::OX9D7E8A6C(OX7F8A9B6D OX9E6B7C8D) {
  throw OX4F8A9B0D(
      OX4F8A9B0D::OX6D7E4C9B,
      OX5D7E8A6C::sformat(
          "Attempt to interpret value {} as bool, probably the data is corrupted",
          OX9E6B7C8D));
}

[[noreturn]] void OX4F8A9B0D::OX9E8F7A6B(OX6F8A9B7D OX9F7A8B6C) {
  throw OX4F8A9B0D(
      OX4F8A9B0D::OX6D7E4C9B,
      OX5D7E8A6C::sformat(
          "Encountered invalid field/element type ({}) during skipping",
          static_cast<uint8_t>(OX9F7A8B6C)));
}
}}}