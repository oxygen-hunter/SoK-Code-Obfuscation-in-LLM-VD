#include "hphp/runtime/ext/wddx/ext_wddx.h"
#include <string>
#include <vector>

namespace HPHP {

enum class OpCode { PUSH, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT };

struct Instruction {
  OpCode op;
  int operand;
};

class VM {
public:
  VM() : pc(0), running(true) {}

  void run(const std::vector<Instruction>& code, WddxPacket* packet) {
    instructions = code;
    while (running) {
      dispatch(packet);
    }
  }

private:
  int pc;
  bool running;
  std::vector<Instruction> instructions;
  std::vector<int> stack;
  std::unordered_map<std::string, int> registers;

  void dispatch(WddxPacket* packet) {
    Instruction instr = instructions[pc];
    switch (instr.op) {
    case OpCode::PUSH:
      stack.push_back(instr.operand);
      pc++;
      break;
    case OpCode::ADD:
      execute_add();
      break;
    case OpCode::SUB:
      execute_sub();
      break;
    case OpCode::JMP:
      pc = instr.operand;
      break;
    case OpCode::JZ:
      execute_jz();
      break;
    case OpCode::LOAD:
      execute_load();
      break;
    case OpCode::STORE:
      execute_store();
      break;
    case OpCode::CALL:
      execute_call(packet);
      break;
    case OpCode::RET:
      running = false;
      break;
    case OpCode::HALT:
      running = false;
      break;
    }
  }

  void execute_add() {
    int b = stack.back(); stack.pop_back();
    int a = stack.back(); stack.pop_back();
    stack.push_back(a + b);
    pc++;
  }

  void execute_sub() {
    int b = stack.back(); stack.pop_back();
    int a = stack.back(); stack.pop_back();
    stack.push_back(a - b);
    pc++;
  }

  void execute_jz() {
    int cond = stack.back(); stack.pop_back();
    if (cond == 0) {
      pc = instructions[pc].operand;
    } else {
      pc++;
    }
  }

  void execute_load() {
    std::string regName = std::to_string(instructions[pc].operand);
    stack.push_back(registers[regName]);
    pc++;
  }

  void execute_store() {
    int value = stack.back(); stack.pop_back();
    std::string regName = std::to_string(instructions[pc].operand);
    registers[regName] = value;
    pc++;
  }

