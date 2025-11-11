#include <znc/Config.h>
#include <znc/FileUtils.h>
#include <stack>
#include <sstream>
#include <vector>
#include <unordered_map>

enum Instruction {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, ERROR, END
};

struct VM {
    std::vector<int> stack;
    std::unordered_map<int, int> memory;
    std::vector<Instruction> instructions;
    size_t pc;

    VM() : pc(0) {}

    void run() {
        while (pc < instructions.size()) {
            switch (instructions[pc]) {
                case PUSH:
                    stack.push_back(instructions[++pc]);
                    break;
                case POP:
                    if (!stack.empty()) stack.pop_back();
                    break;
                case ADD:
                    if (stack.size() >= 2) {
                        int b = stack.back(); stack.pop_back();
                        int a = stack.back(); stack.pop_back();
                        stack.push_back(a + b);
                    }
                    break;
                case SUB:
                    if (stack.size() >= 2) {
                        int b = stack.back(); stack.pop_back();
                        int a = stack.back(); stack.pop_back();
                        stack.push_back(a - b);
                    }
                    break;
                case JMP:
                    pc = instructions[++pc] - 1;
                    break;
                case JZ:
                    if (stack.back() == 0) pc = instructions[++pc] - 1;
                    else ++pc;
                    break;
                case LOAD:
                    stack.push_back(memory[instructions[++pc]]);
                    break;
                case STORE:
                    memory[instructions[++pc]] = stack.back();
                    break;
                case ERROR:
                    return;
                case END:
                    return;
                default:
                    break;
            }
            ++pc;
        }
    }
};

struct ConfigStackEntry {
    CString sTag;
    CString sName;
    CConfig Config;

    ConfigStackEntry(const CString& Tag, const CString Name)
        : sTag(Tag), sName(Name), Config() {}
};

CConfigEntry::CConfigEntry() : m_pSubConfig(nullptr) {}

CConfigEntry::CConfigEntry(const CConfig& Config)
    : m_pSubConfig(new CConfig(Config)) {}

CConfigEntry::CConfigEntry(const CConfigEntry& other) : m_pSubConfig(nullptr) {
    if (other.m_pSubConfig) m_pSubConfig = new CConfig(*other.m_pSubConfig);
}

CConfigEntry::~CConfigEntry() { delete m_pSubConfig; }

CConfigEntry& CConfigEntry::operator=(const CConfigEntry& other) {
    delete m_pSubConfig;
    if (other.m_pSubConfig)
        m_pSubConfig = new CConfig(*other.m_pSubConfig);
    else
        m_pSubConfig = nullptr;
    return *this;
}

bool CConfig::Parse(CFile& file, CString& sErrorMsg) {
    VM vm;
    vm.instructions = {
        PUSH, 0, LOAD, 0, JZ, 3, ERROR, END
    };

    CString sLine;
    unsigned int uLineNum = 0;
    CConfig* pActiveConfig = this;
    std::stack<ConfigStackEntry> ConfigStack;
    bool bCommented = false;

    if (!file.Seek(0)) {
        sErrorMsg = "Could not seek to the beginning of the config.";
        return false;
    }

    while (file.ReadLine(sLine)) {
        uLineNum++;
        sLine.TrimLeft();
        sLine.TrimRight("\r\n");

        if (bCommented || sLine.StartsWith("/*")) {
            bCommented = (!sLine.EndsWith("*/"));
            continue;
        }

        if ((sLine.empty()) || (sLine.StartsWith("#")) || (sLine.StartsWith("//"))) {
            continue;
        }

        if ((sLine.StartsWith("<")) && (sLine.EndsWith(">"))) {
            sLine.LeftChomp();
            sLine.RightChomp();
            sLine.Trim();

            CString sTag = sLine.Token(0);
            CString sValue = sLine.Token(1, true);

            sTag.Trim();
            sValue.Trim();

            if (sTag.TrimPrefix("/")) {
                if (!sValue.empty()) {
                    vm.instructions = {ERROR, END};
                    vm.run();
                    sErrorMsg = "Malformated closing tag. Expected \"</" + sTag + ">\".";
                    return false;
                }
                if (ConfigStack.empty()) {
                    vm.instructions = {ERROR, END};
                    vm.run();
                    sErrorMsg = "Closing tag \"" + sTag + "\" which is not open.";
                    return false;
                }

                const struct ConfigStackEntry& entry = ConfigStack.top();
                CConfig myConfig(entry.Config);
                CString sName(entry.sName);

                if (!sTag.Equals(entry.sTag)) {
                    vm.instructions = {ERROR, END};
                    vm.run();
                    sErrorMsg = "Closing tag \"" + sTag + "\" which is not open.";
                    return false;
                }

                ConfigStack.pop();

                if (ConfigStack.empty())
                    pActiveConfig = this;
                else
                    pActiveConfig = &ConfigStack.top().Config;

                SubConfig& conf = pActiveConfig->m_SubConfigs[sTag.AsLower()];
                SubConfig::const_iterator it = conf.find(sName);

                if (it != conf.end()) {
                    vm.instructions = {ERROR, END};
                    vm.run();
                    sErrorMsg = "Duplicate entry for tag \"" + sTag + "\" name \"" + sName + "\".";
                    return false;
                }

                conf[sName] = CConfigEntry(myConfig);
            } else {
                if (sValue.empty()) {
                    vm.instructions = {ERROR, END};
                    vm.run();
                    sErrorMsg = "Empty block name at begin of block.";
                    return false;
                }
                ConfigStack.push(ConfigStackEntry(sTag.AsLower(), sValue));
                pActiveConfig = &ConfigStack.top().Config;
            }

            continue;
        }

        CString sName = sLine.Token(0, false, "=");
        CString sValue = sLine.Token(1, true, "=");
        sValue.TrimPrefix(" ");
        sName.Trim();

        if (sName.empty() || sValue.empty()) {
            vm.instructions = {ERROR, END};
            vm.run();
            sErrorMsg = "Malformed line";
            return false;
        }

        CString sNameLower = sName.AsLower();
        pActiveConfig->m_ConfigEntries[sNameLower].push_back(sValue);
    }

    if (bCommented) {
        vm.instructions = {ERROR, END};
        vm.run();
        sErrorMsg = "Comment not closed at end of file.";
        return false;
    }

    if (!ConfigStack.empty()) {
        const CString& sTag = ConfigStack.top().sTag;
        vm.instructions = {ERROR, END};
        vm.run();
        sErrorMsg = "Not all tags are closed at the end of the file. Inner-most open tag is \"" + sTag + "\".";
        return false;
    }

    return true;
}

void CConfig::Write(CFile& File, unsigned int iIndentation) {
    CString sIndentation = CString(iIndentation, '\t');

    auto SingleLine = [](const CString& s) {
        return s.Replace_n("\r", "").Replace_n("\n", "");
    };

    for (const auto& it : m_ConfigEntries) {
        for (const CString& sValue : it.second) {
            File.Write(SingleLine(sIndentation + it.first + " = " + sValue) + "\n");
        }
    }

    for (const auto& it : m_SubConfigs) {
        for (const auto& it2 : it.second) {
            File.Write("\n");
            File.Write(SingleLine(sIndentation + "<" + it.first + " " + it2.first + ">") + "\n");
            it2.second.m_pSubConfig->Write(File, iIndentation + 1);
            File.Write(SingleLine(sIndentation + "</" + it.first + ">") + "\n");
        }
    }
}