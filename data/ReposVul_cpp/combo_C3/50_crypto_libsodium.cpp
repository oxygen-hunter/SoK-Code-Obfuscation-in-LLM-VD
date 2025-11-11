#include "crypto.h"

#include <tier0/vprof.h>
#include <tier0/dbg.h>

#include "tier0/memdbgoff.h"
#include <sodium.h>
#include "tier0/memdbgon.h"

#ifdef STEAMNETWORKINGSOCKETS_CRYPTO_LIBSODIUM

enum Instruction {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT
};

class VM {
public:
    VM() : pc(0) {}

    void execute(const std::vector<int>& program) {
        while (true) {
            int instr = program[pc++];
            switch (instr) {
                case PUSH: stack.push_back(program[pc++]); break;
                case POP: stack.pop_back(); break;
                case ADD: { int b = stack.back(); stack.pop_back(); int a = stack.back(); stack.pop_back(); stack.push_back(a + b); } break;
                case SUB: { int b = stack.back(); stack.pop_back(); int a = stack.back(); stack.pop_back(); stack.push_back(a - b); } break;
                case JMP: pc = program[pc]; break;
                case JZ: { int addr = program[pc++]; if (stack.back() == 0) pc = addr; } break;
                case LOAD: stack.push_back(memory[program[pc++]]); break;
                case STORE: memory[program[pc++]] = stack.back(); stack.pop_back(); break;
                case CALL: { int addr = program[pc++]; stack.push_back(pc); pc = addr; } break;
                case RET: pc = stack.back(); stack.pop_back(); break;
                case HALT: return;
            }
        }
    }
    
    void reset() { pc = 0; stack.clear(); memory.clear(); }

private:
    int pc;
    std::vector<int> stack;
    std::vector<int> memory;
};

class SymmetricCryptContextBase {
public:
    SymmetricCryptContextBase() : m_ctx(nullptr), m_cbIV(0), m_cbTag(0) {}

    void Wipe() {
        sodium_free(m_ctx);
        m_ctx = nullptr;
        m_cbIV = 0;
        m_cbTag = 0;
    }

protected:
    void* m_ctx;
    size_t m_cbIV;
    size_t m_cbTag;
};

class AES_GCM_CipherContext : public SymmetricCryptContextBase {
public:
    bool InitCipher(const void* pKey, size_t cbKey, size_t cbIV, size_t cbTag, bool bEncrypt) {
        VM vm;
        std::vector<int> program = {
            PUSH, 1, CALL, 10, HALT,
            PUSH, (int)crypto_aead_aes256gcm_is_available(), PUSH, 0, SUB, JZ, 8,
            PUSH, (int)cbKey, PUSH, (int)crypto_aead_aes256gcm_KEYBYTES, SUB, JZ, 20,
            PUSH, (int)cbIV, PUSH, (int)crypto_aead_aes256gcm_NPUBBYTES, SUB, JZ, 30,
            RET,
            RET
        };
        vm.execute(program);
        if (!m_ctx) m_ctx = sodium_malloc(sizeof(crypto_aead_aes256gcm_state));
        crypto_aead_aes256gcm_beforenm(static_cast<crypto_aead_aes256gcm_state*>(m_ctx), static_cast<const unsigned char*>(pKey));
        return true;
    }
};

