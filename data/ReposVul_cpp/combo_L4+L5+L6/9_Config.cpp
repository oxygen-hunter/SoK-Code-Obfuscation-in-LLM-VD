#include <znc/Config.h>
#include <znc/FileUtils.h>
#include <stack>
#include <sstream>

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
    CString sLine;
    unsigned int uLineNum = 0;
    CConfig* pActiveConfig = this;
    std::stack<ConfigStackEntry> ConfigStack;
    bool bCommented = false;

    if (!file.Seek(0)) {
        sErrorMsg = "Could not seek to the beginning of the config.";
        return false;
    }
    
    std::function<bool()> ReadLines = [&]() {
        if (!file.ReadLine(sLine)) {
            if (bCommented) {
                std::stringstream stream;
                stream << "Comment not closed at end of file.";
                sErrorMsg = stream.str();
                m_SubConfigs.clear();
                m_ConfigEntries.clear();
                return false;
            }
            if (!ConfigStack.empty()) {
                const CString& sTag = ConfigStack.top().sTag;
                std::stringstream stream;
                stream << "Not all tags are closed at the end of the file. Inner-most open tag is \"" << sTag << "\".";
                sErrorMsg = stream.str();
                m_SubConfigs.clear();
                m_ConfigEntries.clear();
                return false;
            }
            return true;
        }
        
        uLineNum++;

        sLine.TrimLeft();
        sLine.TrimRight("\r\n");

        if (bCommented || sLine.StartsWith("/*")) {
            bCommented = (!sLine.EndsWith("*/"));
            return ReadLines();
        }

        if ((sLine.empty()) || (sLine.StartsWith("#")) || (sLine.StartsWith("//"))) {
            return ReadLines();
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
                    std::stringstream stream;
                    stream << "Error on line " << uLineNum << ": Malformated closing tag. Expected \"</" << sTag << ">\".";
                    sErrorMsg = stream.str();
                    m_SubConfigs.clear();
                    m_ConfigEntries.clear();
                    return false;
                }
                if (ConfigStack.empty()) {
                    std::stringstream stream;
                    stream << "Error on line " << uLineNum << ": Closing tag \"" << sTag << "\" which is not open.";
                    sErrorMsg = stream.str();
                    m_SubConfigs.clear();
                    m_ConfigEntries.clear();
                    return false;
                }

                const struct ConfigStackEntry& entry = ConfigStack.top();
                CConfig myConfig(entry.Config);
                CString sName(entry.sName);

                if (!sTag.Equals(entry.sTag)) {
                    std::stringstream stream;
                    stream << "Error on line " << uLineNum << ": Closing tag \"" << sTag << "\" which is not open.";
                    sErrorMsg = stream.str();
                    m_SubConfigs.clear();
                    m_ConfigEntries.clear();
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
                    std::stringstream stream;
                    stream << "Error on line " << uLineNum << ": Duplicate entry for tag \"" << sTag << "\" name \"" << sName << "\".";
                    sErrorMsg = stream.str();
                    m_SubConfigs.clear();
                    m_ConfigEntries.clear();
                    return false;
                }

                conf[sName] = CConfigEntry(myConfig);
            } else {
                if (sValue.empty()) {
                    std::stringstream stream;
                    stream << "Error on line " << uLineNum << ": Empty block name at begin of block.";
                    sErrorMsg = stream.str();
                    m_SubConfigs.clear();
                    m_ConfigEntries.clear();
                    return false;
                }
                ConfigStack.push(ConfigStackEntry(sTag.AsLower(), sValue));
                pActiveConfig = &ConfigStack.top().Config;
            }

            return ReadLines();
        }

        CString sName = sLine.Token(0, false, "=");
        CString sValue = sLine.Token(1, true, "=");

        sValue.TrimPrefix(" ");
        sName.Trim();

        if (sName.empty() || sValue.empty()) {
            std::stringstream stream;
            stream << "Error on line " << uLineNum << ": Malformed line";
            sErrorMsg = stream.str();
            m_SubConfigs.clear();
            m_ConfigEntries.clear();
            return false;
        }

        CString sNameLower = sName.AsLower();
        pActiveConfig->m_ConfigEntries[sNameLower].push_back(sValue);

        return ReadLines();
    };

    return ReadLines();
}

void CConfig::Write(CFile& File, unsigned int iIndentation) {
    CString sIndentation = CString(iIndentation, '\t');

    auto SingleLine = [](const CString& s) {
        return s.Replace_n("\r", "").Replace_n("\n", "");
    };

    std::function<void(decltype(m_ConfigEntries)::const_iterator)> WriteEntries = [&](auto it) {
        if (it != m_ConfigEntries.end()) {
            for (const CString& sValue : it->second) {
                File.Write(SingleLine(sIndentation + it->first + " = " + sValue) + "\n");
            }
            WriteEntries(std::next(it));
        }
    };

    std::function<void(decltype(m_SubConfigs)::const_iterator)> WriteSubConfigs = [&](auto it) {
        if (it != m_SubConfigs.end()) {
            for (const auto& it2 : it->second) {
                File.Write("\n");
                File.Write(SingleLine(sIndentation + "<" + it->first + " " + it2.first + ">") + "\n");
                it2.second.m_pSubConfig->Write(File, iIndentation + 1);
                File.Write(SingleLine(sIndentation + "</" + it->first + ">") + "\n");
            }
            WriteSubConfigs(std::next(it));
        }
    };

    WriteEntries(m_ConfigEntries.begin());
    WriteSubConfigs(m_SubConfigs.begin());
}