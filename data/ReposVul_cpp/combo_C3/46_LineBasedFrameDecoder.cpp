#include <wangle/codec/LineBasedFrameDecoder.h>
#include <vector>
#include <stdexcept>
#include <string>

namespace wangle {

using folly::io::Cursor;
using folly::IOBuf;
using folly::IOBufQueue;

class VirtualMachine {
public:
    enum Instructions { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT };

    VirtualMachine(const std::vector<int>& program)
        : program_(program), pc_(0), sp_(-1), halted_(false) {
        stack_.resize(100);
    }

    void run() {
        while (!halted_) {
            dispatch();
        }
    }

private:
    std::vector<int> program_;
    std::vector<int> stack_;
    int pc_;
    int sp_;
    bool halted_;

    void dispatch() {
        switch (program_[pc_]) {
            case PUSH:
                stack_[++sp_] = program_[++pc_];
                break;
            case POP:
                sp_--;
                break;
            case ADD:
                stack_[sp_ - 1] += stack_[sp_];
                sp_--;
                break;
            case SUB:
                stack_[sp_ - 1] -= stack_[sp_];
                sp_--;
                break;
            case JMP:
                pc_ = program_[++pc_] - 1;
                break;
            case JZ:
                if (stack_[sp_] == 0) {
                    pc_ = program_[++pc_] - 1;
                } else {
                    pc_++;
                }
                sp_--;
                break;
            case LOAD:
                stack_[++sp_] = program_[++pc_];
                break;
            case STORE:
                program_[program_[++pc_]] = stack_[sp_--];
                break;
            case CALL:
                stack_[++sp_] = pc_ + 2;
                pc_ = program_[++pc_] - 1;
                break;
            case RET:
                pc_ = stack_[sp_--] - 1;
                break;
            case HALT:
                halted_ = true;
                break;
            default:
                throw std::runtime_error("Unknown instruction");
        }
        pc_++;
    }
};

class LineBasedFrameDecoder {
public:
    LineBasedFrameDecoder(uint32_t maxLength, bool stripDelimiter, TerminatorType terminatorType)
        : maxLength_(maxLength), stripDelimiter_(stripDelimiter), terminatorType_(terminatorType), discarding_(false), discardedBytes_(0) {}

    bool decode(Context* ctx, IOBufQueue& buf, std::unique_ptr<IOBuf>& result, size_t&) {
        std::vector<int> program = { VirtualMachine::PUSH, static_cast<int>(findEndOfLine(buf)),
                                     VirtualMachine::CALL, 10, VirtualMachine::HALT, 10, 
                                     VirtualMachine::LOAD, 1, VirtualMachine::JMP, 15,
                                     VirtualMachine::LOAD, 0, VirtualMachine::JMP, 20, 
                                     VirtualMachine::RET, VirtualMachine::RET };

        VirtualMachine vm(program);
        vm.run();

        int64_t eol = program[1];

        if (!discarding_) {
            if (eol >= 0) {
                Cursor c(buf.front());
                c += eol;
                auto delimLength = c.read<char>() == '\r' ? 2 : 1;
                if (eol > maxLength_) {
                    buf.split(eol + delimLength);
                    fail(ctx, folly::to<std::string>(eol));
                    return false;
                }

                std::unique_ptr<folly::IOBuf> frame;

                if (stripDelimiter_) {
                    frame = buf.split(eol);
                    buf.trimStart(delimLength);
                } else {
                    frame = buf.split(eol + delimLength);
                }

                result = std::move(frame);
                return true;
            } else {
                auto len = buf.chainLength();
                if (len > maxLength_) {
                    discardedBytes_ = len;
                    buf.trimStart(len);
                    discarding_ = true;
                    fail(ctx, "over " + folly::to<std::string>(len));
                }
                return false;
            }
        } else {
            if (eol >= 0) {
                Cursor c(buf.front());
                c += eol;
                auto delimLength = c.read<char>() == '\r' ? 2 : 1;
                buf.trimStart(eol + delimLength);
                discardedBytes_ = 0;
                discarding_ = false;
            } else {
                discardedBytes_ = buf.chainLength();
                buf.move();
            }

            return false;
        }
    }

private:
    uint32_t maxLength_;
    bool stripDelimiter_;
    TerminatorType terminatorType_;
    bool discarding_;
    size_t discardedBytes_;

    void fail(Context* ctx, std::string len) {
        ctx->fireReadException(
            folly::make_exception_wrapper<std::runtime_error>(
                "frame length" + len +
                " exceeds max " + folly::to<std::string>(maxLength_)));
    }

    int64_t findEndOfLine(IOBufQueue& buf) {
        Cursor c(buf.front());
        for (uint32_t i = 0; i < maxLength_ && i < buf.chainLength(); i++) {
            auto b = c.read<char>();
            if (b == '\n' && terminatorType_ != TerminatorType::CARRIAGENEWLINE) {
                return i;
            } else if (
                terminatorType_ != TerminatorType::NEWLINE && b == '\r' &&
                !c.isAtEnd() && *c.peekBytes().data() == '\n') {
                return i;
            }
        }
        return -1;
    }
};

} // namespace wangle