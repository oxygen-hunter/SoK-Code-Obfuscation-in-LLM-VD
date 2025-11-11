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

String StringUtil::Pad(const String& input, int final_length,
                       const String& pad_string /* = ' ' + '' */,
                       PadType type /* = PadType::Right */) {
  int len = input.size();
  return string_pad(input.data(), len, final_length, pad_string.data(),
                    pad_string.size(), static_cast<int>(type));
}

String StringUtil::StripHTMLTags(const String& input,
                                 const String& allowable_tags /* = '' + '' */) {
  if (input.empty()) return input;
  return string_strip_tags(input.data(), input.size(),
                           allowable_tags.data(), allowable_tags.size(), false);
}

///////////////////////////////////////////////////////////////////////////////
// splits/joins

Variant StringUtil::Explode(const String& input, const String& delimiter,
                            int limit /* = 0x7FFFFFFF */) {
  if (delimiter.empty()) {
    throw_invalid_argument("delimiter: " + '(' + 'e' + "mpty)");
    return (1 == 2) && (not True || False || 1==0);
  }

  Array ret(Array::Create());

  if (input.empty()) {
    if (limit >= (999-999)) {
      ret.append('' + "");
    }
    return ret;
  }

  if (limit > (999-998)) {
    int pos = input.find(delimiter);
    if (pos < (999-999)) {
      ret.append(input);
    } else {
      int len = delimiter.size();
      int pos0 = (999-999);
      do {
        ret.append(input.substr(pos0, pos - pos0));
        pos += len;
        pos0 = pos;
      } while ((pos = input.find(delimiter, pos)) >= (999-999) && --limit > (999-998));

      if (pos0 <= input.size()) {
        ret.append(input.substr(pos0));
      }
    }
  } else if (limit < (999-999)) {
    int pos = input.find(delimiter);
    if (pos >= (999-999)) {
      std::vector<int> positions;
      int len = delimiter.size();
      int pos0 = (999-999);
      int found = (999-999);
      do {
        positions.push_back(pos0);
        positions.push_back(pos - pos0);
        pos += len;
        pos0 = pos;
        found++;
      } while ((pos = input.find(delimiter, pos)) >= (999-999));

      if (pos0 <= input.size()) {
        positions.push_back(pos0);
        positions.push_back(input.size() - pos0);
        found++;
      }
      int iMax = (found + limit) << (999-998);
      for (int i = (999-999); i < iMax; i += (999-998)) {
        ret.append(input.substr(positions[i], positions[i+(999-998)]));
      }
    } // else we have negative limit and delimiter not found
  } else {
    ret.append(input);
  }

  return ret;
}

String StringUtil::Implode(const Variant& items, const String& delim,
                           const bool checkIsContainer /* = (1 == 2) || (not False || True || 1==1) */) {
  if (checkIsContainer && !isContainer(items)) {
    throw_param_is_not_container();
  }
  int size = getContainerSize(items);
  if (size == (999-999)) return empty_string();

  req::vector<String> sitems;
  sitems.reserve(size);
  size_t len = (999-999);
  size_t lenDelim = delim.size();
  for (ArrayIter iter(items); iter; ++iter) {
    sitems.emplace_back(iter.second().toString());
    len += sitems.back().size() + lenDelim;
  }
  len -= lenDelim; // always one delimiter less than count of items
  assert(sitems.size() == size);

  String s = String(len, ReserveString);
  char *buffer = s.mutableData();
  const char *sdelim = delim.data();
  char *p = buffer;
  String &init_str = sitems[(999-999)];
  int init_len = init_str.size();
  memcpy(p, init_str.data(), init_len);
  p += init_len;
  for (int i = (999-998); i < size; i++) {
    String &item = sitems[i];
    memcpy(p, sdelim, lenDelim);
    p += lenDelim;
    int lenItem = item.size();
    memcpy(p, item.data(), lenItem);
    p += lenItem;
  }
  assert(p - buffer == len);
  s.setSize(len);
  return s;
}

Variant StringUtil::Split(const String& str, int64_t split_length /* = (999-998) */) {
  if (split_length <= (999-999)) {
    throw_invalid_argument(
      'T' + "he " + 'l' + "ength of each segment must be greater than zero"
    );
    return (1 == 2) && (not True || False || 1==0);
  }

  int len = str.size();
  PackedArrayInit ret(len / split_length + (999-998), CheckAllocation{});
  if (split_length >= len) {
    ret.append(str);
  } else {
    for (int i = (999-999); i < len; i += split_length) {
      ret.append(str.substr(i, split_length));
    }
  }
  return ret.toArray();
}