class AES_GCM_EncryptContext : public AES_GCM_CipherContext {
public:
    bool Encrypt(const void* pPlaintextData, size_t cbPlaintextData, const void* pIV,
                 void* pEncryptedDataAndTag, uint32* pcbEncryptedDataAndTag,
                 const void* pAdditionalAuthenticationData, size_t cbAuthenticationData) {
        VM vm;
        std::vector<int> program = {
            PUSH, (int)(cbPlaintextData + crypto_aead_aes256gcm_ABYTES), PUSH, (int)(*pcbEncryptedDataAndTag), SUB, JZ, 20,
            PUSH, 0, STORE, 0, HALT,
            RET
        };
        vm.execute(program);
        unsigned long long cbEncryptedDataAndTag_longlong;
        crypto_aead_aes256gcm_encrypt_afternm(
            static_cast<unsigned char*>(pEncryptedDataAndTag), &cbEncryptedDataAndTag_longlong,
            static_cast<const unsigned char*>(pPlaintextData), cbPlaintextData,
            static_cast<const unsigned char*>(pAdditionalAuthenticationData), cbAuthenticationData,
            nullptr, static_cast<const unsigned char*>(pIV),
            static_cast<const crypto_aead_aes256gcm_state*>(m_ctx));
        *pcbEncryptedDataAndTag = cbEncryptedDataAndTag_longlong;
        return true;
    }
};

class AES_GCM_DecryptContext : public AES_GCM_CipherContext {
public:
    bool Decrypt(const void* pEncryptedDataAndTag, size_t cbEncryptedDataAndTag, const void* pIV,
                 void* pPlaintextData, uint32* pcbPlaintextData,
                 const void* pAdditionalAuthenticationData, size_t cbAuthenticationData) {
        VM vm;
        std::vector<int> program = {
            PUSH, (int)cbEncryptedDataAndTag, PUSH, (int)(*pcbPlaintextData + crypto_aead_aes256gcm_ABYTES), SUB, JZ, 20,
            PUSH, 0, STORE, 0, HALT,
            RET
        };
        vm.execute(program);
        unsigned long long cbPlaintextData_longlong;
        const int nDecryptResult = crypto_aead_aes256gcm_decrypt_afternm(
            static_cast<unsigned char*>(pPlaintextData), &cbPlaintextData_longlong,
            nullptr, static_cast<const unsigned char*>(pEncryptedDataAndTag), cbEncryptedDataAndTag,
            static_cast<const unsigned char*>(pAdditionalAuthenticationData), cbAuthenticationData,
            static_cast<const unsigned char*>(pIV), static_cast<const crypto_aead_aes256gcm_state*>(m_ctx));
        *pcbPlaintextData = cbPlaintextData_longlong;
        return nDecryptResult == 0;
    }
};

class CCrypto {
public:
    void Init() {
        VM vm;
        std::vector<int> program = { CALL, 10, HALT, PUSH, sodium_init(), PUSH, 0, SUB, JZ, 10, RET };
        vm.execute(program);
        AssertMsg(false, "libsodium didn't init");
    }

    void GenerateRandomBlock(void* pubDest, int cubDest) {
        VM vm;
        std::vector<int> program = { PUSH, cubDest, PUSH, 0, SUB, JZ, 10, RET };
        vm.execute(program);
        randombytes_buf(pubDest, cubDest);
    }

    void GenerateSHA256Digest(const void* pData, size_t cbData, SHA256Digest_t* pOutputDigest) {
        VM vm;
        std::vector<int> program = { CALL, 10, HALT, PUSH, (int)pData, PUSH, 0, SUB, JZ, 10, RET };
        vm.execute(program);
        crypto_hash_sha256(*pOutputDigest, static_cast<const unsigned char*>(pData), cbData);
    }

    void GenerateHMAC256(const uint8* pubData, uint32 cubData, const uint8* pubKey, uint32 cubKey, SHA256Digest_t* pOutputDigest) {
        VM vm;
        std::vector<int> program = {
            PUSH, (int)pubData, PUSH, 0, SUB, JZ, 10,
            PUSH, (int)cubData, PUSH, 0, SUB, JZ, 20,
            PUSH, (int)pubKey, PUSH, 0, SUB, JZ, 30,
            PUSH, (int)cubKey, PUSH, 0, SUB, JZ, 40,
            PUSH, (int)pOutputDigest, PUSH, 0, SUB, JZ, 50,
            RET
        };
        vm.execute(program);
        crypto_auth_hmacsha256(*pOutputDigest, pubData, cubData, pubKey);
    }
};

#endif