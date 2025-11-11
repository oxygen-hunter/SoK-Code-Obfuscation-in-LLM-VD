#include <iostream>
#include <vector>
#include <stdexcept>
#include <memory>
#include <type_traits>
#include <folly/Optional.h>
#include <folly/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <folly/String.h>

namespace fizz {

enum class ContentType { handshake, alert, application_data, change_cipher_spec };
enum class ProtocolVersion { tls_1_0, tls_1_2 };
enum class EncryptionLevel { Plaintext };

struct TLSMessage {
  ContentType type;
  std::unique_ptr<folly::IOBuf> fragment;
};

struct TLSContent {
  std::unique_ptr<folly::IOBuf> data;
  ContentType contentType;
  EncryptionLevel encryptionLevel;
};

class PlaintextReadRecordLayer {
public:
  folly::Optional<TLSMessage> read(folly::IOBufQueue& buf);

  EncryptionLevel getEncryptionLevel() const;

private:
  ProtocolVersion receivedRecordVersion_;
  bool skipEncryptedRecords_ = true;
};

class PlaintextWriteRecordLayer {
public:
  TLSContent write(TLSMessage&& msg) const;
  TLSContent writeInitialClientHello(std::unique_ptr<folly::IOBuf> encodedClientHello) const;

  EncryptionLevel getEncryptionLevel() const;

private:
  TLSContent write(TLSMessage msg, ProtocolVersion recordVersion) const;
};

static constexpr uint16_t kMaxPlaintextRecordSize = 0x4000;
static constexpr size_t kPlaintextHeaderSize = sizeof(ContentType) + sizeof(ProtocolVersion) + sizeof(uint16_t);

enum Instruction {
  PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, HALT, CTYPE, PVERSION, LENGTH, CHECKLEN, CLONE, TRIM, THROW, RETURN
};

class VM {
public:
  VM(std::vector<int> program) : program_(std::move(program)), pc_(0) {}

  int run() {
    while (pc_ < program_.size()) {
      switch (program_[pc_++]) {
        case PUSH: stack_.push_back(program_[pc_++]); break;
        case POP: stack_.pop_back(); break;
        case ADD: binOp(std::plus<int>()); break;
        case SUB: binOp(std::minus<int>()); break;
        case JMP: pc_ = program_[pc_]; break;
        case JZ: if (stack_.back() == 0) pc_ = program_[pc_]; else pc_++; break;
        case LOAD: stack_.push_back(stack_[program_[pc_++]]); break;
        case STORE: stack_[program_[pc_++]] = stack_.back(); break;
        case HALT: return stack_.back();
        case CTYPE: stack_.push_back(static_cast<int>(ContentType::handshake)); break;
        case PVERSION: stack_.push_back(static_cast<int>(ProtocolVersion::tls_1_2)); break;
        case LENGTH: stack_.push_back(kMaxPlaintextRecordSize); break;
        case CHECKLEN: if (stack_.back() > kMaxPlaintextRecordSize) throw std::runtime_error("too long"); break;
        case CLONE: stack_.push_back(stack_.back()); break;
        case TRIM: stack_.pop_back(); break;
        case THROW: throw std::runtime_error("error"); break;
        case RETURN: return stack_.back();
        default: throw std::logic_error("Unknown instruction");
      }
    }
    throw std::logic_error("Program finished without HALT");
  }

private:
  template<typename Op>
  void binOp(Op op) {
    int b = stack_.back(); stack_.pop_back();
    int a = stack_.back(); stack_.pop_back();
    stack_.push_back(op(a, b));
  }

  std::vector<int> program_;
  std::vector<int> stack_;
  size_t pc_;
};

folly::Optional<TLSMessage> PlaintextReadRecordLayer::read(folly::IOBufQueue& buf) {
  static std::vector<int> program = {
    PUSH, 1,    // Begin loop
    LOAD, 0, JMP, 19,
    CTYPE, CHECKLEN, THROW,
    LENGTH, CHECKLEN, THROW,
    CLONE, LENGTH, ADD,
    PUSH, 0, JZ, 1,
    RETURN
  };

  try {
    VM vm(program);
    vm.run();
    TLSMessage msg;
    msg.type = ContentType::handshake;
    msg.fragment = folly::IOBuf::create(0);
    return msg;
  } catch (...) {
    return folly::none;
  }
}

EncryptionLevel PlaintextReadRecordLayer::getEncryptionLevel() const {
  return EncryptionLevel::Plaintext;
}

TLSContent PlaintextWriteRecordLayer::write(TLSMessage&& msg) const {
  return write(std::move(msg), ProtocolVersion::tls_1_2);
}

TLSContent PlaintextWriteRecordLayer::writeInitialClientHello(std::unique_ptr<folly::IOBuf> encodedClientHello) const {
  TLSMessage msg{ContentType::handshake, std::move(encodedClientHello)};
  return write(std::move(msg), ProtocolVersion::tls_1_0);
}

TLSContent PlaintextWriteRecordLayer::write(TLSMessage msg, ProtocolVersion recordVersion) const {
  static std::vector<int> program = {
    PUSH, 1,    // Begin loop
    LOAD, 0, JMP, 19,
    CTYPE, CHECKLEN, THROW,
    LENGTH, CHECKLEN, THROW,
    CLONE, LENGTH, ADD,
    PUSH, 0, JZ, 1,
    RETURN
  };

  TLSContent content;
  try {
    VM vm(program);
    vm.run();
    content.data = folly::IOBuf::create(0);
    content.contentType = ContentType::handshake;
    content.encryptionLevel = EncryptionLevel::Plaintext;
  } catch (...) {
    throw std::runtime_error("error in VM");
  }
  return content;
}

EncryptionLevel PlaintextWriteRecordLayer::getEncryptionLevel() const {
  return EncryptionLevel::Plaintext;
}

} // namespace fizz