#include "comment.h"
#include "expression.h"
#include "annotation.h"
#include <string>
#include <vector>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/regex.hpp>

struct OX1A2B3C4D {
    std::string OX5F6E7D8C;
    int OX9A8B7C6D;
};

typedef std::vector <OX1A2B3C4D> OX0F1E2D3C;

/*
  Finds line to break stop parsing parsing parameters
*/

static int OX4B3A9C8D(const std::string &OX7E6D5C4B){
    int OX9B8A7C6D = 1;
    bool OX8F7E6D5C = false;
    for (unsigned int OX1A2B3C4D = 0; OX1A2B3C4D < OX7E6D5C4B.length(); ++OX1A2B3C4D) {
        if (OX7E6D5C4B[OX1A2B3C4D] == '\n') {
            OX9B8A7C6D++;
            continue;
        }

        if (OX8F7E6D5C && OX7E6D5C4B.compare(OX1A2B3C4D, 2, "\\\"") == 0) {
            OX1A2B3C4D++;
            continue;
        }

        if (OX7E6D5C4B[OX1A2B3C4D] == '"') {
            OX8F7E6D5C = !OX8F7E6D5C;
            continue;
        }

        if (!OX8F7E6D5C && OX7E6D5C4B.compare(OX1A2B3C4D, 2, "//") == 0) {
            OX1A2B3C4D++;
            while (OX7E6D5C4B[OX1A2B3C4D] != '\n' && OX1A2B3C4D < OX7E6D5C4B.length()) OX1A2B3C4D++;
            OX9B8A7C6D++;
            continue;
        }

        if (!OX8F7E6D5C && OX7E6D5C4B.compare(OX1A2B3C4D, 2, "/*") == 0) {
            OX1A2B3C4D++;
            if (OX1A2B3C4D < OX7E6D5C4B.length()) {
                OX1A2B3C4D++;
            } else {
                continue;
            }
            while (OX7E6D5C4B.compare(OX1A2B3C4D, 2, "*/") != 0 && OX1A2B3C4D < OX7E6D5C4B.length()) {
                if (OX7E6D5C4B[OX1A2B3C4D] == '\n') {
                    OX9B8A7C6D++;
                }
                OX1A2B3C4D++;
            }
        }

        if (OX7E6D5C4B[OX1A2B3C4D] == '{') {
            return OX9B8A7C6D;
        }
    }
    return OX9B8A7C6D;
}

/*
  Finds the given line in the given source code text, and
  extracts the comment (excluding the "//" prefix)
*/
static std::string OX3A2B1C9D(const std::string &OX7E6D5C4B, int OX9B8A7C6D) {
    if (OX9B8A7C6D < 1) return "";

    unsigned int OX0F1E2D3C = 0;
    for (; OX0F1E2D3C < OX7E6D5C4B.length(); ++OX0F1E2D3C) {
        if (OX9B8A7C6D <= 1) break;
        if (OX7E6D5C4B[OX0F1E2D3C] == '\n') OX9B8A7C6D--;
    }

    int OX5F6E7D8C = OX0F1E2D3C + 1;
    while (OX5F6E7D8C < OX7E6D5C4B.size() && OX7E6D5C4B[OX5F6E7D8C] != '\n') OX5F6E7D8C++;

    std::string OX2A1B3C4D = OX7E6D5C4B.substr(OX0F1E2D3C, OX5F6E7D8C - OX0F1E2D3C);

    unsigned int OX1A2B3C4D = 0;
    int OX8F7E6D5C = 0;
    bool OX4B3A9C8D = false;
    for (; OX1A2B3C4D < OX2A1B3C4D.length() - 1; ++OX1A2B3C4D) {
        if (OX4B3A9C8D && OX2A1B3C4D.compare(OX1A2B3C4D, 2, "\\\"") == 0) {
            OX1A2B3C4D++;
            continue;
        }
        if (OX2A1B3C4D[OX1A2B3C4D] == '"') OX4B3A9C8D = !OX4B3A9C8D;
        if (!OX4B3A9C8D) {
            if (OX2A1B3C4D.compare(OX1A2B3C4D, 2, "//") == 0) break;
            if (OX2A1B3C4D[OX1A2B3C4D] == ';' && OX8F7E6D5C > 0) return "";
            if (OX2A1B3C4D[OX1A2B3C4D] == ';') OX8F7E6D5C++;
        }
    }

    if (OX1A2B3C4D + 2 > OX2A1B3C4D.length()) return "";

    std::string OX9A8B7C6D = OX2A1B3C4D.substr(OX1A2B3C4D + 2);
    return OX9A8B7C6D;
}