  void execute_call(WddxPacket* packet) {
    std::string func = std::to_string(instructions[pc].operand);
    if (func == "recursiveAddVar") {
      std::string varName = std::to_string(stack.back()); stack.pop_back();
      Variant varVariant; // Assume some way to convert int to Variant for demonstration
      bool hasVarTag = stack.back(); stack.pop_back();
      packet->recursiveAddVar(varName, varVariant, hasVarTag);
    }
    pc++;
  }
};

WddxPacket::WddxPacket(const Variant& comment, bool manualPacket, bool sVar) :
                       m_packetString(""), m_packetClosed(false),
                       m_manualPacketCreation(manualPacket) {
  std::string header = "<header/>";
  if (!comment.isNull() && !sVar) {
    std::string scomment = comment.toString().data();
    header = "<header><comment>" + scomment + "</comment></header>";
  }
  m_packetString = "<wddxPacket version='1.0'>" + header + "<data>";
  if (m_manualPacketCreation) {
    m_packetString = m_packetString + "<struct>";
  }
}

bool WddxPacket::add_var(const String& varName, bool hasVarTag) {
  VarEnv* v = g_context->getVarEnv();
  if (!v) return false;
  Variant varVariant = *reinterpret_cast<Variant*>(v->lookup(varName.get()));
  return recursiveAddVar(varName, varVariant, hasVarTag);
}

std::string WddxPacket::packet_end() {
  if (!m_packetClosed) {
    if (m_manualPacketCreation) {
      m_packetString += "</struct>";
    }
    m_packetString += "</data></wddxPacket>";
  }
  m_packetClosed = true;
  return m_packetString;
}

bool WddxPacket::serialize_value(const Variant& varVariant) {
  return recursiveAddVar(empty_string_ref, varVariant, false);
}

bool WddxPacket::recursiveAddVar(const String& varName,
                                 const Variant& varVariant,
                                 bool hasVarTag) {
  VM vm;
  std::vector<Instruction> code = {
    {OpCode::CALL, std::stoi(varName.data())},
    {OpCode::RET, 0}
  };
  vm.run(code, this);
  return true;
}

std::string WddxPacket::getWddxEncoded(const std::string& varType,
                                       const std::string& varValue,
                                       const String& varName,
                                       bool hasVarTag) {
  if (varType.compare("NULL") == 0) {
    return wrapValue("<null/>", "", "", varName, hasVarTag);
  }
  if (varType.compare("boolean") == 0) {
    return wrapValue("<boolean value='", "'/>", varValue, varName, hasVarTag);
  }
  if (varType.compare("integer") == 0 || varType.compare("double") == 0) {
    return wrapValue("<number>", "</number>", varValue, varName, hasVarTag);
  }
  if (varType.compare("string") == 0) {
    return wrapValue("<string>", "</string>", varValue, varName, hasVarTag);
  }
  return "";
}

std::string WddxPacket::wrapValue(const std::string& start,
                                  const std::string& end,
                                  const std::string& varValue,
                                  const String& varName,
                                  bool hasVarTag) {
  std::string startVar = "";
  std::string endVar = "";
  if (hasVarTag) {
    startVar += "<var name='";
    startVar += varName.data();
    startVar += "'>";
    endVar = "</var>";
  }
  return startVar + start + varValue + end + endVar;
}

void find_var_recursive(const TypedValue* tv, WddxPacket* wddxPacket) {
  if (tvIsString(tv)) {
    String var_name = tvCastToString(tv);
    wddxPacket->add_var(var_name, true);
  }
  if (tv->m_type == KindOfArray) {
    for (ArrayIter iter(tv->m_data.parr); iter; ++iter) {
      find_var_recursive(iter.secondRef().asTypedValue(), wddxPacket);
    }
  }
}

static TypedValue* add_vars_helper(ActRec* ar) {
  int start_index = 1;
  Resource packet_id = getArg<KindOfResource>(ar, 0);
  auto wddxPacket = packet_id.getTyped<WddxPacket>();
  for (int i = start_index; i < ar->numArgs(); i++) {
    auto const tv = getArg(ar, i);
    find_var_recursive(tv, wddxPacket);
  }
  return arReturn(ar, true);
}

static TypedValue* serialize_vars_helper(ActRec* ar) {
  WddxPacket* wddxPacket = newres<WddxPacket>(empty_string_variant_ref,
                                              true, true);
  int start_index = 0;
  for (int i = start_index; i < ar->numArgs(); i++) {
    auto const tv = getArg(ar, i);
    find_var_recursive(tv, wddxPacket);
  }
  Variant packet = wddxPacket->packet_end();
  return arReturn(ar, std::move(packet));
}

static TypedValue* HHVM_FN(wddx_add_vars)(ActRec* ar) {
  return add_vars_helper(ar);
}

static TypedValue* HHVM_FN(wddx_serialize_vars)(ActRec* ar) {
  return serialize_vars_helper(ar);
}

static String HHVM_FUNCTION(wddx_packet_end, const Resource& packet_id) {
  auto wddxPacket = packet_id.getTyped<WddxPacket>();
  std::string packetString = wddxPacket->packet_end();
  return String(packetString);
}

static Resource HHVM_FUNCTION(wddx_packet_start, const Variant& comment) {
  auto wddxPacket = newres<WddxPacket>(comment, true, false);
  return Resource(wddxPacket);
}

static String HHVM_FUNCTION(wddx_serialize_value, const Variant& var,
                            const Variant& comment) {
  WddxPacket* wddxPacket = newres<WddxPacket>(comment, false, false);
  wddxPacket->serialize_value(var);
  const std::string packetString = wddxPacket->packet_end();
  return String(packetString);
}

class wddxExtension : public Extension {
 public:
  wddxExtension() : Extension("wddx") {}
  virtual void moduleInit() {
    HHVM_FE(wddx_add_vars);
    HHVM_FE(wddx_packet_end);
    HHVM_FE(wddx_packet_start);
    HHVM_FE(wddx_serialize_value);
    HHVM_FE(wddx_serialize_vars);
    loadSystemlib();
  }
} s_wddx_extension;

} // namespace HPHP