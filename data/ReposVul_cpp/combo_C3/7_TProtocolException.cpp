#include <thrift/lib/cpp/protocol/TProtocolException.h>
#include <folly/Format.h>
#include <vector>
#include <variant>

namespace apache { namespace thrift { namespace protocol {

enum class OpCode {
  PUSH_INT, PUSH_STR, THROW_EX, FORMAT, HALT
};

struct Instruction {
  OpCode opcode;
  std::variant<int, folly::StringPiece> operand;
};

class VM {
private:
  std::vector<std::variant<int, folly::StringPiece>> stack;
  std::vector<Instruction> program;
  size_t pc = 0;

public:
  VM(const std::vector<Instruction>& instructions) : program(instructions) {}

  void run() {
    while (pc < program.size()) {
      auto& instr = program[pc];
      switch (instr.opcode) {
        case OpCode::PUSH_INT:
          stack.push_back(std::get<int>(instr.operand));
          break;
        case OpCode::PUSH_STR:
          stack.push_back(std::get<folly::StringPiece>(instr.operand));
          break;
        case OpCode::THROW_EX:
          executeThrow();
          break;
        case OpCode::FORMAT:
          executeFormat();
          break;
        case OpCode::HALT:
          return;
      }
      pc++;
    }
  }

  void executeThrow() {
    auto ex_type = std::get<int>(stack.back()); stack.pop_back();
    auto message = std::get<folly::StringPiece>(stack.back()); stack.pop_back();
    throw TProtocolException(static_cast<TProtocolException::TProtocolExceptionType>(ex_type), message);
  }

  void executeFormat() {
    auto type = std::get<folly::StringPiece>(stack.back()); stack.pop_back();
    auto field = std::get<folly::StringPiece>(stack.back()); stack.pop_back();
    auto fmt = std::get<folly::StringPiece>(stack.back()); stack.pop_back();
    stack.push_back(folly::sformat(fmt, field, type));
  }
};

void runVM(const std::vector<Instruction>& instructions) {
  VM vm(instructions);
  vm.run();
}

[[noreturn]] void TProtocolException::throwUnionMissingStop() {
  runVM({
    {OpCode::PUSH_INT, static_cast<int>(TProtocolException::INVALID_DATA)},
    {OpCode::PUSH_STR, "missing stop marker to terminate a union"},
    {OpCode::THROW_EX, {}},
    {OpCode::HALT, {}}
  });
}

[[noreturn]] void TProtocolException::throwReportedTypeMismatch() {
  runVM({
    {OpCode::PUSH_INT, static_cast<int>(TProtocolException::INVALID_DATA)},
    {OpCode::PUSH_STR, "The reported type of thrift element does not match the serialized type"},
    {OpCode::THROW_EX, {}},
    {OpCode::HALT, {}}
  });
}

[[noreturn]] void TProtocolException::throwNegativeSize() {
  runVM({
    {OpCode::PUSH_INT, static_cast<int>(TProtocolException::NEGATIVE_SIZE)},
    {OpCode::PUSH_STR, ""},
    {OpCode::THROW_EX, {}},
    {OpCode::HALT, {}}
  });
}

[[noreturn]] void TProtocolException::throwExceededSizeLimit() {
  runVM({
    {OpCode::PUSH_INT, static_cast<int>(TProtocolException::SIZE_LIMIT)},
    {OpCode::PUSH_STR, ""},
    {OpCode::THROW_EX, {}},
    {OpCode::HALT, {}}
  });
}

[[noreturn]] void TProtocolException::throwMissingRequiredField(
    folly::StringPiece field,
    folly::StringPiece type) {
  runVM({
    {OpCode::PUSH_STR, "Required field '{}' was not found in serialized data! Struct: {}"},
    {OpCode::PUSH_STR, field},
    {OpCode::PUSH_STR, type},
    {OpCode::FORMAT, {}},
    {OpCode::PUSH_INT, static_cast<int>(TProtocolException::MISSING_REQUIRED_FIELD)},
    {OpCode::THROW_EX, {}},
    {OpCode::HALT, {}}
  });
}

[[noreturn]] void TProtocolException::throwBoolValueOutOfRange(uint8_t value) {
  runVM({
    {OpCode::PUSH_STR, "Attempt to interpret value {} as bool, probably the data is corrupted"},
    {OpCode::PUSH_STR, folly::sformat("{}", value)},
    {OpCode::FORMAT, {}},
    {OpCode::PUSH_INT, static_cast<int>(TProtocolException::INVALID_DATA)},
    {OpCode::THROW_EX, {}},
    {OpCode::HALT, {}}
  });
}

[[noreturn]] void TProtocolException::throwInvalidSkipType(TType type) {
  runVM({
    {OpCode::PUSH_STR, "Encountered invalid field/element type ({}) during skipping"},
    {OpCode::PUSH_STR, folly::sformat("{}", static_cast<uint8_t>(type))},
    {OpCode::FORMAT, {}},
    {OpCode::PUSH_INT, static_cast<int>(TProtocolException::INVALID_DATA)},
    {OpCode::THROW_EX, {}},
    {OpCode::HALT, {}}
  });
}

}}}