#include <znc/Config.h>
#include <znc/FileUtils.h>
#include <stack>
#include <sstream>

struct OX8A9C3D2E {
    CString OX2F7F4B2C;
    CString OX3B2F1C4A;
    CConfig OX1D5E9A8B;

    OX8A9C3D2E(const CString& OX4F8D2A3B, const CString OX5E7A9C2D)
        : OX2F7F4B2C(OX4F8D2A3B), OX3B2F1C4A(OX5E7A9C2D), OX1D5E9A8B() {}
};

CConfigEntry::CConfigEntry() : OX7B2D4E5C(nullptr) {}

CConfigEntry::CConfigEntry(const CConfig& OX9D2F4B1C)
    : OX7B2D4E5C(new CConfig(OX9D2F4B1C)) {}

CConfigEntry::CConfigEntry(const CConfigEntry& OX6C1D3A8E) : OX7B2D4E5C(nullptr) {
    if (OX6C1D3A8E.OX7B2D4E5C) OX7B2D4E5C = new CConfig(*OX6C1D3A8E.OX7B2D4E5C);
}

CConfigEntry::~CConfigEntry() { delete OX7B2D4E5C; }

CConfigEntry& CConfigEntry::operator=(const CConfigEntry& OX8E2D5A3C) {
    delete OX7B2D4E5C;
    if (OX8E2D5A3C.OX7B2D4E5C)
        OX7B2D4E5C = new CConfig(*OX8E2D5A3C.OX7B2D4E5C);
    else
        OX7B2D4E5C = nullptr;
    return *this;
}

