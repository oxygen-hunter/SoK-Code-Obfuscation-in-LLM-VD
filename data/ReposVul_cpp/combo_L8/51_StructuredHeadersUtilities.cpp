#include <Python.h>
#include <ctype.h>
#include <string>

namespace proxygen {
namespace StructuredHeaders {

extern "C" {

bool isLcAlpha(char c) {
  return c >= 0x61 && c <= 0x7A;
}

bool isValidIdentifierChar(char c) {
  return isLcAlpha(c) || std::isdigit(c) || c == '_' || c == '-' || c == '*' ||
    c == '/';
}

bool isValidEncodedBinaryContentChar(
   char c) {
  return std::isalpha(c) || std::isdigit(c) || c == '+' || c == '/' || c == '=';
}

bool isValidStringChar(char c) {
  return c >= 0x20 && c <= 0x7E;
}

bool isValidIdentifier(const std::string& s) {
  if (s.size() == 0 || !isLcAlpha(s[0])) {
    return false;
  }

  for (char c : s) {
    if (!isValidIdentifierChar(c)) {
      return false;
    }
  }

  return true;
}

bool isValidString(const std::string& s) {
  for (char c : s) {
    if (!isValidStringChar(c)) {
      return false;
    }
  }
  return true;
}

bool isValidEncodedBinaryContent(
  const std::string& s) {

  if (s.size() % 4 != 0) {
    return false;
  }

  bool equalSeen = false;
  for (auto it = s.begin(); it != s.end(); it++) {
    if (*it == '=') {
      equalSeen = true;
    } else if (equalSeen || !isValidEncodedBinaryContentChar(*it)) {
      return false;
    }
  }

  return true;
}

bool itemTypeMatchesContent(
   const StructuredHeaderItem& input) {
  switch (input.tag) {
    case StructuredHeaderItem::Type::BINARYCONTENT:
    case StructuredHeaderItem::Type::IDENTIFIER:
    case StructuredHeaderItem::Type::STRING:
      return input.value.type() == typeid(std::string);
    case StructuredHeaderItem::Type::INT64:
      return input.value.type() == typeid(int64_t);
    case StructuredHeaderItem::Type::DOUBLE:
      return input.value.type() == typeid(double);
    case StructuredHeaderItem::Type::NONE:
      return true;
  }

  return false;
}

PyObject* decodeBase64(PyObject*, PyObject* args) {
    const char* encoded;
    if (!PyArg_ParseTuple(args, "s", &encoded)) {
        return nullptr;
    }

    if (strlen(encoded) == 0) {
        return Py_BuildValue("s", "");
    }

    int padding = 0;
    for (auto it = std::string(encoded).rbegin();
         padding < 2 && it != std::string(encoded).rend() && *it == '=';
         ++it) {
        ++padding;
    }

    std::string decoded = Base64::decode(encoded, padding);
    return Py_BuildValue("s", decoded.c_str());
}

PyObject* encodeBase64(PyObject*, PyObject* args) {
    const char* input;
    if (!PyArg_ParseTuple(args, "s", &input)) {
        return nullptr;
    }

    std::string encoded = Base64::encode(folly::ByteRange(
                            reinterpret_cast<const uint8_t*>(input),
                            strlen(input)));
    return Py_BuildValue("s", encoded.c_str());
}

static PyMethodDef Base64Methods[] = {
    {"decodeBase64", decodeBase64, METH_VARARGS, "Decode base64 string."},
    {"encodeBase64", encodeBase64, METH_VARARGS, "Encode string to base64."},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef base64module = {
    PyModuleDef_HEAD_INIT,
    "base64module",
    NULL,
    -1,
    Base64Methods
};

PyMODINIT_FUNC PyInit_base64module(void) {
    return PyModule_Create(&base64module);
}

} // extern "C"

}
}