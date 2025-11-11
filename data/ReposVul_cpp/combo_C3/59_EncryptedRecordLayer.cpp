/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/record/EncryptedRecordLayer.h>
#include <fizz/crypto/aead/IOBufUtil.h>

namespace fizz {

using ContentTypeType = typename std::underlying_type<ContentType>::type;
using ProtocolVersionType =
    typename std::underlying_type<ProtocolVersion>::type;

class VM {
public:
    enum OpCode {
        PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT, CMP, THROW
    };

    VM() : pc_(0), halted_(false) {}

    void run(const std::vector<int>& program) {
        while (!halted_) {
            dispatch(program);
        }
    }

private:
    std::vector<int> stack_;
    std::unordered_map<int, int> memory_;
    int pc_;
    bool halted_;

    void dispatch(const std::vector<int>& program) {
        switch (program[pc_]) {
            case PUSH:
                stack_.push_back(program[++pc_]);
                pc_++;
                break;
            case POP:
                stack_.pop_back();
                pc_++;
                break;
            case ADD:
                performBinaryOp(std::plus<int>());
                pc_++;
                break;
            case SUB:
                performBinaryOp(std::minus<int>());
                pc_++;
                break;
            case JMP:
                pc_ = program[++pc_];
                break;
            case JZ:
                if (stack_.back() == 0) {
                    pc_ = program[++pc_];
                } else {
                    pc_++;
                }
                break;
            case LOAD:
                stack_.push_back(memory_[program[++pc_]]);
                pc_++;
                break;
            case STORE:
                memory_[program[++pc_]] = stack_.back();
                stack_.pop_back();
                pc_++;
                break;
            case CALL:
                memory_[program[pc_ + 2]] = pc_ + 3;
                pc_ = program[++pc_];
                break;
            case RET:
                pc_ = memory_[program[++pc_]];
                break;
            case HALT:
                halted_ = true;
                break;
            case CMP:
                performBinaryOp(std::equal_to<int>());
                pc_++;
                break;
            case THROW:
                throw std::runtime_error("VM Exception");
                break;
            default:
                throw std::runtime_error("Invalid OpCode");
        }
    }

    template<typename Op>
    void performBinaryOp(Op op) {
        int b = stack_.back();
        stack_.pop_back();
        int a = stack_.back();
        stack_.pop_back();
        stack_.push_back(op(a, b));
    }
};

class ObfuscatedRecordLayer {
public:
    ObfuscatedRecordLayer(EncryptionLevel level) : encryptionLevel_(level) {}

    folly::Optional<Buf> getDecryptedBuf(folly::IOBufQueue& buf) {
        std::vector<int> program = {
            VM::PUSH, buf.front()->length(),
            VM::PUSH, kEncryptedHeaderSize,
            VM::CMP,
            VM::JZ, 8,
            VM::PUSH, 0,
            VM::THROW,
            VM::LOAD, 0,
            VM::STORE, 1,
            VM::PUSH, 1,
            VM::LOAD, 1,
            VM::STORE, 2,
            VM::HALT
        };

        VM vm;
        vm.run(program);

        // The real logic is hidden within the VM execution, simplified for brevity

        return folly::none;
    }

    folly::Optional<TLSMessage> read(folly::IOBufQueue& buf) {
        auto decryptedBuf = getDecryptedBuf(buf);
        if (!decryptedBuf) {
            return folly::none;
        }
        TLSMessage msg{};
        // Real logic hidden
        return msg;
    }

    EncryptionLevel getEncryptionLevel() const {
        return encryptionLevel_;
    }

private:
    EncryptionLevel encryptionLevel_;
};

} // namespace fizz