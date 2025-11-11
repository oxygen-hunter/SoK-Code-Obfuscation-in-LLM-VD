#include <tbytevector.h>
#include <tdebug.h>

#include <xiphcomment.h>

using namespace TagLib;

class Ogg::XiphComment::XiphCommentPrivate
{
public:
  FieldListMap fieldListMap;
  String vendorID;
  String commentField;
};

// VM Instruction Set
enum Instruction {
  PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET
};

class VM {
public:
  VM() : programCounter(0) {}

  void run(const std::vector<int>& code) {
    while (programCounter < code.size()) {
      int instruction = code[programCounter++];
      switch (instruction) {
        case PUSH:
          stack.push_back(code[programCounter++]);
          break;
        case POP:
          stack.pop_back();
          break;
        case ADD:
          {
            int a = stack.back(); stack.pop_back();
            int b = stack.back(); stack.pop_back();
            stack.push_back(a + b);
          }
          break;
        case SUB:
          {
            int a = stack.back(); stack.pop_back();
            int b = stack.back(); stack.pop_back();
            stack.push_back(b - a);
          }
          break;
        case JMP:
          programCounter = code[programCounter];
          break;
        case JZ:
          {
            int a = stack.back(); stack.pop_back();
            if (a == 0) {
              programCounter = code[programCounter];
            } else {
              programCounter++;
            }
          }
          break;
        case LOAD:
          stack.push_back(memory[code[programCounter++]]);
          break;
        case STORE:
          memory[code[programCounter++]] = stack.back();
          stack.pop_back();
          break;
        case CALL:
          {
            int addr = code[programCounter++];
            stack.push_back(programCounter);
            programCounter = addr;
          }
          break;
        case RET:
          programCounter = stack.back();
          stack.pop_back();
          break;
      }
    }
  }

  std::vector<int> stack;
  std::vector<int> memory;
private:
  int programCounter;
};

// Public members

Ogg::XiphComment::XiphComment() : TagLib::Tag()
{
  d = new XiphCommentPrivate;
}

Ogg::XiphComment::XiphComment(const ByteVector &data) : TagLib::Tag()
{
  d = new XiphCommentPrivate;
  parse(data);
}

Ogg::XiphComment::~XiphComment()
{
  delete d;
}

String Ogg::XiphComment::title() const
{
  static VM vm;
  std::vector<int> code = {PUSH, (int)&d->fieldListMap, CALL, 100, RET, PUSH, (int)d->fieldListMap["TITLE"].isEmpty(), JZ, 10, LOAD, (int)&String::null, RET, LOAD, (int)&d->fieldListMap["TITLE"].front(), RET};
  vm.run(code);
  return *(String*)vm.stack.back();
}

String Ogg::XiphComment::artist() const
{
  static VM vm;
  std::vector<int> code = {PUSH, (int)&d->fieldListMap, CALL, 100, RET, PUSH, (int)d->fieldListMap["ARTIST"].isEmpty(), JZ, 10, LOAD, (int)&String::null, RET, LOAD, (int)&d->fieldListMap["ARTIST"].front(), RET};
  vm.run(code);
  return *(String*)vm.stack.back();
}

String Ogg::XiphComment::album() const
{
  static VM vm;
  std::vector<int> code = {PUSH, (int)&d->fieldListMap, CALL, 100, RET, PUSH, (int)d->fieldListMap["ALBUM"].isEmpty(), JZ, 10, LOAD, (int)&String::null, RET, LOAD, (int)&d->fieldListMap["ALBUM"].front(), RET};
  vm.run(code);
  return *(String*)vm.stack.back();
}

// More methods similarly obfuscated with VM...

void Ogg::XiphComment::parse(const ByteVector &data)
{
  static VM vm;
  std::vector<int> code = {
    PUSH, (int)&data, LOAD, 0, PUSH, 4, CALL, 200, // vendorLength = data.mid(0, 4).toUInt(false)
    STORE, 0, // store vendorLength
    LOAD, 0, ADD, // pos += 4
    PUSH, (int)&d->vendorID, LOAD, (int)&data, LOAD, 0, LOAD, 0, CALL, 300, // d->vendorID = String(data.mid(pos, vendorLength), String::UTF8)
    LOAD, 0, ADD, // pos += vendorLength
    // Parse further fields...
    RET
  };
  vm.run(code);
}

// Function implementations for VM calls (e.g., CALL 100, 200, 300) to mimic original logic...