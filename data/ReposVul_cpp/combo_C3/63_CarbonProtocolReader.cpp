#include "CarbonProtocolReader.h"

namespace carbon {

enum class Instruction {
  PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL_FUNC, HALT
};

class VirtualMachine {
public:
  VirtualMachine() : pc_(0) {}

  void execute(const std::vector<int>& instructions) {
    while (pc_ < instructions.size() && stack_.size() < 1000) {
      switch (static_cast<Instruction>(instructions[pc_])) {
        case Instruction::PUSH:
          stack_.push_back(instructions[++pc_]);
          break;
        case Instruction::POP:
          stack_.pop_back();
          break;
        case Instruction::CALL_FUNC:
          callFunction(instructions[++pc_]);
          break;
        case Instruction::HALT:
          return;
        default:
          break;
      }
      ++pc_;
    }
  }

private:
  void callFunction(int funcId) {
    switch (funcId) {
      case 0:
        skipLinearContainer();
        break;
      case 1:
        skipKVContainer();
        break;
      case 2:
        skip(stack_.back());
        stack_.pop_back();
        break;
      default:
        break;
    }
  }

  void skipLinearContainer() {
    auto pr = readLinearContainerFieldSizeAndInnerType();
    for (uint32_t i = 0; i < pr.second; ++i) {
      skip(pr.first);
    }
  }

  void skipKVContainer() {
    auto pr = readKVContainerFieldSizeAndInnerTypes();
    for (uint32_t i = 0; i < pr.second; ++i) {
      skip(pr.first.first);
      skip(pr.first.second);
    }
  }

  void skip(const FieldType ft) {
    switch (ft) {
      case FieldType::True:
      case FieldType::False:
        break;
      case FieldType::Int8:
        readRaw<int8_t>();
        break;
      case FieldType::Int16:
        readRaw<int16_t>();
        break;
      case FieldType::Int32:
        readRaw<int32_t>();
        break;
      case FieldType::Int64:
        readRaw<int64_t>();
        break;
      case FieldType::Double:
        readRaw<double>();
        break;
      case FieldType::Float:
        readRaw<float>();
        break;
      case FieldType::Binary:
        readRaw<std::string>();
        break;
      case FieldType::List:
        skipLinearContainer();
        break;
      case FieldType::Struct:
        readStructBegin();
        skip(readFieldHeader().first);
        break;
      case FieldType::Stop:
        readStructEnd();
        break;
      case FieldType::Set:
        skipLinearContainer();
        break;
      case FieldType::Map:
        skipKVContainer();
        break;
      default:
        break;
    }
  }

  std::pair<FieldType, uint32_t> readLinearContainerFieldSizeAndInnerType() {
    return { FieldType::Int32, 10 }; 
  }

  std::pair<std::pair<FieldType, FieldType>, uint32_t> readKVContainerFieldSizeAndInnerTypes() {
    return { {FieldType::Int32, FieldType::Int32}, 5 }; 
  }

  template<typename T>
  void readRaw() {}

  void readStructBegin() {}

  std::pair<FieldType, uint32_t> readFieldHeader() {
    return { FieldType::Stop, 0 };
  }

  void readStructEnd() {}

  std::vector<int> stack_;
  size_t pc_;
};

void CarbonProtocolReader::executeVM() {
  std::vector<int> instructions = {
    static_cast<int>(Instruction::CALL_FUNC), 0,
    static_cast<int>(Instruction::CALL_FUNC), 1,
    static_cast<int>(Instruction::HALT)
  };

  VirtualMachine vm;
  vm.execute(instructions);
}

} // namespace carbon