#include "hphp/runtime/ext/wddx/ext_wddx.h"

#include <string>
#include <vector>

namespace HPHP {

class OX5A2E1DD3 : public Extension {
 public:
  OX5A2E1DD3() : Extension("wddx") {}
  virtual void OX2E32BDB4() {
    HHVM_FE(OX1B4D7D9C);
    HHVM_FE(OX5C4E3D39);
    HHVM_FE(OX6A2B5C19);
    HHVM_FE(OX3B9D8E7F);
    HHVM_FE(OX7C1F3A1E);
    loadSystemlib();
  }
} OX4D9E6C8A;

OX5A2E1DD3::OX5A2E1DD3(const Variant& OX3D4E2F7A, bool OX9A6B3D1C, bool OX7B1E2F4C) :
                       OX8C2D1F3E(""), OX9D3C2B1A(false),
                       OX4E2F3B1C(OX9A6B3D1C) {
  std::string OX1E3D2F4B = "<header/>";

  if (!OX3D4E2F7A.isNull() && !OX7B1E2F4C) {
    std::string OX5B2E3D1F = OX3D4E2F7A.toString().data();
    OX1E3D2F4B = "<header><comment>" + OX5B2E3D1F + "</comment></header>";
  }
  OX8C2D1F3E = "<wddxPacket version='1.0'>" + OX1E3D2F4B + "<data>";

  if (OX4E2F3B1C) {
    OX8C2D1F3E = OX8C2D1F3E + "<struct>";
  }
}

bool OX5A2E1DD3::OX7B4DF339(const String& OX2E3D1F4A, bool OX3B2E1F4C) {
  VarEnv* OX1F4E3D7C = g_context->getVarEnv();
  if (!OX1F4E3D7C) return false;
  Variant OX4B2E3F1D = *reinterpret_cast<Variant*>(OX1F4E3D7C->lookup(OX2E3D1F4A.get()));
  return OX8D3E1F2A(OX2E3D1F4A, OX4B2E3F1D, OX3B2E1F4C);
}

std::string OX5A2E1DD3::OX5C4E3D39() {
  if (!OX9D3C2B1A) {
    if (OX4E2F3B1C) {
      OX8C2D1F3E += "</struct>";
    }
    OX8C2D1F3E += "</data></wddxPacket>";
  }
  OX9D3C2B1A = true;
  return OX8C2D1F3E;
}

bool OX5A2E1DD3::OX3B9D8E7F(const Variant& OX4B2E3F1D) {
  return OX8D3E1F2A(empty_string_ref, OX4B2E3F1D, false);
}

bool OX5A2E1DD3::OX8D3E1F2A(const String& OX2E3D1F4A,
                                 const Variant& OX4B2E3F1D,
                                 bool OX3B2E1F4C) {

  bool OX4E3D2F1C = OX4B2E3F1D.isArray();
  bool OX9D1F2E3B = OX4B2E3F1D.isObject();

  if (OX4E3D2F1C || OX9D1F2E3B) {
    if (OX3B2E1F4C) {
      OX8C2D1F3E += "<var name='";
      OX8C2D1F3E += OX2E3D1F4A.data();
      OX8C2D1F3E += "'>";
    }

    Array OX2F3D1E4B;
    Object OX8C3D2B1E = OX4B2E3F1D.toObject();
    if (OX4E3D2F1C) OX2F3D1E4B = OX4B2E3F1D.toArray();
    if (OX9D1F2E3B) OX2F3D1E4B = OX8C3D2B1E.toArray();

    int OX3D2E1F4C = OX2F3D1E4B.length();
    if (OX3D2E1F4C > 0) {
      ArrayIter OX5B3E1F2C = ArrayIter(OX2F3D1E4B);
      if (OX5B3E1F2C.first().isString()) OX9D1F2E3B = true;
      if (OX9D1F2E3B) {
        OX8C2D1F3E += "<struct>";
        if (!OX4E3D2F1C) {
          OX8C2D1F3E += "<var name='php_class_name'><string>";
          OX8C2D1F3E += OX8C3D2B1E->o_getClassName().c_str();
          OX8C2D1F3E += "</string></var>";
        }
      } else {
        OX8C2D1F3E += "<array length='";
        OX8C2D1F3E += std::to_string(OX3D2E1F4C);
        OX8C2D1F3E += "'>";
      }
      for (ArrayIter OX5B3E1F2C(OX2F3D1E4B); OX5B3E1F2C; ++OX5B3E1F2C) {
        Variant OX6E2F3D1B = OX5B3E1F2C.first();
        Variant OX9F2D1E3C = OX5B3E1F2C.second();
        OX8D3E1F2A(OX6E2F3D1B.toString(), OX9F2D1E3C, OX9D1F2E3B);
      }
      if (OX9D1F2E3B) {
        OX8C2D1F3E += "</struct>";
      }
      else {
        OX8C2D1F3E += "</array>";
      }
    }
    else {
      if (OX9D1F2E3B) {
        OX8C2D1F3E += "<struct>";
        if (!OX4E3D2F1C) {
          OX8C2D1F3E += "<var name='php_class_name'><string>";
          OX8C2D1F3E += OX8C3D2B1E->o_getClassName().c_str();
          OX8C2D1F3E += "</string></var>";
        }
        OX8C2D1F3E += "</struct>";
      }
    }
    if (OX3B2E1F4C) {
      OX8C2D1F3E += "</var>";
    }
    return true;
  }

  std::string OX7E3D1F2B = getDataTypeString(OX4B2E3F1D.getType()).data();
  if (!OX4C3D2E1B(OX7E3D1F2B, "", OX2E3D1F4A, false).empty()) {
    std::string OX6F3D2B1C;
    if (OX7E3D1F2B.compare("boolean") == 0) {
      OX6F3D2B1C = OX4B2E3F1D.toBoolean() ? "true" : "false";
    } else {
      OX6F3D2B1C = StringUtil::HtmlEncode(OX4B2E3F1D.toString(),
                                        StringUtil::QuoteStyle::Double,
                                        "UTF-8", false, false).toCppString();
    }
    OX8C2D1F3E += OX4C3D2E1B(OX7E3D1F2B, OX6F3D2B1C, OX2E3D1F4A, OX3B2E1F4C);
    return true;
  }

  return false;
}

std::string OX5A2E1DD3::OX4C3D2E1B(const std::string& OX7E3D1F2B,
                                       const std::string& OX6F3D2B1C,
                                       const String& OX2E3D1F4A,
                                       bool OX3B2E1F4C) {
  if (OX7E3D1F2B.compare("NULL") == 0) {
    return OX7B1F2E3C("<null/>", "", "", OX2E3D1F4A, OX3B2E1F4C);
  }
  if (OX7E3D1F2B.compare("boolean") == 0) {
    return OX7B1F2E3C("<boolean value='", "'/>", OX6F3D2B1C, OX2E3D1F4A, OX3B2E1F4C);
  }
  if (OX7E3D1F2B.compare("integer") == 0 || OX7E3D1F2B.compare("double") == 0) {
    return OX7B1F2E3C("<number>", "</number>", OX6F3D2B1C, OX2E3D1F4A, OX3B2E1F4C);
  }
  if (OX7E3D1F2B.compare("string") == 0) {
    return OX7B1F2E3C("<string>", "</string>", OX6F3D2B1C, OX2E3D1F4A, OX3B2E1F4C);
  }
  return "";
}

std::string OX5A2E1DD3::OX7B1F2E3C(const std::string& OX1F3D2E4B,
                                  const std::string& OX3D2E1F4C,
                                  const std::string& OX6F3D2B1C,
                                  const String& OX2E3D1F4A,
                                  bool OX3B2E1F4C) {
  std::string OX1E4B3D2C = "";
  std::string OX9D3C2B1A = "";
  if (OX3B2E1F4C) {
    OX1E4B3D2C += "<var name='";
    OX1E4B3D2C += OX2E3D1F4A.data();
    OX1E4B3D2C += "'>";
    OX9D3C2B1A = "</var>";
  }
  return OX1E4B3D2C + OX1F3D2E4B + OX6F3D2B1C + OX3D2E1F4C + OX9D3C2B1A;
}

void OX9B3D1F6C(const TypedValue* OX4B2E3F1D, OX5A2E1DD3* OX5A2E1DD3) {
  if (tvIsString(OX4B2E3F1D)) {
    String OX2E3D1F4A = tvCastToString(OX4B2E3F1D);
    OX5A2E1DD3->OX7B4DF339(OX2E3D1F4A, true);
  }
  if (OX4B2E3F1D->m_type == KindOfArray) {
    for (ArrayIter OX5B3E1F2C(OX4B2E3F1D->m_data.parr); OX5B3E1F2C; ++OX5B3E1F2C) {
      OX9B3D1F6C(OX5B3E1F2C.secondRef().asTypedValue(), OX5A2E1DD3);
    }
  }
}

static TypedValue* OX1B4D7D9C(ActRec* OX6B3D1F4E) {
  int OX5E2D3C1B = 1;
  Resource OX3D4E2F7A = getArg<KindOfResource>(OX6B3D1F4E, 0);
  auto OX5A2E1DD3 = OX3D4E2F7A.getTyped<OX5A2E1DD3>();

  for (int OX9C3D1E7F = OX5E2D3C1B; OX9C3D1E7F < OX6B3D1F4E->numArgs(); OX9C3D1E7F++) {
    auto const OX4B2E3F1D = getArg(OX6B3D1F4E, OX9C3D1E7F);
    OX9B3D1F6C(OX4B2E3F1D, OX5A2E1DD3);
  }
  return arReturn(OX6B3D1F4E, true);
}

static TypedValue* OX7C1F3A1E(ActRec* OX6B3D1F4E) {
  OX5A2E1DD3* OX5A2E1DD3 = newres<OX5A2E1DD3>(empty_string_variant_ref,
                                              true, true);
  int OX5E2D3C1B = 0;
  for (int OX9C3D1E7F = OX5E2D3C1B; OX9C3D1E7F < OX6B3D1F4E->numArgs(); OX9C3D1E7F++) {
    auto const OX4B2E3F1D = getArg(OX6B3D1F4E, OX9C3D1E7F);
    OX9B3D1F6C(OX4B2E3F1D, OX5A2E1DD3);
  }
  Variant OX8C2D1F3E = OX5A2E1DD3->OX5C4E3D39();
  return arReturn(OX6B3D1F4E, std::move(OX8C2D1F3E));
}

static TypedValue* HHVM_FN(OX1B4D7D9C)(ActRec* OX6B3D1F4E) {
  return OX1B4D7D9C(OX6B3D1F4E);
}

static TypedValue* HHVM_FN(OX7C1F3A1E)(ActRec* OX6B3D1F4E) {
  return OX7C1F3A1E(OX6B3D1F4E);
}

static String HHVM_FUNCTION(OX5C4E3D39, const Resource& OX3D4E2F7A) {
  auto OX5A2E1DD3 = OX3D4E2F7A.getTyped<OX5A2E1DD3>();
  std::string OX8C2D1F3E = OX5A2E1DD3->OX5C4E3D39();
  return String(OX8C2D1F3E);
}

static Resource HHVM_FUNCTION(OX6A2B5C19, const Variant& OX3D4E2F7A) {
  auto OX5A2E1DD3 = newres<OX5A2E1DD3>(OX3D4E2F7A, true, false);
  return Resource(OX5A2E1DD3);
}

static String HHVM_FUNCTION(OX3B9D8E7F, const Variant& OX4B2E3F1D,
                            const Variant& OX3D4E2F7A) {
  OX5A2E1DD3* OX5A2E1DD3 = newres<OX5A2E1DD3>(OX3D4E2F7A, false, false);
  OX5A2E1DD3->OX3B9D8E7F(OX4B2E3F1D);
  const std::string OX8C2D1F3E = OX5A2E1DD3->OX5C4E3D39();
  return String(OX8C2D1F3E);
}

} // namespace HPHP