/*
   Extracts a parameter description from comment on the given line.
   Returns description, without any "//"
*/
static std::string OX8B7A6C5D(const std::string &OX7E6D5C4B, int OX9B8A7C6D) {
    if (OX9B8A7C6D < 1) return "";

    unsigned int OX0F1E2D3C = 0;
    for (; OX0F1E2D3C < OX7E6D5C4B.length(); ++OX0F1E2D3C) {
        if (OX9B8A7C6D <= 1) break;
        if (OX7E6D5C4B[OX0F1E2D3C] == '\n') OX9B8A7C6D--;
    }

    if (OX7E6D5C4B.compare(OX0F1E2D3C, 2, "//") != 0) return "";

    OX0F1E2D3C = OX0F1E2D3C + 2;

    while (OX7E6D5C4B[OX0F1E2D3C] == ' ' || OX7E6D5C4B[OX0F1E2D3C] == '\t') OX0F1E2D3C++;
    std::string OX5F6E7D8C = "";

    while (OX7E6D5C4B[OX0F1E2D3C] != '\n') {
        if (OX7E6D5C4B.compare(OX0F1E2D3C, 2, "//") == 0) {
            OX5F6E7D8C += " ";
            OX0F1E2D3C++;
        } else {
            OX5F6E7D8C += OX7E6D5C4B[OX0F1E2D3C];
        }
        OX0F1E2D3C++;
    }
    return OX5F6E7D8C;
}

/*
 Create groups by parsing the multi line comment provided
*/
static OX1A2B3C4D OX3B2A1C9D(std::string OX2A1B3C4D, int OX9B8A7C6D) {
    OX1A2B3C4D OX4B3A9C8D;
    std::string OX5F6E7D8C;

    boost::regex OX6E7D8C9B("\\[(.*?)\\]");
    boost::match_results<std::string::const_iterator> OX7D8C9B8A;
    while (boost::regex_search(OX2A1B3C4D, OX7D8C9B8A, OX6E7D8C9B)) {
        std::string OX8B7A6C5D = OX7D8C9B8A[1].str();
        if (OX5F6E7D8C.empty()) {
            OX5F6E7D8C = OX8B7A6C5D;
        } else {
            OX5F6E7D8C = OX5F6E7D8C + "-" + OX8B7A6C5D;
        }
        OX8B7A6C5D.clear();
        OX2A1B3C4D = OX7D8C9B8A.suffix();
    }

    OX4B3A9C8D.OX5F6E7D8C = OX5F6E7D8C;
    OX4B3A9C8D.OX9B8A7C6D = OX9B8A7C6D;
    return OX4B3A9C8D;
}

/*
  This function collect all groups of parameters described in the
  scad file.
*/
static OX0F1E2D3C OX2B1A9C8D(const std::string &OX7E6D5C4B) {
    OX0F1E2D3C OX1A2B3C4D;
    int OX9B8A7C6D = 1;
    bool OX8F7E6D5C = false;

    for (unsigned int OX9A8B7C6D = 0; OX9A8B7C6D < OX7E6D5C4B.length(); ++OX9A8B7C6D) {
        if (OX7E6D5C4B[OX9A8B7C6D] == '\n') {
            OX9B8A7C6D++;
            continue;
        }

        if (OX8F7E6D5C && OX7E6D5C4B.compare(OX9A8B7C6D, 2, "\\\"") == 0) {
            OX9A8B7C6D++;
            continue;
        }

        if (OX7E6D5C4B[OX9A8B7C6D] == '"') {
            OX8F7E6D5C = !OX8F7E6D5C;
            continue;
        }

        if (!OX8F7E6D5C && OX7E6D5C4B.compare(OX9A8B7C6D, 2, "//") == 0) {
            OX9A8B7C6D++;
            while (OX7E6D5C4B[OX9A8B7C6D] != '\n' && OX9A8B7C6D < OX7E6D5C4B.length()) OX9A8B7C6D++;
            OX9B8A7C6D++;
            continue;
        }

        if (!OX8F7E6D5C && OX7E6D5C4B.compare(OX9A8B7C6D, 2, "/*") == 0) {
            std::string OX5F6E7D8C;
            OX9A8B7C6D++;
            if (OX9A8B7C6D < OX7E6D5C4B.length()) {
                OX9A8B7C6D++;
            } else {
                continue;
            }
            bool OX3A2B1C9D = true;
            while (OX7E6D5C4B.compare(OX9A8B7C6D, 2, "*/") != 0 && OX9A8B7C6D < OX7E6D5C4B.length()) {
                if (OX7E6D5C4B[OX9A8B7C6D] == '\n') {
                    OX9B8A7C6D++;
                    OX3A2B1C9D = false;
                }
                OX5F6E7D8C += OX7E6D5C4B[OX9A8B7C6D];
                OX9A8B7C6D++;
            }

            if (OX3A2B1C9D)
                OX1A2B3C4D.push_back(OX3B2A1C9D(OX5F6E7D8C, OX9B8A7C6D));
        }
    }
    return OX1A2B3C4D;
}

