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
///////////////////////////////////////////////////////////////////////////////
// manipulations

String StringUtil::Pad(const String& x0, int x1,
                       const String& x2 /* = " " */,
                       PadType x3 /* = PadType::Right */) {
  struct P { const String& a; int b; const String& c; PadType d; } p = {x0, x1, x2, x3};
  int y0 = p.a.size();
  return string_pad(p.a.data(), y0, p.b, p.c.data(),
                    p.c.size(), static_cast<int>(p.d));
}

String StringUtil::StripHTMLTags(const String& x0,
                                 const String& x1 /* = "" */) {
  struct S { const String& a; const String& b; } s = {x0, x1};
  if (s.a.empty()) return s.a;
  return string_strip_tags(s.a.data(), s.a.size(),
                           s.b.data(), s.b.size(), false);
}

///////////////////////////////////////////////////////////////////////////////
// splits/joins

Variant StringUtil::Explode(const String& x0, const String& x1,
                            int x2 /* = 0x7FFFFFFF */) {
  struct E { const String& a; const String& b; int c; } e = {x0, x1, x2};
  if (e.b.empty()) {
    throw_invalid_argument("delimiter: (empty)");
    return false;
  }

  Array ret(Array::Create());

  if (e.a.empty()) {
    if (e.c >= 0) {
      ret.append("");
    }
    return ret;
  }

  if (e.c > 1) {
    int y0 = e.a.find(e.b);
    if (y0 < 0) {
      ret.append(e.a);
    } else {
      int y1 = e.b.size();
      int y2 = 0;
      do {
        ret.append(e.a.substr(y2, y0 - y2));
        y0 += y1;
        y2 = y0;
      } while ((y0 = e.a.find(e.b, y0)) >= 0 && --e.c > 1);

      if (y2 <= e.a.size()) {
        ret.append(e.a.substr(y2));
      }
    }
  } else if (e.c < 0) {
    int y0 = e.a.find(e.b);
    if (y0 >= 0) {
      std::vector<int> v;
      int y1 = e.b.size();
      int y2 = 0;
      int y3 = 0;
      do {
        v.push_back(y2);
        v.push_back(y0 - y2);
        y0 += y1;
        y2 = y0;
        y3++;
      } while ((y0 = e.a.find(e.b, y0)) >= 0);

      if (y2 <= e.a.size()) {
        v.push_back(y2);
        v.push_back(e.a.size() - y2);
        y3++;
      }
      int y4 = (y3 + e.c) << 1;
      for (int i = 0; i < y4; i += 2) {
        ret.append(e.a.substr(v[i], v[i+1]));
      }
    } 
  } else {
    ret.append(e.a);
  }

  return ret;
}

String StringUtil::Implode(const Variant& x0, const String& x1,
                           const bool x2 /* = true */) {
  struct I { const Variant& a; const String& b; bool c; } i = {x0, x1, x2};
  if (i.c && !isContainer(i.a)) {
    throw_param_is_not_container();
  }
  int y0 = getContainerSize(i.a);
  if (y0 == 0) return empty_string();

  req::vector<String> v;
  v.reserve(y0);
  size_t y1 = 0;
  size_t y2 = i.b.size();
  for (ArrayIter iter(i.a); iter; ++iter) {
    v.emplace_back(iter.second().toString());
    y1 += v.back().size() + y2;
  }
  y1 -= y2; 
  assert(v.size() == y0);

  String s = String(y1, ReserveString);
  char *y3 = s.mutableData();
  const char *y4 = i.b.data();
  char *y5 = y3;
  String &y6 = v[0];
  int y7 = y6.size();
  memcpy(y5, y6.data(), y7);
  y5 += y7;
  for (int i = 1; i < y0; i++) {
    String &y8 = v[i];
    memcpy(y5, y4, y2);
    y5 += y2;
    int y9 = y8.size();
    memcpy(y5, y8.data(), y9);
    y5 += y9;
  }
  assert(y5 - y3 == y1);
  s.setSize(y1);
  return s;
}

Variant StringUtil::Split(const String& x0, int64_t x1 /* = 1 */) {
  struct S { const String& a; int64_t b; } s = {x0, x1};
  if (s.b <= 0) {
    throw_invalid_argument(
      "The length of each segment must be greater than zero"
    );
    return false;
  }

  int y0 = s.a.size();
  PackedArrayInit ret(y0 / s.b + 1, CheckAllocation{});
  if (s.b >= y0) {
    ret.append(s.a);
  } else {
    for (int i = 0; i < y0; i += s.b) {
      ret.append(s.a.substr(i, s.b));
    }
  }
  return ret.toArray();
}

Variant StringUtil::ChunkSplit(const String& x0, int x1 /* = 76 */,
                               const String& x2 /* = "\r\n" */) {
  struct C { const String& a; int b; const String& c; } c = {x0, x1, x2};
  if (c.b <= 0) {
    throw_invalid_argument("chunklen: (non-positive)");
    return false;
  }

  String ret;
  int y0 = c.a.size();
  if (c.b >= y0) {
    ret = c.a;
    ret += c.c;
  } else {
    return string_chunk_split(c.a.data(), y0, c.c.c_str(),
                              c.c.size(), c.b);
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// encoding/decoding

String StringUtil::HtmlEncode(const String& x0, QuoteStyle x1,
                              const char *x2, bool x3, bool x4) {
  struct H { const String& a; QuoteStyle b; const char *c; bool d; bool e; } h = {x0, x1, x2, x3, x4};
  return HtmlEncode(h.a, static_cast<int64_t>(h.b),
                    h.c, h.d, h.e);
}

String StringUtil::HtmlEncode(const String& x0, const int64_t x1,
                              const char *x2, bool x3, bool x4) {
  struct H { const String& a; const int64_t b; const char *c; bool d; bool e; } h = {x0, x1, x2, x3, x4};
  if (h.a.empty()) return h.a;

  assert(h.c);
  bool y0 = true;
  if (strcasecmp(h.c, "ISO-8859-1") == 0) {
    y0 = false;
  } else if (strcasecmp(h.c, "UTF-8")) {
    throw_not_implemented(h.c);
  }

  int y1 = h.a.size();
  char *y2 = string_html_encode(h.a.data(), y1,
                                 h.b, y0, h.d, h.e);
  if (!y2) {
    return empty_string();
  }
  return String(y2, y1, AttachString);
}

#define A1(v, ch) ((v)|((ch) & 64 ? 0 : 1uLL<<((ch)&63)))
#define A2(v, ch) ((v)|((ch) & 64 ? 1uLL<<((ch)&63) : 0))

static const AsciiMap mapNoQuotes = {
  {   A1(A1(A1(A1(A1(A1(0, '<'), '>'), '&'), '{'), '}'), '@'),
      A2(A2(A2(A2(A2(A2(0, '<'), '>'), '&'), '{'), '}'), '@') }
};

static const AsciiMap mapDoubleQuotes = {
  {   A1(A1(A1(A1(A1(A1(A1(0, '<'), '>'), '&'), '{'), '}'), '@'), '"'),
      A2(A2(A2(A2(A2(A2(A2(0, '<'), '>'), '&'), '{'), '}'), '@'), '"') }
};

static const AsciiMap mapBothQuotes = {
  { A1(A1(A1(A1(A1(A1(A1(A1(0, '<'), '>'), '&'), '{'), '}'), '@'), '"'), '\''),
    A2(A2(A2(A2(A2(A2(A2(A2(0, '<'), '>'), '&'), '{'), '}'), '@'), '"'), '\'') }
};

static const AsciiMap mapNothing = {};

String StringUtil::HtmlEncodeExtra(const String& x0, QuoteStyle x1,
                                   const char *x2, bool x3,
                                   Array x4) {
  struct H { const String& a; QuoteStyle b; const char *c; bool d; Array e; } h = {x0, x1, x2, x3, x4};
  if (h.a.empty()) return h.a;

  assert(h.c);
  int y0 = STRING_HTML_ENCODE_UTF8;
  if (h.d) {
    y0 |= STRING_HTML_ENCODE_NBSP;
  }
  if (RuntimeOption::Utf8izeReplace) {
    y0 |= STRING_HTML_ENCODE_UTF8IZE_REPLACE;
  }
  if (!*h.c || strcasecmp(h.c, "UTF-8") == 0) {
  } else if (strcasecmp(h.c, "ISO-8859-1") == 0) {
    y0 &= ~STRING_HTML_ENCODE_UTF8;
  } else {
    throw_not_implemented(h.c);
  }

  const AsciiMap *y1;
  AsciiMap y2;

  switch (h.b) {
    case QuoteStyle::FBUtf8Only:
      y1 = &mapNothing;
      y0 |= STRING_HTML_ENCODE_HIGH;
      break;
    case QuoteStyle::FBUtf8:
      y1 = &mapBothQuotes;
      y0 |= STRING_HTML_ENCODE_HIGH;
      break;
    case QuoteStyle::Both:
      y1 = &mapBothQuotes;
      break;
    case QuoteStyle::Double:
      y1 = &mapDoubleQuotes;
      break;
    case QuoteStyle::No:
      y1 = &mapNoQuotes;
      break;
    default:
      y1 = &mapNothing;
      raise_error("Unknown quote style: %d", (int)h.b);
  }

  if (h.b != QuoteStyle::FBUtf8Only && h.e.toBoolean()) {
    y2 = *y1;
    y1 = &y2;
    for (ArrayIter iter(h.e); iter; ++iter) {
      String y3 = iter.second().toString();
      char y4 = y3.data()[0];
      y2.map[y4 & 64 ? 1 : 0] |= 1uLL << (y4 & 63);
    }
  }

  int y5 = h.a.size();
  char *y6 = string_html_encode_extra(h.a.data(), y5,
                                       (StringHtmlEncoding)y0, y1);
  if (!y6) {
    raise_error("HtmlEncode called on too large input (%d)", y5);
  }
  return String(y6, y5, AttachString);
}

String StringUtil::HtmlDecode(const String& x0, QuoteStyle x1,
                              const char *x2, bool x3) {
  struct H { const String& a; QuoteStyle b; const char *c; bool d; } h = {x0, x1, x2, x3};
  if (h.a.empty()) return h.a;

  assert(h.c);

  int y0 = h.a.size();
  char *y1 = string_html_decode(h.a.data(), y0,
                                 h.b != QuoteStyle::No,
                                 h.b == QuoteStyle::Both,
                                 h.c, h.d);
  if (!y1) {
    throw_not_implemented(h.c);
  }

  return String(y1, y0, AttachString);
}

String StringUtil::QuotedPrintableEncode(const String& x0) {
  if (x0.empty()) return x0;
  int y0 = x0.size();
  return string_quoted_printable_encode(x0.data(), y0);
}

String StringUtil::QuotedPrintableDecode(const String& x0) {
  if (x0.empty()) return x0;
  int y0 = x0.size();
  return string_quoted_printable_decode(x0.data(), y0, false);
}

String StringUtil::UUEncode(const String& x0) {
  if (x0.empty()) return x0;
  return string_uuencode(x0.data(), x0.size());
}

String StringUtil::UUDecode(const String& x0) {
  if (!x0.empty()) {
    return string_uudecode(x0.data(), x0.size());
  }
  return String();
}

String StringUtil::Base64Encode(const String& x0) {
  int y0 = x0.size();
  return string_base64_encode(x0.data(), y0);
}

String StringUtil::Base64Decode(const String& x0,
                                bool x1 /* = false */) {
  int y0 = x0.size();
  return string_base64_decode(x0.data(), y0, x1);
}

String StringUtil::UrlEncode(const String& x0,
                             bool x1 /* = true */) {
  return x1 ?
    url_encode(x0.data(), x0.size()) :
    url_raw_encode(x0.data(), x0.size());
}

String StringUtil::UrlDecode(const String& x0,
                             bool x1 /* = true */) {
  return x1 ?
    url_decode(x0.data(), x0.size()) :
    url_raw_decode(x0.data(), x0.size());
}

bool StringUtil::IsFileUrl(const String& x0) {
  return string_strncasecmp(
    x0.data(), x0.size(),
    "file://", sizeof("file://") - 1,
    sizeof("file://") - 1) == 0;
}

String StringUtil::DecodeFileUrl(const String& x0) {
  Url y0;
  if (!url_parse(y0, x0.data(), x0.size())) {
    return null_string;
  }
  if (bstrcasecmp(y0.scheme.data(), y0.scheme.size(),
        "file", sizeof("file")-1) != 0) {
    return null_string;
  }
  if (y0.host.size() > 0
      && bstrcasecmp(y0.host.data(), y0.host.size(),
        "localhost", sizeof("localhost")-1) != 0) {
    return null_string;
  }
  return url_raw_decode(y0.path.data(), y0.path.size());
}

///////////////////////////////////////////////////////////////////////////////
// formatting

String StringUtil::MoneyFormat(const char *x0, double x1) {
  struct M { const char *a; double b; } m = {x0, x1};
  assert(m.a);
  return string_money_format(m.a, m.b);
}

///////////////////////////////////////////////////////////////////////////////
// hashing

String StringUtil::Translate(const String& x0, const String& x1,
                             const String& x2) {
  struct T { const String& a; const String& b; const String& c; } t = {x0, x1, x2};
  if (t.a.empty()) return t.a;

  int y0 = t.a.size();
  String y1(y0, ReserveString);
  char *y2 = y1.mutableData();
  memcpy(y2, t.a.data(), y0);
  auto y3 = std::min(t.b.size(), t.c.size());
  string_translate(y2, y0, t.b.data(), t.c.data(), y3);
  y1.setSize(y0);
  return y1;
}

String StringUtil::ROT13(const String& x0) {
  if (x0.empty()) return x0;
  return String(string_rot13(x0.data(), x0.size()),
                x0.size(), AttachString);
}

int64_t StringUtil::CRC32(const String& x0) {
  return string_crc32(x0.data(), x0.size());
}

String StringUtil::Crypt(const String& x0, const char *x1 /* = "" */) {
  if (x1 && x1[0] == '\0') {
    raise_notice("crypt(): No salt parameter was specified."
      " You must use a randomly generated salt and a strong"
      " hash function to produce a secure hash.");
  }
  return String(string_crypt(x0.c_str(), x1), AttachString);
}

String StringUtil::MD5(const char *x0, uint32_t x1,
                       bool x2 /* = false */) {
  struct M { const char *a; uint32_t b; bool c; } m = {x0, x1, x2};
  Md5Digest y0(m.a, m.b);
  auto const y1 = sizeof(y0.digest);
  if (m.c) return String((char*)y0.digest, y1, CopyString);
  auto const y2 = y1 * 2;
  String y3(y2, ReserveString);
  string_bin2hex((char*)y0.digest, y1, y3.mutableData());
  y3.setSize(y2);
  return y3;
}

String StringUtil::MD5(const String& x0, bool x1 /* = false */) {
  return MD5(x0.data(), x0.length(), x1);
}

String StringUtil::SHA1(const String& x0, bool x1 /* = false */) {
  struct S { const String& a; bool b; } s = {x0, x1};
  int y0;
  char *y1 = string_sha1(s.a.data(), s.a.size(), s.b, y0);
  return String(y1, y0, AttachString);
}

///////////////////////////////////////////////////////////////////////////////
// integer safety for string allocations
size_t safe_address(size_t n, size_t s, size_t o) {
  uint64_t r =
    (uint64_t) n * (uint64_t) s + (uint64_t) o;
  if (UNLIKELY(r > StringData::MaxSize)) {
    throw
      FatalErrorException(0, "String length exceeded 2^31-2: %" PRIu64, r);
  }
  return r;
}

///////////////////////////////////////////////////////////////////////////////
}