bool CConfig::Parse(CFile& OX6E1D2A3B, CString& OX9A4C2D7E) {
    CString OX2C7F1A9D;
    unsigned int OX5A8C3E1D = 0;
    CConfig* OX7D1C3E5A = this;
    std::stack<OX8A9C3D2E> OX9B2F4D1A;
    bool OX3E5A7C2D = false;

    if (!OX6E1D2A3B.Seek(0)) {
        OX9A4C2D7E = "Could not seek to the beginning of the config.";
        return false;
    }

    while (OX6E1D2A3B.ReadLine(OX2C7F1A9D)) {
        OX5A8C3E1D++;

#define ERROR(arg)                                             \
    do {                                                       \
        std::stringstream OX9D3A1B7C;                          \
        OX9D3A1B7C << "Error on line " << OX5A8C3E1D << ": " << arg; \
        OX9A4C2D7E = OX9D3A1B7C.str();                         \
        OX3C7B1D2E.clear();                                    \
        OX6A9E4C3D.clear();                                    \
        return false;                                          \
    } while (0)

        OX2C7F1A9D.TrimLeft();
        OX2C7F1A9D.TrimRight("\r\n");

        if (OX3E5A7C2D || OX2C7F1A9D.StartsWith("/*")) {
            OX3E5A7C2D = (!OX2C7F1A9D.EndsWith("*/"));
            continue;
        }

        if ((OX2C7F1A9D.empty()) || (OX2C7F1A9D.StartsWith("#")) ||
            (OX2C7F1A9D.StartsWith("//"))) {
            continue;
        }

        if ((OX2C7F1A9D.StartsWith("<")) && (OX2C7F1A9D.EndsWith(">"))) {
            OX2C7F1A9D.LeftChomp();
            OX2C7F1A9D.RightChomp();
            OX2C7F1A9D.Trim();

            CString OX8C1E7A2D = OX2C7F1A9D.Token(0);
            CString OX5D4E6B3C = OX2C7F1A9D.Token(1, true);

            OX8C1E7A2D.Trim();
            OX5D4E6B3C.Trim();

            if (OX8C1E7A2D.TrimPrefix("/")) {
                if (!OX5D4E6B3C.empty())
                    ERROR("Malformated closing tag. Expected \"</" << OX8C1E7A2D
                                                                   << ">\".");
                if (OX9B2F4D1A.empty())
                    ERROR("Closing tag \"" << OX8C1E7A2D << "\" which is not open.");

                const struct OX8A9C3D2E& OX4E1D7C3B = OX9B2F4D1A.top();
                CConfig OX2D4B7E6A(OX4E1D7C3B.OX1D5E9A8B);
                CString OX9A3D4E5B(OX4E1D7C3B.OX3B2F1C4A);

                if (!OX8C1E7A2D.Equals(OX4E1D7C3B.OX2F7F4B2C))
                    ERROR("Closing tag \"" << OX8C1E7A2D << "\" which is not open.");

                OX9B2F4D1A.pop();

                if (OX9B2F4D1A.empty())
                    OX7D1C3E5A = this;
                else
                    OX7D1C3E5A = &OX9B2F4D1A.top().OX1D5E9A8B;

                OX7B2D4C1E& OX5E9A4C3B = OX7D1C3E5A->OX3C7B1D2E[OX8C1E7A2D.AsLower()];
                OX7B2D4C1E::const_iterator OX2E5A7C4B = OX5E9A4C3B.find(OX9A3D4E5B);

                if (OX2E5A7C4B != OX5E9A4C3B.end())
                    ERROR("Duplicate entry for tag \"" << OX8C1E7A2D << "\" name \""
                                                       << OX9A3D4E5B << "\".");

                OX5E9A4C3B[OX9A3D4E5B] = CConfigEntry(OX2D4B7E6A);
            } else {
                if (OX5D4E6B3C.empty())
                    ERROR("Empty block name at begin of block.");
                OX9B2F4D1A.push(OX8A9C3D2E(OX8C1E7A2D.AsLower(), OX5D4E6B3C));
                OX7D1C3E5A = &OX9B2F4D1A.top().OX1D5E9A8B;
            }

            continue;
        }

        CString OX8E1D4A7B = OX2C7F1A9D.Token(0, false, "=");
        CString OX9A5D7C3E = OX2C7F1A9D.Token(1, true, "=");

        OX9A5D7C3E.TrimPrefix(" ");
        OX8E1D4A7B.Trim();

        if (OX8E1D4A7B.empty() || OX9A5D7C3E.empty()) ERROR("Malformed line");

        CString OX6D3E9A2B = OX8E1D4A7B.AsLower();
        OX7D1C3E5A->OX6A9E4C3D[OX6D3E9A2B].push_back(OX9A5D7C3E);
    }

    if (OX3E5A7C2D) ERROR("Comment not closed at end of file.");

    if (!OX9B2F4D1A.empty()) {
        const CString& OX5C3A7E1D = OX9B2F4D1A.top().OX2F7F4B2C;
        ERROR(
            "Not all tags are closed at the end of the file. Inner-most open "
            "tag is \""
            << OX5C3A7E1D << "\".");
    }

    return true;
}

void CConfig::Write(CFile& OX4F1D2A9B, unsigned int OX9E3B5C2D) {
    CString OX2B7F4E1C = CString(OX9E3B5C2D, '\t');

    auto OX5A7C3D1E = [](const CString& OX2D4C7B1E) {
        return OX2D4C7B1E.Replace_n("\r", "").Replace_n("\n", "");
    };

    for (const auto& OX3D9A4B2C : OX6A9E4C3D) {
        for (const CString& OX1B7E5C3A : OX3D9A4B2C.second) {
            OX4F1D2A9B.Write(OX5A7C3D1E(OX2B7F4E1C + OX3D9A4B2C.first + " = " + OX1B7E5C3A) +
                       "\n");
        }
    }

    for (const auto& OX8E2D4C5A : OX3C7B1D2E) {
        for (const auto& OX3B9C2E1D : OX8E2D4C5A.second) {
            OX4F1D2A9B.Write("\n");

            OX4F1D2A9B.Write(OX5A7C3D1E(OX2B7F4E1C + "<" + OX8E2D4C5A.first + " " +
                                  OX3B9C2E1D.first + ">") +
                       "\n");
            OX3B9C2E1D.second.OX7B2D4E5C->Write(OX4F1D2A9B, OX9E3B5C2D + 1);
            OX4F1D2A9B.Write(OX5A7C3D1E(OX2B7F4E1C + "</" + OX8E2D4C5A.first + ">") + "\n");
        }
    }
}