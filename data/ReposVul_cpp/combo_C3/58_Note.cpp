#include "Note.hpp"
#include "prf.h"
#include "crypto/sha256.h"
#include "random.h"
#include "version.h"
#include "streams.h"
#include "zcash/util.h"
#include "librustzcash.h"

using namespace libzcash;

enum OpCode {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET
};

class VM {
    std::vector<uint256> stack;
    std::vector<uint256> memory;
    std::vector<uint256> registers;
    size_t pc;
    bool running;

    void run(const std::vector<uint256>& bytecode) {
        running = true;
        pc = 0;
        while (running && pc < bytecode.size()) {
            uint256 instruction = bytecode[pc++];
            execute(instruction);
        }
    }

    void execute(uint256 instruction) {
        switch (static_cast<OpCode>(instruction.GetLow64())) {
            case PUSH: {
                uint256 value = memory[pc++];
                stack.push_back(value);
                break;
            }
            case POP: {
                if (!stack.empty()) stack.pop_back();
                break;
            }
            case ADD: {
                if (stack.size() >= 2) {
                    uint256 b = stack.back(); stack.pop_back();
                    uint256 a = stack.back(); stack.pop_back();
                    stack.push_back(a + b);
                }
                break;
            }
            case SUB: {
                if (stack.size() >= 2) {
                    uint256 b = stack.back(); stack.pop_back();
                    uint256 a = stack.back(); stack.pop_back();
                    stack.push_back(a - b);
                }
                break;
            }
            case JMP: {
                pc = memory[pc].GetLow64();
                break;
            }
            case JZ: {
                if (!stack.empty() && stack.back().IsNull()) {
                    pc = memory[pc].GetLow64();
                } else {
                    pc++;
                }
                break;
            }
            case LOAD: {
                uint256 address = memory[pc++];
                stack.push_back(memory[address.GetLow64()]);
                break;
            }
            case STORE: {
                uint256 address = stack.back(); stack.pop_back();
                uint256 value = stack.back(); stack.pop_back();
                memory[address.GetLow64()] = value;
                break;
            }
            case CALL: {
                uint256 address = memory[pc++];
                call(address.GetLow64());
                break;
            }
            case RET: {
                running = false;
                break;
            }
            default:
                break;
        }
    }

    void call(size_t address) {
        size_t returnAddress = pc;
        run({LOAD, address, CALL, address, PUSH, returnAddress, RET});
    }

public:
    VM() : pc(0), running(false) {
        memory.resize(1024);
        registers.resize(16);
    }

    uint256 executeFunction(size_t address, const std::vector<uint256>& args) {
        for (size_t i = 0; i < args.size(); ++i) {
            memory[i] = args[i];
        }
        run({CALL, address});
        return stack.empty() ? uint256() : stack.back();
    }
};

SproutNote::SproutNote() {
    a_pk = random_uint256();
    rho = random_uint256();
    r = random_uint256();
}

uint256 SproutNote::cm() const {
    VM vm;
    return vm.executeFunction(0, {a_pk, rho, r, uint256(0xb0)});
}

uint256 SproutNote::nullifier(const SproutSpendingKey& a_sk) const {
    VM vm;
    return vm.executeFunction(1, {a_sk, rho});
}

SaplingNote::SaplingNote(const SaplingPaymentAddress& address, const uint64_t value) : BaseNote(value) {
    d = address.d;
    pk_d = address.pk_d;
    librustzcash_sapling_generate_r(r.begin());
}

boost::optional<uint256> SaplingNote::cm() const {
    VM vm;
    return vm.executeFunction(2, {d, pk_d, uint256(value()), r});
}

boost::optional<uint256> SaplingNote::nullifier(const SaplingFullViewingKey& vk, const uint64_t position) const {
    VM vm;
    return vm.executeFunction(3, {d, pk_d, uint256(value()), r, vk.ak, vk.nk, uint256(position)});
}

SproutNotePlaintext::SproutNotePlaintext(
    const SproutNote& note,
    std::array<unsigned char, ZC_MEMO_SIZE> memo) : BaseNotePlaintext(note, memo)
{
    rho = note.rho;
    r = note.r;
}

SproutNote SproutNotePlaintext::note(const SproutPaymentAddress& addr) const {
    VM vm;
    return vm.executeFunction(4, {addr.a_pk, uint256(value_), rho, r});
}

SproutNotePlaintext SproutNotePlaintext::decrypt(const ZCNoteDecryption& decryptor,
                                     const ZCNoteDecryption::Ciphertext& ciphertext,
                                     const uint256& ephemeralKey,
                                     const uint256& h_sig,
                                     unsigned char nonce
                                    )
{
    VM vm;
    return vm.executeFunction(5, {decryptor, ciphertext, ephemeralKey, h_sig, uint256(nonce)});
}

ZCNoteEncryption::Ciphertext SproutNotePlaintext::encrypt(ZCNoteEncryption& encryptor,
                                                    const uint256& pk_enc
                                                   ) const
{
    VM vm;
    return vm.executeFunction(6, {encryptor, pk_enc});
}

SaplingNotePlaintext::SaplingNotePlaintext(
    const SaplingNote& note,
    std::array<unsigned char, ZC_MEMO_SIZE> memo) : BaseNotePlaintext(note, memo)
{
    d = note.d;
    rcm = note.r;
}

boost::optional<SaplingNote> SaplingNotePlaintext::note(const SaplingIncomingViewingKey& ivk) const {
    VM vm;
    return vm.executeFunction(7, {ivk, d, uint256(value_), rcm});
}

boost::optional<SaplingOutgoingPlaintext> SaplingOutgoingPlaintext::decrypt(
    const SaplingOutCiphertext &ciphertext,
    const uint256& ovk,
    const uint256& cv,
    const uint256& cm,
    const uint256& epk
)
{
    VM vm;
    return vm.executeFunction(8, {ciphertext, ovk, cv, cm, epk});
}

boost::optional<SaplingNotePlaintext> SaplingNotePlaintext::decrypt(
    const SaplingEncCiphertext &ciphertext,
    const uint256 &ivk,
    const uint256 &epk,
    const uint256 &cmu
)
{
    VM vm;
    return vm.executeFunction(9, {ciphertext, ivk, epk, cmu});
}

boost::optional<SaplingNotePlaintext> SaplingNotePlaintext::decrypt(
    const SaplingEncCiphertext &ciphertext,
    const uint256 &epk,
    const uint256 &esk,
    const uint256 &pk_d,
    const uint256 &cmu
)
{
    VM vm;
    return vm.executeFunction(10, {ciphertext, epk, esk, pk_d, cmu});
}

boost::optional<SaplingNotePlaintextEncryptionResult> SaplingNotePlaintext::encrypt(const uint256& pk_d) const
{
    VM vm;
    return vm.executeFunction(11, {pk_d});
}

SaplingOutCiphertext SaplingOutgoingPlaintext::encrypt(
        const uint256& ovk,
        const uint256& cv,
        const uint256& cm,
        SaplingNoteEncryption& enc
    ) const
{
    VM vm;
    return vm.executeFunction(12, {ovk, cv, cm, enc});
}