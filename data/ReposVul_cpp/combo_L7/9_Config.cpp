// SPDX-License-Identifier: Apache-2.0
#include <znc/Config.h>
#include <znc/FileUtils.h>
#include <stack>
#include <sstream>

struct ConfigStackEntry {
    CString sTag;
    CString sName;
    CConfig Config;

    ConfigStackEntry(const CString& T, const CString N)
        : sTag(T), sName(N), Config() {}
};

CConfigEntry::CConfigEntry() : m_pSubConfig(nullptr) {}

CConfigEntry::CConfigEntry(const CConfig& C)
    : m_pSubConfig(new CConfig(C)) {}

CConfigEntry::CConfigEntry(const CConfigEntry& o) : m_pSubConfig(nullptr) {
    if (o.m_pSubConfig) m_pSubConfig = new CConfig(*o.m_pSubConfig);
}

CConfigEntry::~CConfigEntry() { delete m_pSubConfig; }

CConfigEntry& CConfigEntry::operator=(const CConfigEntry& o) {
    delete m_pSubConfig;
    if (o.m_pSubConfig)
        m_pSubConfig = new CConfig(*o.m_pSubConfig);
    else
        m_pSubConfig = nullptr;
    return *this;
}

bool CConfig::Parse(CFile& f, CString& e) {
    CString s;
    unsigned int u = 0;
    CConfig* p = this;
    std::stack<ConfigStackEntry> c;
    bool b = false;

    if (!f.Seek(0)) {
        e = "Could not seek to the beginning of the config.";
        return false;
    }

    while (f.ReadLine(s)) {
        u++;

#define ERROR(arg)                                         \
    do {                                                   \
        std::stringstream ss;                              \
        ss << "Error on line " << u << ": " << arg;        \
        e = ss.str();                                      \
        m_SubConfigs.clear();                              \
        m_ConfigEntries.clear();                           \
        return false;                                      \
    } while (0)

        s.TrimLeft();
        s.TrimRight("\r\n");

        if (b || s.StartsWith("/*")) {
            b = (!s.EndsWith("*/"));
            continue;
        }

        if ((s.empty()) || (s.StartsWith("#")) || (s.StartsWith("//"))) {
            continue;
        }

        if ((s.StartsWith("<")) && (s.EndsWith(">"))) {
            s.LeftChomp();
            s.RightChomp();
            s.Trim();

            CString t = s.Token(0);
            CString v = s.Token(1, true);

            t.Trim();
            v.Trim();

            if (t.TrimPrefix("/")) {
                if (!v.empty())
                    ERROR("Malformated closing tag. Expected \"</" << t
                                                                   << ">\".");
                if (c.empty())
                    ERROR("Closing tag \"" << t << "\" which is not open.");

                const struct ConfigStackEntry& entry = c.top();
                CConfig myConfig(entry.Config);
                CString n(entry.sName);

                if (!t.Equals(entry.sTag))
                    ERROR("Closing tag \"" << t << "\" which is not open.");

                c.pop();

                if (c.empty())
                    p = this;
                else
                    p = &c.top().Config;

                SubConfig& conf = p->m_SubConfigs[t.AsLower()];
                SubConfig::const_iterator it = conf.find(n);

                if (it != conf.end())
                    ERROR("Duplicate entry for tag \"" << t << "\" name \""
                                                       << n << "\".");

                conf[n] = CConfigEntry(myConfig);
            } else {
                if (v.empty())
                    ERROR("Empty block name at begin of block.");
                c.push(ConfigStackEntry(t.AsLower(), v));
                p = &c.top().Config;
            }

            continue;
        }

        CString n = s.Token(0, false, "=");
        CString v = s.Token(1, true, "=");

        v.TrimPrefix(" ");
        n.Trim();

        if (n.empty() || v.empty()) ERROR("Malformed line");

        CString nLower = n.AsLower();
        p->m_ConfigEntries[nLower].push_back(v);
    }

    if (b) ERROR("Comment not closed at end of file.");

    if (!c.empty()) {
        const CString& t = c.top().sTag;
        ERROR("Not all tags are closed at the end of the file. Inner-most open tag is \""
            << t << "\".");
    }

    return true;
}

void CConfig::Write(CFile& F, unsigned int I) {
    CString i = CString(I, '\t');

    auto S = [](const CString& s) {
        return s.Replace_n("\r", "").Replace_n("\n", "");
    };

    for (const auto& it : m_ConfigEntries) {
        for (const CString& v : it.second) {
            F.Write(S(i + it.first + " = " + v) + "\n");
        }
    }

    for (const auto& it : m_SubConfigs) {
        for (const auto& it2 : it.second) {
            F.Write("\n");

            F.Write(S(i + "<" + it.first + " " +
                                  it2.first + ">") +
                       "\n");
            it2.second.m_pSubConfig->Write(F, I + 1);
            F.Write(S(i + "</" + it.first + ">") + "\n");
        }
    }
}