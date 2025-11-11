#include "hphp/runtime/ext/wddx/ext_wddx.h"
#include <string>
#include <vector>

namespace HPHP {

WddxPacket::WddxPacket(const Variant& comment, bool manualPacket, bool sVar) :
                       m_packetString(""), m_packetClosed(false),
                       m_manualPacketCreation(manualPacket) {
  std::string header = "<header/>";
  int state = 0;
  while (state >= 0) {
    switch (state) {
      case 0:
        if (!comment.isNull() && !sVar) {
          std::string scomment = comment.toString().data();
          header = "<header><comment>" + scomment + "</comment></header>";
        }
        m_packetString = "<wddxPacket version='1.0'>" + header + "<data>";
        state = 1;
        break;
      case 1:
        if (m_manualPacketCreation) {
          m_packetString = m_packetString + "<struct>";
        }
        state = -1;
        break;
    }
  }
}

bool WddxPacket::add_var(const String& varName, bool hasVarTag) {
  VarEnv* v = g_context->getVarEnv();
  if (!v) return false;
  Variant varVariant = *reinterpret_cast<Variant*>(v->lookup(varName.get()));
  return recursiveAddVar(varName, varVariant, hasVarTag);
}

std::string WddxPacket::packet_end() {
  int state = 0;
  while (state >= 0) {
    switch (state) {
      case 0:
        if (!m_packetClosed) {
          state = 1;
        } else {
          state = 3;
        }
        break;
      case 1:
        if (m_manualPacketCreation) {
          m_packetString += "</struct>";
        }
        state = 2;
        break;
      case 2:
        m_packetString += "</data></wddxPacket>";
        state = 3;
        break;
      case 3:
        m_packetClosed = true;
        return m_packetString;
    }
  }
  return m_packetString;
}

bool WddxPacket::serialize_value(const Variant& varVariant) {
  return recursiveAddVar(empty_string_ref, varVariant, false);
}

bool WddxPacket::recursiveAddVar(const String& varName,
                                 const Variant& varVariant,
                                 bool hasVarTag) {
  bool isArray = varVariant.isArray();
  bool isObject = varVariant.isObject();
  int state = 0;
  while (state >= 0) {
    switch (state) {
      case 0:
        if (isArray || isObject) {
          state = 1;
        } else {
          state = 9;
        }
        break;
      case 1:
        if (hasVarTag) {
          m_packetString += "<var name='";
          m_packetString += varName.data();
          m_packetString += "'>";
        }
        state = 2;
        break;
      case 2:
        Array varAsArray;
        Object varAsObject = varVariant.toObject();
        if (isArray) varAsArray = varVariant.toArray();
        if (isObject) varAsArray = varAsObject.toArray();
        int length = varAsArray.length();
        if (length > 0) {
          state = 3;
        } else {
          state = 7;
        }
        break;
      case 3:
        ArrayIter it = ArrayIter(varAsArray);
        if (it.first().isString()) isObject = true;
        if (isObject) {
          m_packetString += "<struct>";
          if (!isArray) {
            m_packetString += "<var name='php_class_name'><string>";
            m_packetString += varAsObject->o_getClassName().c_str();
            m_packetString += "</string></var>";
          }
        } else {
          m_packetString += "<array length='";
          m_packetString += std::to_string(length);
          m_packetString += "'>";
        }
        state = 4;
        break;
      case 4:
        for (ArrayIter it(varAsArray); it; ++it) {
          Variant key = it.first();
          Variant value = it.second();
          recursiveAddVar(key.toString(), value, isObject);
        }
        state = 5;
        break;
      case 5:
        if (isObject) {
          m_packetString += "</struct>";
        } else {
          m_packetString += "</array>";
        }
        state = 6;
        break;
      case 6:
        if (hasVarTag) {
          m_packetString += "</var>";
        }
        return true;
      case 7:
        if (isObject) {
          m_packetString += "<struct>";
          if (!isArray) {
            m_packetString += "<var name='php_class_name'><string>";
            m_packetString += varAsObject->o_getClassName().c_str();
            m_packetString += "</string></var>";
          }
          m_packetString += "</struct>";
        }
        state = 8;
        break;
      case 8:
        if (hasVarTag) {
          m_packetString += "</var>";
        }
        return true;
      case 9:
        std::string varType = getDataTypeString(varVariant.getType()).data();
        if (!getWddxEncoded(varType, "", varName, false).empty()) {
          std::string varValue;
          if (varType.compare("boolean") == 0) {
            varValue = varVariant.toBoolean() ? "true" : "false";
          } else {
            varValue = StringUtil::HtmlEncode(varVariant.toString(),
                                              StringUtil::QuoteStyle::Double,
                                              "UTF-8", false, false).toCppString();
          }
          m_packetString += getWddxEncoded(varType, varValue, varName, hasVarTag);
          return true;
        }
        return false;
    }
  }
  return false;
}

std::string WddxPacket::getWddxEncoded(const std::string& varType,
                                       const std::string& varValue,
                                       const String& varName,
                                       bool hasVarTag) {
  int state = 0;
  while (state >= 0) {
    switch (state) {
      case 0:
        if (varType.compare("NULL") == 0) {
          return wrapValue("<null/>", "", "", varName, hasVarTag);
        }
        state = 1;
        break;
      case 1:
        if (varType.compare("boolean") == 0) {
          return wrapValue("<boolean value='", "'/>", varValue, varName, hasVarTag);
        }
        state = 2;
        break;
      case 2:
        if (varType.compare("integer") == 0 || varType.compare("double") == 0) {
          return wrapValue("<number>", "</number>", varValue, varName, hasVarTag);
        }
        state = 3;
        break;
      case 3:
        if (varType.compare("string") == 0) {
          return wrapValue("<string>", "</string>", varValue, varName, hasVarTag);
        }
        return "";
    }
  }
  return "";
}

std::string WddxPacket::wrapValue(const std::string& start,
                                  const std::string& end,
                                  const std::string& varValue,
                                  const String& varName,
                                  bool hasVarTag) {
  std::string startVar = "";
  std::string endVar = "";
  int state = 0;
  while (state >= 0) {
    switch (state) {
      case 0:
        if (hasVarTag) {
          startVar += "<var name='";
          startVar += varName.data();
          startVar += "'>";
          endVar = "</var>";
        }
        state = 1;
        break;
      case 1:
        return startVar + start + varValue + end + endVar;
    }
  }
  return startVar + start + varValue + end + endVar;
}

//////////////////////////////////////////////////////////////////////////////
// helpers

void find_var_recursive(const TypedValue* tv, WddxPacket* wddxPacket) {
  int state = 0;
  while (state >= 0) {
    switch (state) {
      case 0:
        if (tvIsString(tv)) {
          String var_name = tvCastToString(tv);
          wddxPacket->add_var(var_name, true);
        }
        state = 1;
        break;
      case 1:
        if (tv->m_type == KindOfArray) {
          for (ArrayIter iter(tv->m_data.parr); iter; ++iter) {
            find_var_recursive(iter.secondRef().asTypedValue(), wddxPacket);
          }
        }
        state = -1;
        break;
    }
  }
}

static TypedValue* add_vars_helper(ActRec* ar) {
  int start_index = 1;
  Resource packet_id = getArg<KindOfResource>(ar, 0);
  auto wddxPacket = packet_id.getTyped<WddxPacket>();
  int state = 0;
  while (state >= 0) {
    switch (state) {
      case 0:
        for (int i = start_index; i < ar->numArgs(); i++) {
          auto const tv = getArg(ar, i);
          find_var_recursive(tv, wddxPacket);
        }
        state = 1;
        break;
      case 1:
        return arReturn(ar, true);
    }
  }
  return arReturn(ar, true);
}

static TypedValue* serialize_vars_helper(ActRec* ar) {
  WddxPacket* wddxPacket = newres<WddxPacket>(empty_string_variant_ref,
                                              true, true);
  int start_index = 0;
  int state = 0;
  while (state >= 0) {
    switch (state) {
      case 0:
        for (int i = start_index; i < ar->numArgs(); i++) {
          auto const tv = getArg(ar, i);
          find_var_recursive(tv, wddxPacket);
        }
        state = 1;
        break;
      case 1:
        Variant packet = wddxPacket->packet_end();
        return arReturn(ar, std::move(packet));
    }
  }
  Variant packet = wddxPacket->packet_end();
  return arReturn(ar, std::move(packet));
}

//////////////////////////////////////////////////////////////////////////////
// functions

static TypedValue* HHVM_FN(wddx_add_vars)(ActRec* ar) {
  return add_vars_helper(ar);
}

static TypedValue* HHVM_FN(wddx_serialize_vars)(ActRec* ar) {
  return serialize_vars_helper(ar);
}

static String HHVM_FUNCTION(wddx_packet_end, const Resource& packet_id) {
  auto wddxPacket = packet_id.getTyped<WddxPacket>();
  std::string packetString = wddxPacket->packet_end();
  return String(packetString);
}

static Resource HHVM_FUNCTION(wddx_packet_start, const Variant& comment) {
  auto wddxPacket = newres<WddxPacket>(comment, true, false);
  return Resource(wddxPacket);
}

static String HHVM_FUNCTION(wddx_serialize_value, const Variant& var,
                            const Variant& comment) {
  WddxPacket* wddxPacket = newres<WddxPacket>(comment, false, false);
  wddxPacket->serialize_value(var);
  const std::string packetString = wddxPacket->packet_end();
  return String(packetString);
}

//////////////////////////////////////////////////////////////////////////////

class wddxExtension : public Extension {
 public:
  wddxExtension() : Extension("wddx") {}
  virtual void moduleInit() {
    HHVM_FE(wddx_add_vars);
    HHVM_FE(wddx_packet_end);
    HHVM_FE(wddx_packet_start);
    HHVM_FE(wddx_serialize_value);
    HHVM_FE(wddx_serialize_vars);
    loadSystemlib();
  }
} s_wddx_extension;

// Uncomment for non-bundled module
//HHVM_GET_MODULE(wddx);

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP