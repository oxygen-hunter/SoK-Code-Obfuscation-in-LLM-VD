#include "cipher.h"
#include "logger.h"
#include <vector>
#include <map>
#include <functional>

enum Instruction {
    PUSH,
    POP,
    ADD,
    SUB,
    JMP,
    JZ,
    LOAD,
    STORE,
    CALL,
    RET,
    NOP
};

class VM {
public:
    VM() : pc(0) {}

    void execute(const std::vector<int>& program) {
        while (pc < program.size()) {
            int instr = program[pc++];
            switch (instr) {
                case PUSH: {
                    int value = program[pc++];
                    stack.push_back(value);
                    break;
                }
                case POP: {
                    stack.pop_back();
                    break;
                }
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
                case JMP: {
                    int target = program[pc++];
                    pc = target;
                    break;
                }
                case JZ: {
                    int target = program[pc++];
                    if (stack.back() == 0) {
                        pc = target;
                    }
                    stack.pop_back();
                    break;
                }
                case LOAD: {
                    int address = program[pc++];
                    stack.push_back(memory[address]);
                    break;
                }
                case STORE: {
                    int address = program[pc++];
                    memory[address] = stack.back();
                    stack.pop_back();
                    break;
                }
                case CALL: {
                    int target = program[pc++];
                    callStack.push_back(pc);
                    pc = target;
                    break;
                }
                case RET: {
                    pc = callStack.back();
                    callStack.pop_back();
                    break;
                }
                case NOP: {
                    break;
                }
            }
        }
    }

private:
    std::vector<int> stack;
    std::vector<int> memory;
    std::vector<int> callStack;
    int pc;
};

Cipher::Cipher() {
    runVM([&] {
        m_primeNum = QCA::BigInteger("12745216229761186769575009943944198619149164746831579719941140425076456621824834322853258804883232842877311723249782818608677050956745409379781245497526069657222703636504651898833151008222772087491045206203033063108075098874712912417029101508315117935752962862335062591404043092163187352352197487303798807791605274487594646923");
        setType("blowfish");
    });
}

Cipher::Cipher(QByteArray key, QString cipherType) {
    runVM([&] {
        m_primeNum = QCA::BigInteger("12745216229761186769575009943944198619149164746831579719941140425076456621824834322853258804883232842877311723249782818608677050956745409379781245497526069657222703636504651898833151008222772087491045206203033063108075098874712912417029101508315117935752962862335062591404043092163187352352197487303798807791605274487594646923");
        setKey(key);
        setType(cipherType);
    });
}

Cipher::~Cipher() {}

bool Cipher::setKey(QByteArray key) {
    return runVM([&] {
        if (key.isEmpty()) {
            m_key.clear();
            return false;
        }
        if (key.mid(0, 4).toLower() == "ecb:") {
            m_cbc = false;
            m_key = key.mid(4);
        } else if (key.mid(0, 4).toLower() == "cbc:") {
            m_cbc = true;
            m_key = key.mid(4);
        } else {
            m_cbc = false;
            m_key = key;
        }
        return true;
    });
}

bool Cipher::setType(const QString &type) {
    return runVM([&] {
        m_type = type;
        return true;
    });
}

QByteArray Cipher::decrypt(QByteArray cipherText) {
    return runVM([&] {
        QByteArray pfx = "";
        bool error = false;
        if (cipherText.mid(0, 5) == "+OK *") {
            if (m_cbc) {
                cipherText = cipherText.mid(5);
            } else {
                cipherText = cipherText.mid(5);
                pfx = "ERROR_NONECB: ";
                error = true;
            }
        } else if (cipherText.mid(0, 4) == "+OK " || cipherText.mid(0, 5) == "mcps ") {
            if (m_cbc) {
                cipherText = (cipherText.mid(0, 4) == "+OK ") ? cipherText.mid(4) : cipherText.mid(5);
                pfx = "ERROR_NONCBC: ";
                error = true;
            } else {
                cipherText = (cipherText.mid(0, 4) == "+OK ") ? cipherText.mid(4) : cipherText.mid(5);
            }
        } else {
            return cipherText;
        }
        QByteArray temp;
        if ((m_cbc && !error) || (!m_cbc && error)) {
            temp = blowfishCBC(cipherText, false);
            if (temp == cipherText) {
                return cipherText + ' ' + '\n';
            } else {
                cipherText = temp;
            }
        } else {
            temp = blowfishECB(cipherText, false);
            if (temp == cipherText) {
                return cipherText + ' ' + '\n';
            } else {
                cipherText = temp;
            }
        }
        if (cipherText.at(0) == 1) {
            pfx = "\x0";
        }
        cipherText = pfx + cipherText + ' ' + '\n';
        return cipherText;
    });
}

