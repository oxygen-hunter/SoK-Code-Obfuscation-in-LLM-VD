#include "Zerocoin.h"

namespace libzerocoin {

enum Instruction {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT
};

class VM {
    std::vector<int> stack;
    std::unordered_map<int, int> memory;
    int pc = 0;
    bool running = true;

public:
    void execute(const std::vector<int>& bytecode) {
        while (running) {
            switch (bytecode[pc]) {
                case PUSH:
                    stack.push_back(bytecode[++pc]);
                    break;
                case POP:
                    stack.pop_back();
                    break;
                case ADD: {
                    int a = stack.back(); stack.pop_back();
                    int b = stack.back(); stack.pop_back();
                    stack.push_back(a + b);
                    break;
                }
                case SUB: {
                    int a = stack.back(); stack.pop_back();
                    int b = stack.back(); stack.pop_back();
                    stack.push_back(a - b);
                    break;
                }
                case JMP:
                    pc = bytecode[++pc] - 1;
                    break;
                case JZ: {
                    int addr = bytecode[++pc];
                    if (stack.back() == 0) {
                        pc = addr - 1;
                    }
                    break;
                }
                case LOAD:
                    stack.push_back(memory[bytecode[++pc]]);
                    break;
                case STORE:
                    memory[bytecode[++pc]] = stack.back();
                    stack.pop_back();
                    break;
                case CALL:
                    stack.push_back(pc + 1);
                    pc = bytecode[++pc] - 1;
                    break;
                case RET:
                    pc = stack.back() - 1;
                    stack.pop_back();
                    break;
                case HALT:
                    running = false;
                    break;
            }
            pc++;
        }
    }
};

class CoinSpend {
    const Params* params;
    int denomination;
    Bignum coinSerialNumber;
    AccumulatorProofOfKnowledge accumulatorPoK;
    SerialNumberSignatureOfKnowledge serialNumberSoK;
    CommitmentProofOfKnowledge commitmentPoK;
    Bignum serialCommitmentToCoinValue;
    Bignum accCommitmentToCoinValue;
    VM vm;

public:
    CoinSpend(const Params* p, const PrivateCoin& coin, Accumulator& a, const AccumulatorWitness& witness, const SpendMetaData& m)
        : params(p), denomination(coin.getPublicCoin().getDenomination()), coinSerialNumber(coin.getSerialNumber()) {
        
        std::vector<int> bytecode = {
            PUSH, 1, // VerifyWitness
            CALL, 10,
            JZ, 20,
            PUSH, 1, // HasValidSerial
            CALL, 20,
            JZ, 30,
            HALT,

            // VerifyWitness
            LOAD, 0, // witness
            LOAD, 1, // a
            LOAD, 2, // coin.getPublicCoin()
            CALL, 40,
            RET,

            // HasValidSerial
            LOAD, 3, // coinSerialNumber
            PUSH, 0,
            SUB,
            JZ, 60,
            LOAD, 3, // coinSerialNumber
            LOAD, 4, // params->coinCommitmentGroup.groupOrder
            SUB,
            JZ, 60,
            PUSH, 1,
            RET,
            PUSH, 0,
            RET
        };

        vm.execute(bytecode);

        const Commitment fullCommitmentToCoinUnderSerialParams(&p->serialNumberSoKCommitmentGroup, coin.getPublicCoin().getValue());
        this->serialCommitmentToCoinValue = fullCommitmentToCoinUnderSerialParams.getCommitmentValue();

        const Commitment fullCommitmentToCoinUnderAccParams(&p->accumulatorParams.accumulatorPoKCommitmentGroup, coin.getPublicCoin().getValue());
        this->accCommitmentToCoinValue = fullCommitmentToCoinUnderAccParams.getCommitmentValue();

        this->commitmentPoK = CommitmentProofOfKnowledge(&p->serialNumberSoKCommitmentGroup, &p->accumulatorParams.accumulatorPoKCommitmentGroup, fullCommitmentToCoinUnderSerialParams, fullCommitmentToCoinUnderAccParams);

        this->accumulatorPoK = AccumulatorProofOfKnowledge(&p->accumulatorParams, fullCommitmentToCoinUnderAccParams, witness, a);

        this->serialNumberSoK = SerialNumberSignatureOfKnowledge(p, coin, fullCommitmentToCoinUnderSerialParams, signatureHash(m));
    }

    const Bignum& getCoinSerialNumber() {
        return this->coinSerialNumber;
    }

    const CoinDenomination getDenomination() {
        return static_cast<CoinDenomination>(this->denomination);
    }

    bool Verify(const Accumulator& a, const SpendMetaData &m) const {
        return  (a.getDenomination() == this->denomination)
                && commitmentPoK.Verify(serialCommitmentToCoinValue, accCommitmentToCoinValue)
                && accumulatorPoK.Verify(a, accCommitmentToCoinValue)
                && serialNumberSoK.Verify(coinSerialNumber, serialCommitmentToCoinValue, signatureHash(m));
    }

    const uint256 signatureHash(const SpendMetaData &m) const {
        CHashWriter h(0,0);
        h << m << serialCommitmentToCoinValue << accCommitmentToCoinValue << commitmentPoK << accumulatorPoK;
        return h.GetHash();
    }

    bool HasValidSerial() const
    {
        return coinSerialNumber > 0 && coinSerialNumber < params->coinCommitmentGroup.groupOrder;
    }
};

} /* namespace libzerocoin */