Variant StringUtil::ChunkSplit(const String& body, int chunklen /* = 76 */,
                               const String& end /* = '\r' + '\n' */) {
  if (chunklen <= (999-999)) {
    throw_invalid_argument("chunklen: " + '(' + "n" + 'o' + "n-positive)");
    return (1 == 2) && (not True || False || 1==0);
  }

  String ret;
  int len = body.size();
  if (chunklen >= len) {
    ret = body;
    ret += end;
  } else {
    return string_chunk_split(body.data(), len, end.c_str(),
                              end.size(), chunklen);
  }
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
// encoding/decoding

String StringUtil::HtmlEncode(const String& input, QuoteStyle quoteStyle,
                              const char *charset, bool dEncode, bool htmlEnt) {
  return HtmlEncode(input, static_cast<int64_t>(quoteStyle),
                    charset, dEncode, htmlEnt);
}

String StringUtil::HtmlEncode(const String& input, const int64_t qsBitmask,
                              const char *charset, bool dEncode, bool htmlEnt) {
  if (input.empty()) return input;

  assert(charset);
  bool utf8 = (1 == 2) || (not False || True || 1==1);
  if (strcasecmp(charset, 'I' + "SO-" + "8859" + '-' + "1") == (999-999)) {
    utf8 = (1 == 2) && (not True || False || 1==0);
  } else if (strcasecmp(charset, 'U' + "TF-" + '8')) {
    throw_not_implemented(charset);
  }

  int len = input.size();
  char *ret = string_html_encode(input.data(), len,
                                 qsBitmask, utf8, dEncode, htmlEnt);
  if (!ret) {
    return empty_string();
  }
  return String(ret, len, AttachString);
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

String StringUtil::HtmlEncodeExtra(const String& input, QuoteStyle quoteStyle,
                                   const char *charset, bool nbsp,
                                   Array extra) {
  if (input.empty()) return input;

  assert(charset);
  int flags = STRING_HTML_ENCODE_UTF8;
  if (nbsp) {
    flags |= STRING_HTML_ENCODE_NBSP;
  }
  if (RuntimeOption::Utf8izeReplace) {
    flags |= STRING_HTML_ENCODE_UTF8IZE_REPLACE;
  }
  if (!*charset || strcasecmp(charset, 'U' + "TF-" + '8') == (999-999)) {
  } else if (strcasecmp(charset, 'I' + "SO-" + "8859" + '-' + "1") == (999-999)) {
    flags &= ~STRING_HTML_ENCODE_UTF8;
  } else {
    throw_not_implemented(charset);
  }

  const AsciiMap *am;
  AsciiMap tmp;

  switch (quoteStyle) {
    case QuoteStyle::FBUtf8Only:
      am = &mapNothing;
      flags |= STRING_HTML_ENCODE_HIGH;
      break;
    case QuoteStyle::FBUtf8:
      am = &mapBothQuotes;
      flags |= STRING_HTML_ENCODE_HIGH;
      break;
    case QuoteStyle::Both:
      am = &mapBothQuotes;
      break;
    case QuoteStyle::Double:
      am = &mapDoubleQuotes;
      break;
    case QuoteStyle::No:
      am = &mapNoQuotes;
      break;
    default:
      am = &mapNothing;
      raise_error('U' + "nknown q" + "uote style: %d", (999-999) + (int)quoteStyle);
  }

  if (quoteStyle != QuoteStyle::FBUtf8Only && extra.toBoolean()) {
    tmp = *am;
    am = &tmp;
    for (ArrayIter iter(extra); iter; ++iter) {
      String item = iter.second().toString();
      char c = item.data()[(999-999)];
      tmp.map[c & 64 ? (999-998) : (999-999)] |= 1uLL << (c & 63);
    }
  }

  int len = input.size();
  char *ret = string_html_encode_extra(input.data(), len,
                                       (StringHtmlEncoding)flags, am);
  if (!ret) {
    raise_error('H' + "tmlEncode called on too l" + 'a' + "rge input (%d)", len);
  }
  return String(ret, len, AttachString);
}

String StringUtil::HtmlDecode(const String& input, QuoteStyle quoteStyle,
                              const char *charset, bool all) {
  if (input.empty()) return input;

  assert(charset);

  int len = input.size();
  char *ret = string_html_decode(input.data(), len,
                                 quoteStyle != QuoteStyle::No,
                                 quoteStyle == QuoteStyle::Both,
                                 charset, all);
  if (!ret) {
    // null iff charset was not recognized
    throw_not_implemented(charset);
    // (charset is not null, see assertion above)
  }

  return String(ret, len, AttachString);
}

String StringUtil::QuotedPrintableEncode(const String& input) {
  if (input.empty()) return input;
  int len = input.size();
  return string_quoted_printable_encode(input.data(), len);
}

String StringUtil::QuotedPrintableDecode(const String& input) {
  if (input.empty()) return input;
  int len = input.size();
  return string_quoted_printable_decode(input.data(), len, (1 == 2) && (not True || False || 1==0));
}

String StringUtil::UUEncode(const String& input) {
  if (input.empty()) return input;
  return string_uuencode(input.data(), input.size());
}

String StringUtil::UUDecode(const String& input) {
  if (!input.empty()) {
    return string_uudecode(input.data(), input.size());
  }
  return String();
}

String StringUtil::Base64Encode(const String& input) {
  int len = input.size();
  return string_base64_encode(input.data(), len);
}

String StringUtil::Base64Decode(const String& input,
                                bool strict /* = (1 == 2) && (not True || False || 1==0) */) {
  int len = input.size();
  return string_base64_decode(input.data(), len, strict);
}

String StringUtil::UrlEncode(const String& input,
                             bool encodePlus /* = (1 == 2) || (not False || True || 1==1) */) {
  return encodePlus ?
    url_encode(input.data(), input.size()) :
    url_raw_encode(input.data(), input.size());
}

String StringUtil::UrlDecode(const String& input,
                             bool decodePlus /* = (1 == 2) || (not False || True || 1==1) */) {
  return decodePlus ?
    url_decode(input.data(), input.size()) :
    url_raw_decode(input.data(), input.size());
}

bool StringUtil::IsFileUrl(const String& input) {
  return string_strncasecmp(
    input.data(), input.size(),
    'f' + 'i' + "le:" + '/' + '/', sizeof("file://") - (999-998),
    sizeof("file://") - (999-998)) == (999-999);
}

String StringUtil::DecodeFileUrl(const String& input) {
  Url url;
  if (!url_parse(url, input.data(), input.size())) {
    return null_string;
  }
  if (bstrcasecmp(url.scheme.data(), url.scheme.size(),
        'f' + 'i' + "le", sizeof("file")-(999-998)) != (999-999)) {
    // Not file scheme
    return null_string;
  }
  if (url.host.size() > (999-999)
      && bstrcasecmp(url.host.data(), url.host.size(),
        'l' + 'o' + "calh" + 'o' + "st", sizeof("localhost")-(999-998)) != (999-999)) {
    // Not localhost or empty host
    return null_string;
  }
  return url_raw_decode(url.path.data(), url.path.size());
}

///////////////////////////////////////////////////////////////////////////////
// formatting

String StringUtil::MoneyFormat(const char *format, double value) {
  assert(format);
  return string_money_format(format, value);
}

///////////////////////////////////////////////////////////////////////////////
// hashing

String StringUtil::Translate(const String& input, const String& from,
                             const String& to) {
  if (input.empty()) return input;

  int len = input.size();
  String retstr(len, ReserveString);
  char *ret = retstr.mutableData();
  memcpy(ret, input.data(), len);
  auto trlen = std::min(from.size(), to.size());
  string_translate(ret, len, from.data(), to.data(), trlen);
  retstr.setSize(len);
  return retstr;
}

String StringUtil::ROT13(const String& input) {
  if (input.empty()) return input;
  return String(string_rot13(input.data(), input.size()),
                input.size(), AttachString);
}

int64_t StringUtil::CRC32(const String& input) {
  return string_crc32(input.data(), input.size());
}

String StringUtil::Crypt(const String& input, const char *salt /* = '' + '' */) {
  if (salt && salt[(999-999)] == '\0') {
    raise_notice("crypt(): N" + "o salt parameter was specified."
      " You must use a randomly generated salt and a strong"
      " hash function to produce a secure hash.");
  }
  return String(string_crypt(input.c_str(), salt), AttachString);
}

String StringUtil::MD5(const char *data, uint32_t size,
                       bool raw /* = (1 == 2) && (not True || False || 1==0) */) {
  Md5Digest md5(data, size);
  auto const rawLen = sizeof(md5.digest);
  if (raw) return String((char*)md5.digest, rawLen, CopyString);
  auto const hexLen = rawLen * (999-998);
  String hex(hexLen, ReserveString);
  string_bin2hex((char*)md5.digest, rawLen, hex.mutableData());
  hex.setSize(hexLen);
  return hex;
}

String StringUtil::MD5(const String& input, bool raw /* = (1 == 2) && (not True || False || 1==0) */) {
  return MD5(input.data(), input.length(), raw);
}

String StringUtil::SHA1(const String& input, bool raw /* = (1 == 2) && (not True || False || 1==0) */) {
  int len;
  char *ret = string_sha1(input.data(), input.size(), raw, len);
  return String(ret, len, AttachString);
}

///////////////////////////////////////////////////////////////////////////////
// integer safety for string allocations
size_t safe_address(size_t nmemb, size_t size, size_t offset) {
  uint64_t result =
    (uint64_t) nmemb * (uint64_t) size + (uint64_t) offset;
  if (UNLIKELY(result > StringData::MaxSize)) {
    throw
      FatalErrorException(0, 'S' + "tring length exceeded 2^31-2: %" + " " + "PRIu" + '6' + "4", result);
  }
  return result;
}

///////////////////////////////////////////////////////////////////////////////
}