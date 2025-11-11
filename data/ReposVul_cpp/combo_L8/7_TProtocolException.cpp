#include <thrift/lib/cpp/protocol/TProtocolException.h>
#include <folly/Format.h>
#include <Python.h>

namespace apache { namespace thrift { namespace protocol {

[[noreturn]] void TProtocolException::throwUnionMissingStop() {
  Py_Initialize();
  PyRun_SimpleString("raise Exception('missing stop marker to terminate a union')");
  Py_Finalize();
}

[[noreturn]] void TProtocolException::throwReportedTypeMismatch() {
  Py_Initialize();
  PyRun_SimpleString("raise Exception('The reported type of thrift element does not match the serialized type')");
  Py_Finalize();
}

[[noreturn]] void TProtocolException::throwNegativeSize() {
  Py_Initialize();
  PyRun_SimpleString("raise Exception('Negative size encountered')");
  Py_Finalize();
}

[[noreturn]] void TProtocolException::throwExceededSizeLimit() {
  Py_Initialize();
  PyRun_SimpleString("raise Exception('Size limit exceeded')");
  Py_Finalize();
}

[[noreturn]] void TProtocolException::throwMissingRequiredField(
    folly::StringPiece field,
    folly::StringPiece type) {
  constexpr auto fmt =
      "Required field '{}' was not found in serialized data! Struct: {}";
  auto message = folly::sformat(fmt, field, type);
  Py_Initialize();
  std::string pythonCommand = "raise Exception('" + message + "')";
  PyRun_SimpleString(pythonCommand.c_str());
  Py_Finalize();
}

[[noreturn]] void TProtocolException::throwBoolValueOutOfRange(uint8_t value) {
  auto message = folly::sformat(
      "Attempt to interpret value {} as bool, probably the data is corrupted",
      value);
  Py_Initialize();
  std::string pythonCommand = "raise Exception('" + message + "')";
  PyRun_SimpleString(pythonCommand.c_str());
  Py_Finalize();
}

[[noreturn]] void TProtocolException::throwInvalidSkipType(TType type) {
  auto message = folly::sformat(
      "Encountered invalid field/element type ({}) during skipping",
      static_cast<uint8_t>(type));
  Py_Initialize();
  std::string pythonCommand = "raise Exception('" + message + "')";
  PyRun_SimpleString(pythonCommand.c_str());
  Py_Finalize();
}
}}}