QByteArray Cipher::initKeyExchange() {
    return runVM([&] {
        QCA::Initializer init;
        m_tempKey = QCA::KeyGenerator().createDH(QCA::DLGroup(m_primeNum, QCA::BigInteger(2))).toDH();

        if (m_tempKey.isNull()) {
            return QByteArray();
        }

        QByteArray publicKey = m_tempKey.toPublicKey().toDH().y().toArray().toByteArray();

        if (publicKey.length() > 135 && publicKey.at(0) == '\0') {
            publicKey = publicKey.mid(1);
        }

        return publicKey.toBase64().append('A');
    });
}

QByteArray Cipher::parseInitKeyX(QByteArray key) {
    return runVM([&] {
        QCA::Initializer init;
        bool isCBC = false;

        if (key.endsWith(" CBC")) {
            isCBC = true;
            key.chop(4);
        }

        if (key.length() != 181) {
            return QByteArray();
        }

        QCA::SecureArray remoteKey = QByteArray::fromBase64(key.left(180));
        QCA::DLGroup group(m_primeNum, QCA::BigInteger(2));
        QCA::DHPrivateKey privateKey = QCA::KeyGenerator().createDH(group).toDH();

        if (privateKey.isNull()) {
            return QByteArray();
        }

        QByteArray publicKey = privateKey.y().toArray().toByteArray();

        if (publicKey.length() > 135 && publicKey.at(0) == '\0') {
            publicKey = publicKey.mid(1);
        }

        QCA::DHPublicKey remotePub(group, remoteKey);

        if (remotePub.isNull()) {
            return QByteArray();
        }

        QByteArray sharedKey = privateKey.deriveKey(remotePub).toByteArray();
        sharedKey = QCA::Hash("sha256").hash(sharedKey).toByteArray().toBase64();

        while (sharedKey.endsWith('=')) sharedKey.chop(1);

        if (isCBC) {
            sharedKey.prepend("cbc:");
        }

        bool success = setKey(sharedKey);

        if (!success) {
            return QByteArray();
        }

        return publicKey.toBase64().append('A');
    });
}

bool Cipher::parseFinishKeyX(QByteArray key) {
    return runVM([&] {
        QCA::Initializer init;

        if (key.length() != 181) {
            return false;
        }

        QCA::SecureArray remoteKey = QByteArray::fromBase64(key.left(180));
        QCA::DLGroup group(m_primeNum, QCA::BigInteger(2));

        QCA::DHPublicKey remotePub(group, remoteKey);

        if (remotePub.isNull()) {
            return false;
        }

        if (m_tempKey.isNull()) {
            return false;
        }

        QByteArray sharedKey = m_tempKey.deriveKey(remotePub).toByteArray();
        sharedKey = QCA::Hash("sha256").hash(sharedKey).toByteArray().toBase64();

        while (sharedKey.endsWith('=')) sharedKey.chop(1);

        bool success = setKey(sharedKey);

        return success;
    });
}

QByteArray Cipher::decryptTopic(QByteArray cipherText) {
    return runVM([&] {
        if (cipherText.mid(0, 4) == "+OK ") {
            cipherText = cipherText.mid(4);
        } else if (cipherText.left(5) == "«m«") {
            cipherText = cipherText.mid(5, cipherText.length() - 10);
        } else {
            return cipherText;
        }

        QByteArray temp;
        if (m_cbc) {
            temp = blowfishCBC(cipherText.mid(1), false);
        } else {
            temp = blowfishECB(cipherText, false);
        }

        if (temp == cipherText) {
            return cipherText;
        } else {
            cipherText = temp;
        }

        if (cipherText.mid(0, 2) == "@@") {
            cipherText = cipherText.mid(2);
        }

        return cipherText;
    });
}

bool Cipher::encrypt(QByteArray &cipherText) {
    return runVM([&] {
        if (cipherText.left(3) == "+p ") {
            cipherText = cipherText.mid(3);
        } else {
            if (m_cbc) {
                QByteArray temp = blowfishCBC(cipherText, true);

                if (temp == cipherText) {
                    return false;
                }

                cipherText = "+OK *" + temp;
            } else {
                QByteArray temp = blowfishECB(cipherText, true);

                if (temp == cipherText) {
                    return false;
                }

                cipherText = "+OK " + temp;
            }
        }
        return true;
    });
}