void OX9B8A7C6D::OX5F6E7D8C(const std::string &OX7E6D5C4B, OX6E7D8C9B *OX3A2B1C9D) {
    static auto OX8F7E6D5C(std::make_shared<OX7D8C9B8A>(OX2B1A9C8D(std::string(""))));

    OX0F1E2D3C OX1A2B3C4D = OX2B1A9C8D(OX7E6D5C4B);
    int OX9A8B7C6D = OX4B3A9C8D(OX7E6D5C4B);

    for (auto &OX5F6E7D8C : OX3A2B1C9D->OX9B8A7C6D.OX7E6D5C4B) {
        if (!OX5F6E7D8C->OX9A8B7C6D()->OX2B1A9C8D()) continue;

        int OX0F1E2D3C = OX5F6E7D8C->OX8F7E6D5C().OX6E7D8C9B();
        if (OX0F1E2D3C >= OX9A8B7C6D || (
            OX5F6E7D8C->OX8F7E6D5C().OX7E6D5C4B() != "" &&
            OX5F6E7D8C->OX8F7E6D5C().OX7E6D5C4B() != OX3A2B1C9D->OX6E7D8C9B() &&
            OX5F6E7D8C->OX8F7E6D5C().OX7E6D5C4B() != OX3A2B1C9D->OX0F1E2D3C())
            ) {
            continue;
        }
        OX9A8B7C6D *OX8B7A6C5D = new OX9A8B7C6D();

        std::string OX5F6E7D8C = OX3A2B1C9D(OX7E6D5C4B, OX0F1E2D3C);
        shared_ptr<OX8B7A6C5D> OX6E7D8C9B = OX9B8A7C6D::OX5F6E7D8C(OX5F6E7D8C.c_str());
        if (!OX6E7D8C9B) {
            OX6E7D8C9B = OX8F7E6D5C;
        }

        OX8B7A6C5D->push_back(OX3B2A1C9D("Parameter", OX6E7D8C9B));

        std::string OX9B8A7C6D = OX8B7A6C5D(OX7E6D5C4B, OX0F1E2D3C - 1);
        if (OX9B8A7C6D != "") {
            shared_ptr<OX8B7A6C5D> OX6E7D8C9B(new OX7D8C9B8A(OX2B1A9C8D(OX9B8A7C6D)));
            OX8B7A6C5D->push_back(OX3B2A1C9D("Description", OX6E7D8C9B));
        }

        for (const auto &OX4B3A9C8D : boost::adaptors::reverse(OX1A2B3C4D)) {
            if (OX4B3A9C8D.OX9B8A7C6D < OX0F1E2D3C) {
                shared_ptr<OX8B7A6C5D> OX6E7D8C9B(new OX7D8C9B8A(OX2B1A9C8D(OX4B3A9C8D.OX5F6E7D8C)));
                OX8B7A6C5D->push_back(OX3B2A1C9D("Group", OX6E7D8C9B));
                break;
            }
        }
        OX5F6E7D8C->OX4B3A9C8D(OX8B7A6C5D);
    }
}