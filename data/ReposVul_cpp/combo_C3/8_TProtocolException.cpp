#include <thrift/lib/cpp/protocol/TProtocolException.h>
#include <folly/Format.h>

namespace apache { namespace thrift { namespace protocol {

enum Opcode {
  PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, THROW, HALT
};

class VM {
public:
  VM() : pc(0) {}

  void execute(const std::vector<int>& program) {
    while (pc < program.size()) {
      switch (program[pc]) {
        case PUSH: stack.push_back(program[++pc]); break;
        case POP: stack.pop_back(); break;
        case ADD: binaryOp(std::plus<int>()); break;
        case SUB: binaryOp(std::minus<int>()); break;
        case JMP: pc = program[++pc] - 1; break;
        case JZ: { int target = program[++pc]; if (stack.back() == 0) pc = target - 1; } break;
        case LOAD: { int idx = program[++pc]; stack.push_back(memory[idx]); } break;
        case STORE: { int idx = program[++pc]; memory[idx] = stack.back(); stack.pop_back(); } break;
        case THROW: throwException(program[++pc]); break;
        case HALT: return;
        default: break;
      }
      ++pc;
    }
  }

private:
  std::vector<int> stack;
  std::vector<int> memory = std::vector<int>(10, 0);
  size_t pc;

  template <typename Op>
  void binaryOp(Op op) {
    int b = stack.back(); stack.pop_back();
    int a = stack.back(); stack.pop_back();
    stack.push_back(op(a, b));
  }

  void throwException(int exceptionType) {
    switch (exceptionType) {
      case 0: throw TProtocolException(TProtocolException::INVALID_DATA, "missing stop marker to terminate a union");
      case 1: throw TProtocolException(TProtocolException::INVALID_DATA, "The reported type of thrift element does not match the serialized type");
      case 2: throw TProtocolException(TProtocolException::NEGATIVE_SIZE);
      case 3: throw TProtocolException(TProtocolException::SIZE_LIMIT);
      case 4: {
        std::string field = "field";
        std::string type = "type";
        constexpr auto fmt = "Required field '{}' was not found in serialized data! Struct: {}";
        throw TProtocolException(TProtocolException::MISSING_REQUIRED_FIELD, folly::sformat(fmt, field, type));
      }
      case 5: {
        uint8_t value = 255;
        throw TProtocolException(TProtocolException::INVALID_DATA, folly::sformat("Attempt to interpret value {} as bool, probably the data is corrupted", value));
      }
      case 6: {
        TType type = TType::T_STOP;
        throw TProtocolException(TProtocolException::INVALID_DATA, folly::sformat("Encountered invalid field/element type ({}) during skipping", static_cast<uint8_t>(type)));
      }
    }
  }
};

void executeVM() {
  std::vector<int> program = {
    PUSH, 0, THROW, 
    PUSH, 1, THROW, 
    PUSH, 2, THROW, 
    PUSH, 3, THROW, 
    PUSH, 4, THROW, 
    PUSH, 5, THROW, 
    PUSH, 6, THROW, 
    HALT
  };

  VM vm;
  vm.execute(program);
}

}}}