QByteArray Cipher::blowfishCBC(QByteArray cipherText, bool direction) {
    return runVM([&] {
        QCA::Initializer init;
        QByteArray temp = cipherText;
        if (direction) {
            while ((temp.length() % 8) != 0) temp.append('\0');
            QCA::InitializationVector iv(8);
            temp.prepend(iv.toByteArray());
        } else {
            temp = QByteArray::fromBase64(temp);
            while ((temp.length() % 8) != 0) temp.append('\0');
        }

        QCA::Direction dir = (direction) ? QCA::Encode : QCA::Decode;
        QCA::Cipher cipher(m_type, QCA::Cipher::CBC, QCA::Cipher::NoPadding, dir, m_key, QCA::InitializationVector(QByteArray("0")));
        QByteArray temp2 = cipher.update(QCA::MemoryRegion(temp)).toByteArray();
        temp2 += cipher.final().toByteArray();

        if (!cipher.ok())
            return cipherText;

        if (direction) {
            temp2 = temp2.toBase64();
        } else {
            temp2 = temp2.remove(0, 8);
        }

        return temp2;
    });
}

QByteArray Cipher::blowfishECB(QByteArray cipherText, bool direction) {
    return runVM([&] {
        QCA::Initializer init;
        QByteArray temp = cipherText;

        if (direction) {
            while ((temp.length() % 8) != 0) temp.append('\0');
        } else {
            if ((temp.length() % 12) != 0)
                return cipherText;

            temp = b64ToByte(temp);
            while ((temp.length() % 8) != 0) temp.append('\0');
        }

        QCA::Direction dir = (direction) ? QCA::Encode : QCA::Decode;
        QCA::Cipher cipher(m_type, QCA::Cipher::ECB, QCA::Cipher::NoPadding, dir, m_key);
        QByteArray temp2 = cipher.update(QCA::MemoryRegion(temp)).toByteArray();
        temp2 += cipher.final().toByteArray();

        if (!cipher.ok())
            return cipherText;

        if (direction) {
            if ((temp2.length() % 8) != 0)
                return cipherText;

            temp2 = byteToB64(temp2);
        }

        return temp2;
    });
}

QByteArray Cipher::byteToB64(QByteArray text) {
    return runVM([&] {
        int left = 0;
        int right = 0;
        int k = -1;
        int v;
        QString base64 = "./0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        QByteArray encoded;
        while (k < (text.length() - 1)) {
            k++;
            v = text.at(k); if (v < 0) v += 256;
            left = v << 24;
            k++;
            v = text.at(k); if (v < 0) v += 256;
            left += v << 16;
            k++;
            v = text.at(k); if (v < 0) v += 256;
            left += v << 8;
            k++;
            v = text.at(k); if (v < 0) v += 256;
            left += v;

            k++;
            v = text.at(k); if (v < 0) v += 256;
            right = v << 24;
            k++;
            v = text.at(k); if (v < 0) v += 256;
            right += v << 16;
            k++;
            v = text.at(k); if (v < 0) v += 256;
            right += v << 8;
            k++;
            v = text.at(k); if (v < 0) v += 256;
            right += v;

            for (int i = 0; i < 6; i++) {
                encoded.append(base64.at(right & 0x3F).toAscii());
                right = right >> 6;
            }

            for (int i = 0; i < 6; i++) {
                encoded.append(base64.at(left & 0x3F).toAscii());
                left = left >> 6;
            }
        }
        return encoded;
    });
}

QByteArray Cipher::b64ToByte(QByteArray text) {
    return runVM([&] {
        QString base64 = "./0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
        QByteArray decoded;
        int k = -1;
        while (k < (text.length() - 1)) {
            int right = 0;
            int left = 0;
            int v = 0;
            int w = 0;
            int z = 0;

            for (int i = 0; i < 6; i++) {
                k++;
                v = base64.indexOf(text.at(k));
                right |= v << (i * 6);
            }

            for (int i = 0; i < 6; i++) {
                k++;
                v = base64.indexOf(text.at(k));
                left |= v << (i * 6);
            }

            for (int i = 0; i < 4; i++) {
                w = ((left & (0xFF << ((3 - i) * 8))));
                z = w >> ((3 - i) * 8);
                if (z < 0) { z = z + 256; }
                decoded.append(z);
            }

            for (int i = 0; i < 4; i++) {
                w = ((right & (0xFF << ((3 - i) * 8))));
                z = w >> ((3 - i) * 8);
                if (z < 0) { z = z + 256; }
                decoded.append(z);
            }
        }
        return decoded;
    });
}

bool Cipher::neededFeaturesAvailable() {
    return runVM([&] {
        QCA::Initializer init;

        if (QCA::isSupported("blowfish-ecb") && QCA::isSupported("blowfish-cbc") && QCA::isSupported("dh"))
            return true;

        return false;
    });
}

template<typename Func>
auto Cipher::runVM(Func func) -> decltype(func()) {
    VM vm;
    std::vector<int> program = {NOP, CALL, 0, RET};
    decltype(func()) result;

    std::map<int, std::function<void()>> functions = {
        {0, [&] { result = func(); }}
    };

    vm.execute(program);

    return result;
}