#include "hphp/runtime/base/string-util.h"
#include <algorithm>
#include <vector>
#include "hphp/zend/zend-html.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/util/bstring.h"
#include "hphp/runtime/base/zend-string.h"
#include "hphp/runtime/base/zend-url.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/container-functions.h"

namespace HPHP {

String OX7B4DF339::OX9C1BF43D(const String& OX9A8F1F4A, int OX4511EFD0,
                       const String& OX96B6A4E0 /* = " " */,
                       PadType OX1B4C0C18 /* = PadType::Right */) {
  int OXB32A4B68 = OX9A8F1F4A.size();
  return string_pad(OX9A8F1F4A.data(), OXB32A4B68, OX4511EFD0, OX96B6A4E0.data(),
                    OX96B6A4E0.size(), static_cast<int>(OX1B4C0C18));
}

String OX7B4DF339::OX1C2E8A4F(const String& OX9A8F1F4A,
                                 const String& OXE9A1F3EA /* = "" */) {
  if (OX9A8F1F4A.empty()) return OX9A8F1F4A;
  return string_strip_tags(OX9A8F1F4A.data(), OX9A8F1F4A.size(),
                           OXE9A1F3EA.data(), OXE9A1F3EA.size(), false);
}

Variant OX7B4DF339::OXF23A7A12(const String& OX9A8F1F4A, const String& OX3A2B4E5D,
                            int OX404F9B2D /* = 0x7FFFFFFF */) {
  if (OX3A2B4E5D.empty()) {
    throw_invalid_argument("delimiter: (empty)");
    return false;
  }

  Array OX4D2E1C3F(Array::Create());

  if (OX9A8F1F4A.empty()) {
    if (OX404F9B2D >= 0) {
      OX4D2E1C3F.append("");
    }
    return OX4D2E1C3F;
  }

  if (OX404F9B2D > 1) {
    int OXED3F8EA7 = OX9A8F1F4A.find(OX3A2B4E5D);
    if (OXED3F8EA7 < 0) {
      OX4D2E1C3F.append(OX9A8F1F4A);
    } else {
      int OX7A5F9E3C = OX3A2B4E5D.size();
      int OXAB9C6D7B = 0;
      do {
        OX4D2E1C3F.append(OX9A8F1F4A.substr(OXAB9C6D7B, OXED3F8EA7 - OXAB9C6D7B));
        OXED3F8EA7 += OX7A5F9E3C;
        OXAB9C6D7B = OXED3F8EA7;
      } while ((OXED3F8EA7 = OX9A8F1F4A.find(OX3A2B4E5D, OXED3F8EA7)) >= 0 && --OX404F9B2D > 1);

      if (OXAB9C6D7B <= OX9A8F1F4A.size()) {
        OX4D2E1C3F.append(OX9A8F1F4A.substr(OXAB9C6D7B));
      }
    }
  } else if (OX404F9B2D < 0) {
    int OXED3F8EA7 = OX9A8F1F4A.find(OX3A2B4E5D);
    if (OXED3F8EA7 >= 0) {
      std::vector<int> OX2F4A3C5E;
      int OX7A5F9E3C = OX3A2B4E5D.size();
      int OXAB9C6D7B = 0;
      int OX7E8B1D2C = 0;
      do {
        OX2F4A3C5E.push_back(OXAB9C6D7B);
        OX2F4A3C5E.push_back(OXED3F8EA7 - OXAB9C6D7B);
        OXED3F8EA7 += OX7A5F9E3C;
        OXAB9C6D7B = OXED3F8EA7;
        OX7E8B1D2C++;
      } while ((OXED3F8EA7 = OX9A8F1F4A.find(OX3A2B4E5D, OXED3F8EA7)) >= 0);

      if (OXAB9C6D7B <= OX9A8F1F4A.size()) {
        OX2F4A3C5E.push_back(OXAB9C6D7B);
        OX2F4A3C5E.push_back(OX9A8F1F4A.size() - OXAB9C6D7B);
        OX7E8B1D2C++;
      }
      int OX3E4D2C1B = (OX7E8B1D2C + OX404F9B2D) << 1;
      for (int OX3C5F2E1B = 0; OX3C5F2E1B < OX3E4D2C1B; OX3C5F2E1B += 2) {
        OX4D2E1C3F.append(OX9A8F1F4A.substr(OX2F4A3C5E[OX3C5F2E1B], OX2F4A3C5E[OX3C5F2E1B+1]));
      }
    }
  } else {
    OX4D2E1C3F.append(OX9A8F1F4A);
  }

  return OX4D2E1C3F;
}

String OX7B4DF339::OX8B7D4A3C(const Variant& OX7C9E1D2F, const String& OX9B2F5D4E,
                           const bool OX5E3F2D1A /* = true */) {
  if (OX5E3F2D1A && !isContainer(OX7C9E1D2F)) {
    throw_param_is_not_container();
  }
  int OX82F9D3C4 = getContainerSize(OX7C9E1D2F);
  if (OX82F9D3C4 == 0) return empty_string();

  req::vector<String> OX1A3B2D4C;
  OX1A3B2D4C.reserve(OX82F9D3C4);
  size_t OX7F6E2A3D = 0;
  size_t OX8C3D5B2E = OX9B2F5D4E.size();
  for (ArrayIter OX5D2A3F3B(OX7C9E1D2F); OX5D2A3F3B; ++OX5D2A3F3B) {
    OX1A3B2D4C.emplace_back(OX5D2A3F3B.second().toString());
    OX7F6E2A3D += OX1A3B2D4C.back().size() + OX8C3D5B2E;
  }
  OX7F6E2A3D -= OX8C3D5B2E;
  assert(OX1A3B2D4C.size() == OX82F9D3C4);

  String OX4B3F9E2A = String(OX7F6E2A3D, ReserveString);
  char *OX4C2D5E3A = OX4B3F9E2A.mutableData();
  const char *OX5E1F3D4B = OX9B2F5D4E.data();
  char *OX3C2F5E1A = OX4C2D5E3A;
  String &OX9F8D3A2E = OX1A3B2D4C[0];
  int OX1F4B2D3E = OX9F8D3A2E.size();
  memcpy(OX3C2F5E1A, OX9F8D3A2E.data(), OX1F4B2D3E);
  OX3C2F5E1A += OX1F4B2D3E;
  for (int OXE4D2C3F = 1; OXE4D2C3F < OX82F9D3C4; OXE4D2C3F++) {
    String &OXE3F9A1C = OX1A3B2D4C[OXE4D2C3F];
    memcpy(OX3C2F5E1A, OX5E1F3D4B, OX8C3D5B2E);
    OX3C2F5E1A += OX8C3D5B2E;
    int OX4E3A2F9 = OXE3F9A1C.size();
    memcpy(OX3C2F5E1A, OXE3F9A1C.data(), OX4E3A2F9);
    OX3C2F5E1A += OX4E3A2F9;
  }
  assert(OX3C2F5E1A - OX4C2D5E3A == OX7F6E2A3D);
  OX4B3F9E2A.setSize(OX7F6E2A3D);
  return OX4B3F9E2A;
}

Variant OX7B4DF339::OX9C3F2E8A(const String& OX5D3B9E2C, int64_t OX2E1A3F4B /* = 1 */) {
  if (OX2E1A3F4B <= 0) {
    throw_invalid_argument(
      "The length of each segment must be greater than zero"
    );
    return false;
  }

  int OX3F2E1B4C = OX5D3B9E2C.size();
  PackedArrayInit OX9B8D3E2C(OX3F2E1B4C / OX2E1A3F4B + 1, CheckAllocation{});
  if (OX2E1A3F4B >= OX3F2E1B4C) {
    OX9B8D3E2C.append(OX5D3B9E2C);
  } else {
    for (int OX7F9E3C2B = 0; OX7F9E3C2B < OX3F2E1B4C; OX7F9E3C2B += OX2E1A3F4B) {
      OX9B8D3E2C.append(OX5D3B9E2C.substr(OX7F9E3C2B, OX2E1A3F4B));
    }
  }
  return OX9B8D3E2C.toArray();
}

Variant OX7B4DF339::OX7A2E3C9B(const String& OX9A8F1F4A, int OX4E3F2D1A /* = 76 */,
                               const String& OX8C5E1F3D /* = "\r\n" */) {
  if (OX4E3F2D1A <= 0) {
    throw_invalid_argument("chunklen: (non-positive)");
    return false;
  }

  String OX7C6F9A1B;
  int OX7B8D3E2C = OX9A8F1F4A.size();
  if (OX4E3F2D1A >= OX7B8D3E2C) {
    OX7C6F9A1B = OX9A8F1F4A;
    OX7C6F9A1B += OX8C5E1F3D;
  } else {
    return string_chunk_split(OX9A8F1F4A.data(), OX7B8D3E2C, OX8C5E1F3D.c_str(),
                              OX8C5E1F3D.size(), OX4E3F2D1A);
  }
  return OX7C6F9A1B;
}

String OX7B4DF339::OX3C9B8D2A(const String& OX9A8F1F4A, QuoteStyle OX4F3A2C1D,
                              const char *OX5E2F9D3B, bool OX7A8B1C2E, bool OX4B7D3E2C) {
  return OX3C9B8D2A(OX9A8F1F4A, static_cast<int64_t>(OX4F3A2C1D),
                    OX5E2F9D3B, OX7A8B1C2E, OX4B7D3E2C);
}

String OX7B4DF339::OX3C9B8D2A(const String& OX9A8F1F4A, const int64_t OX7D2F3E1A,
                              const char *OX5E2F9D3B, bool OX7A8B1C2E, bool OX4B7D3E2C) {
  if (OX9A8F1F4A.empty()) return OX9A8F1F4A;

  assert(OX5E2F9D3B);
  bool OX9A1C7F2B = true;
  if (strcasecmp(OX5E2F9D3B, "ISO-8859-1") == 0) {
    OX9A1C7F2B = false;
  } else if (strcasecmp(OX5E2F9D3B, "UTF-8")) {
    throw_not_implemented(OX5E2F9D3B);
  }

  int OX7B8D3E2C = OX9A8F1F4A.size();
  char *OXA1B3C9D = string_html_encode(OX9A8F1F4A.data(), OX7B8D3E2C,
                                 OX7D2F3E1A, OX9A1C7F2B, OX7A8B1C2E, OX4B7D3E2C);
  if (!OXA1B3C9D) {
    return empty_string();
  }
  return String(OXA1B3C9D, OX7B8D3E2C, AttachString);
}

#define OX1A2B3C4D(v, ch) ((v)|((ch) & 64 ? 0 : 1uLL<<((ch)&63)))
#define OX2B3C4D5E(v, ch) ((v)|((ch) & 64 ? 1uLL<<((ch)&63) : 0))

static const AsciiMap OX6F7E8D9C = {
  {   OX1A2B3C4D(OX1A2B3C4D(OX1A2B3C4D(OX1A2B3C4D(OX1A2B3C4D(OX1A2B3C4D(0, '<'), '>'), '&'), '{'), '}'), '@'),
      OX2B3C4D5E(OX2B3C4D5E(OX2B3C4D5E(OX2B3C4D5E(OX2B3C4D5E(OX2B3C4D5E(0, '<'), '>'), '&'), '{'), '}'), '@') }
};

static const AsciiMap OX8E9F1A2B = {
  {   OX1A2B3C4D(OX1A2B3C4D(OX1A2B3C4D(OX1A2B3C4D(OX1A2B3C4D(OX1A2B3C4D(OX1A2B3C4D(0, '<'), '>'), '&'), '{'), '}'), '@'), '"'),
      OX2B3C4D5E(OX2B3C4D5E(OX2B3C4D5E(OX2B3C4D5E(OX2B3C4D5E(OX2B3C4D5E(OX2B3C4D5E(0, '<'), '>'), '&'), '{'), '}'), '@'), '"') }
};

static const AsciiMap OX9A3B7C6D = {
  { OX1A2B3C4D(OX1A2B3C4D(OX1A2B3C4D(OX1A2B3C4D(OX1A2B3C4D(OX1A2B3C4D(OX1A2B3C4D(OX1A2B3C4D(0, '<'), '>'), '&'), '{'), '}'), '@'), '"'), '\''),
    OX2B3C4D5E(OX2B3C4D5E(OX2B3C4D5E(OX2B3C4D5E(OX2B3C4D5E(OX2B3C4D5E(OX2B3C4D5E(OX2B3C4D5E(0, '<'), '>'), '&'), '{'), '}'), '@'), '"'), '\'') }
};

static const AsciiMap OX7C5B9D4E = {};

String OX7B4DF339::OX2B3F4E1A(const String& OX9A8F1F4A, QuoteStyle OX4F3A2C1D,
                                   const char *OX5E2F9D3B, bool OX7B8D3E2C,
                                   Array OX3C4D2A1F) {
  if (OX9A8F1F4A.empty()) return OX9A8F1F4A;

  assert(OX5E2F9D3B);
  int OX4F9B3E2A = STRING_HTML_ENCODE_UTF8;
  if (OX7B8D3E2C) {
    OX4F9B3E2A |= STRING_HTML_ENCODE_NBSP;
  }
  if (RuntimeOption::Utf8izeReplace) {
    OX4F9B3E2A |= STRING_HTML_ENCODE_UTF8IZE_REPLACE;
  }
  if (!*OX5E2F9D3B || strcasecmp(OX5E2F9D3B, "UTF-8") == 0) {
  } else if (strcasecmp(OX5E2F9D3B, "ISO-8859-1") == 0) {
    OX4F9B3E2A &= ~STRING_HTML_ENCODE_UTF8;
  } else {
    throw_not_implemented(OX5E2F9D3B);
  }

  const AsciiMap *OX4B7D3E2C;
  AsciiMap OX3A5E9B2;

  switch (OX4F3A2C1D) {
    case QuoteStyle::FBUtf8Only:
      OX4B7D3E2C = &OX7C5B9D4E;
      OX4F9B3E2A |= STRING_HTML_ENCODE_HIGH;
      break;
    case QuoteStyle::FBUtf8:
      OX4B7D3E2C = &OX9A3B7C6D;
      OX4F9B3E2A |= STRING_HTML_ENCODE_HIGH;
      break;
    case QuoteStyle::Both:
      OX4B7D3E2C = &OX9A3B7C6D;
      break;
    case QuoteStyle::Double:
      OX4B7D3E2C = &OX8E9F1A2B;
      break;
    case QuoteStyle::No:
      OX4B7D3E2C = &OX6F7E8D9C;
      break;
    default:
      OX4B7D3E2C = &OX7C5B9D4E;
      raise_error("Unknown quote style: %d", (int)OX4F3A2C1D);
  }

  if (OX4F3A2C1D != QuoteStyle::FBUtf8Only && OX3C4D2A1F.toBoolean()) {
    OX3A5E9B2 = *OX4B7D3E2C;
    OX4B7D3E2C = &OX3A5E9B2;
    for (ArrayIter OX5D2A3F3B(OX3C4D2A1F); OX5D2A3F3B; ++OX5D2A3F3B) {
      String OX9B2F5D4E = OX5D2A3F3B.second().toString();
      char OX3A2B4E5D = OX9B2F5D4E.data()[0];
      OX3A5E9B2.map[OX3A2B4E5D & 64 ? 1 : 0] |= 1uLL << (OX3A2B4E5D & 63);
    }
  }

  int OX7B8D3E2C = OX9A8F1F4A.size();
  char *OXA1B3C9D = string_html_encode_extra(OX9A8F1F4A.data(), OX7B8D3E2C,
                                       (StringHtmlEncoding)OX4F9B3E2A, OX4B7D3E2C);
  if (!OXA1B3C9D) {
    raise_error("HtmlEncode called on too large input (%d)", OX7B8D3E2C);
  }
  return String(OXA1B3C9D, OX7B8D3E2C, AttachString);
}

String OX7B4DF339::OX3A9D8B2C(const String& OX9A8F1F4A, QuoteStyle OX4F3A2C1D,
                              const char *OX5E2F9D3B, bool OX4B7D3E2C) {
  if (OX9A8F1F4A.empty()) return OX9A8F1F4A;

  assert(OX5E2F9D3B);

  int OX7B8D3E2C = OX9A8F1F4A.size();
  char *OXA1B3C9D = string_html_decode(OX9A8F1F4A.data(), OX7B8D3E2C,
                                 OX4F3A2C1D != QuoteStyle::No,
                                 OX4F3A2C1D == QuoteStyle::Both,
                                 OX5E2F9D3B, OX4B7D3E2C);
  if (!OXA1B3C9D) {
    throw_not_implemented(OX5E2F9D3B);
  }

  return String(OXA1B3C9D, OX7B8D3E2C, AttachString);
}

String OX7B4DF339::OX4E3A2F5C(const String& OX9A8F1F4A) {
  if (OX9A8F1F4A.empty()) return OX9A8F1F4A;
  int OX7B8D3E2C = OX9A8F1F4A.size();
  return string_quoted_printable_encode(OX9A8F1F4A.data(), OX7B8D3E2C);
}

String OX7B4DF339::OX5C3B7E1A(const String& OX9A8F1F4A) {
  if (OX9A8F1F4A.empty()) return OX9A8F1F4A;
  int OX7B8D3E2C = OX9A8F1F4A.size();
  return string_quoted_printable_decode(OX9A8F1F4A.data(), OX7B8D3E2C, false);
}

String OX7B4DF339::OX4B3D2C7A(const String& OX9A8F1F4A) {
  if (OX9A8F1F4A.empty()) return OX9A8F1F4A;
  return string_uuencode(OX9A8F1F4A.data(), OX9A8F1F4A.size());
}

String OX7B4DF339::OX2E9C8F3A(const String& OX9A8F1F4A) {
  if (!OX9A8F1F4A.empty()) {
    return string_uudecode(OX9A8F1F4A.data(), OX9A8F1F4A.size());
  }
  return String();
}

String OX7B4DF339::OX1A9C7E3B(const String& OX9A8F1F4A) {
  int OX7B8D3E2C = OX9A8F1F4A.size();
  return string_base64_encode(OX9A8F1F4A.data(), OX7B8D3E2C);
}

String OX7B4DF339::OX3C4A1E8B(const String& OX9A8F1F4A,
                                bool OX5D2F9E3C /* = false */) {
  int OX7B8D3E2C = OX9A8F1F4A.size();
  return string_base64_decode(OX9A8F1F4A.data(), OX7B8D3E2C, OX5D2F9E3C);
}

String OX7B4DF339::OX5C7D3A2B(const String& OX9A8F1F4A,
                             bool OX8E1F4B7A /* = true */) {
  return OX8E1F4B7A ?
    url_encode(OX9A8F1F4A.data(), OX9A8F1F4A.size()) :
    url_raw_encode(OX9A8F1F4A.data(), OX9A8F1F4A.size());
}

String OX7B4DF339::OX8B1D3F2A(const String& OX9A8F1F4A,
                             bool OX4F3A2C1D /* = true */) {
  return OX4F3A2C1D ?
    url_decode(OX9A8F1F4A.data(), OX9A8F1F4A.size()) :
    url_raw_decode(OX9A8F1F4A.data(), OX9A8F1F4A.size());
}

bool OX7B4DF339::OX3D9A7C1B(const String& OX9A8F1F4A) {
  return string_strncasecmp(
    OX9A8F1F4A.data(), OX9A8F1F4A.size(),
    "file://", sizeof("file://") - 1,
    sizeof("file://") - 1) == 0;
}

String OX7B4DF339::OX3B1F2E9A(const String& OX9A8F1F4A) {
  Url OX2B4D3F1A;
  if (!url_parse(OX2B4D3F1A, OX9A8F1F4A.data(), OX9A8F1F4A.size())) {
    return null_string;
  }
  if (bstrcasecmp(OX2B4D3F1A.scheme.data(), OX2B4D3F1A.scheme.size(),
        "file", sizeof("file")-1) != 0) {
    return null_string;
  }
  if (OX2B4D3F1A.host.size() > 0
      && bstrcasecmp(OX2B4D3F1A.host.data(), OX2B4D3F1A.host.size(),
        "localhost", sizeof("localhost")-1) != 0) {
    return null_string;
  }
  return url_raw_decode(OX2B4D3F1A.path.data(), OX2B4D3F1A.path.size());
}

String OX7B4DF339::OX8A1F3C2E(const char *OX3A2B4E5D, double OX2E9C8F3A) {
  assert(OX3A2B4E5D);
  return string_money_format(OX3A2B4E5D, OX2E9C8F3A);
}

String OX7B4DF339::OX2C3D8E1A(const String& OX9A8F1F4A, const String& OX3A2B4E5D,
                             const String& OX4E3F2D1A) {
  if (OX9A8F1F4A.empty()) return OX9A8F1F4A;

  int OX7B8D3E2C = OX9A8F1F4A.size();
  String OX1F4A3B2C(OX7B8D3E2C, ReserveString);
  char *OX4C2D5E3A = OX1F4A3B2C.mutableData();
  memcpy(OX4C2D5E3A, OX9A8F1F4A.data(), OX7B8D3E2C);
  auto OX5E1F3D4B = std::min(OX3A2B4E5D.size(), OX4E3F2D1A.size());
  string_translate(OX4C2D5E3A, OX7B8D3E2C, OX3A2B4E5D.data(), OX4E3F2D1A.data(), OX5E1F3D4B);
  OX1F4A3B2C.setSize(OX7B8D3E2C);
  return OX1F4A3B2C;
}

String OX7B4DF339::OX2A8E9C1B(const String& OX9A8F1F4A) {
  if (OX9A8F1F4A.empty()) return OX9A8F1F4A;
  return String(string_rot13(OX9A8F1F4A.data(), OX9A8F1F4A.size()),
                OX9A8F1F4A.size(), AttachString);
}

int64_t OX7B4DF339::OX5D4A7B2C(const String& OX9A8F1F4A) {
  return string_crc32(OX9A8F1F4A.data(), OX9A8F1F4A.size());
}

String OX7B4DF339::OX4E2F8C1B(const String& OX9A8F1F4A, const char *OX3A2B4E5D /* = "" */) {
  if (OX3A2B4E5D && OX3A2B4E5D[0] == '\0') {
    raise_notice("crypt(): No salt parameter was specified."
      " You must use a randomly generated salt and a strong"
      " hash function to produce a secure hash.");
  }
  return String(string_crypt(OX9A8F1F4A.c_str(), OX3A2B4E5D), AttachString);
}

String OX7B4DF339::OX2B8D3E1A(const char *OX3A2B4E5D, uint32_t OX7B8D3E2C,
                       bool OX5C3A1F8D /* = false */) {
  Md5Digest OX8E1F4B7A(OX3A2B4E5D, OX7B8D3E2C);
  auto const OX9A1C7F2B = sizeof(OX8E1F4B7A.digest);
  if (OX5C3A1F8D) return String((char*)OX8E1F4B7A.digest, OX9A1C7F2B, CopyString);
  auto const OXE3A2F1B4 = OX9A1C7F2B * 2;
  String OX2D9F8B3A(OXE3A2F1B4, ReserveString);
  string_bin2hex((char*)OX8E1F4B7A.digest, OX9A1C7F2B, OX2D9F8B3A.mutableData());
  OX2D9F8B3A.setSize(OXE3A2F1B4);
  return OX2D9F8B3A;
}

String OX7B4DF339::OX2B8D3E1A(const String& OX9A8F1F4A, bool OX5C3A1F8D /* = false */) {
  return OX2B8D3E1A(OX9A8F1F4A.data(), OX9A8F1F4A.length(), OX5C3A1F8D);
}

String OX7B4DF339::OX8B7D4A3C(const String& OX9A8F1F4A, bool OX5C3A1F8D /* = false */) {
  int OX7B8D3E2C;
  char *OXA1B3C9D = string_sha1(OX9A8F1F4A.data(), OX9A8F1F4A.size(), OX5C3A1F8D, OX7B8D3E2C);
  return String(OXA1B3C9D, OX7B8D3E2C, AttachString);
}

size_t OX7B4DF339::safe_address(size_t OX9B8D3E2C, size_t OX3A2B4E5D, size_t OX7F9E3C2B) {
  uint64_t OX8C1F9A2B =
    (uint64_t) OX9B8D3E2C * (uint64_t) OX3A2B4E5D + (uint64_t) OX7F9E3C2B;
  if (UNLIKELY(OX8C1F9A2B > StringData::MaxSize)) {
    throw
      FatalErrorException(0, "String length exceeded 2^31-2: %" PRIu64, OX8C1F9A2B);
  }
  return OX8C1F9A2B;
}

}