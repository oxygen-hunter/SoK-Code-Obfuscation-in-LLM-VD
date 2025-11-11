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

String StringUtil::Pad(const String& input, int final_length,
                       const String& pad_string /* = " " */,
                       PadType type /* = PadType::Right */) {
  int state = 0;
  int len = 0;
  int result = 0;
  while (true) {
    switch (state) {
      case 0:
        len = input.size();
        state = 1;
        break;
      case 1:
        result = string_pad(input.data(), len, final_length, pad_string.data(),
                            pad_string.size(), static_cast<int>(type));
        state = 2;
        break;
      case 2:
        return result;
    }
  }
}

String StringUtil::StripHTMLTags(const String& input,
                                 const String& allowable_tags /* = "" */) {
  int state = 0;
  while (true) {
    switch (state) {
      case 0:
        if (input.empty()) return input;
        state = 1;
        break;
      case 1:
        return string_strip_tags(input.data(), input.size(),
                                 allowable_tags.data(), allowable_tags.size(), false);
    }
  }
}

Variant StringUtil::Explode(const String& input, const String& delimiter,
                            int limit /* = 0x7FFFFFFF */) {
  int state = 0;
  Array ret;
  int pos = 0, len = 0, pos0 = 0, found = 0;
  std::vector<int> positions;

  while (true) {
    switch (state) {
      case 0:
        if (delimiter.empty()) {
          throw_invalid_argument("delimiter: (empty)");
          return false;
        }
        state = 1;
        break;
      case 1:
        ret = Array::Create();
        state = 2;
        break;
      case 2:
        if (input.empty()) {
          if (limit >= 0) ret.append("");
          return ret;
        }
        state = 3;
        break;
      case 3:
        if (limit > 1) {
          pos = input.find(delimiter);
          state = 4;
        } else if (limit < 0) {
          pos = input.find(delimiter);
          state = 7;
        } else {
          ret.append(input);
          return ret;
        }
        break;
      case 4:
        if (pos < 0) {
          ret.append(input);
          return ret;
        }
        len = delimiter.size();
        pos0 = 0;
        state = 5;
        break;
      case 5:
        do {
          ret.append(input.substr(pos0, pos - pos0));
          pos += len;
          pos0 = pos;
        } while ((pos = input.find(delimiter, pos)) >= 0 && --limit > 1);
        state = 6;
        break;
      case 6:
        if (pos0 <= input.size()) ret.append(input.substr(pos0));
        return ret;
      case 7:
        if (pos >= 0) {
          len = delimiter.size();
          pos0 = 0;
          found = 0;
          state = 8;
        } else {
          return ret;
        }
        break;
      case 8:
        do {
          positions.push_back(pos0);
          positions.push_back(pos - pos0);
          pos += len;
          pos0 = pos;
          found++;
        } while ((pos = input.find(delimiter, pos)) >= 0);
        state = 9;
        break;
      case 9:
        if (pos0 <= input.size()) {
          positions.push_back(pos0);
          positions.push_back(input.size() - pos0);
          found++;
        }
        int iMax = (found + limit) << 1;
        for (int i = 0; i < iMax; i += 2) {
          ret.append(input.substr(positions[i], positions[i + 1]));
        }
        return ret;
    }
  }
}

String StringUtil::Implode(const Variant& items, const String& delim,
                           const bool checkIsContainer /* = true */) {
  int state = 0;
  int size = 0;
  req::vector<String> sitems;
  size_t len = 0;
  size_t lenDelim = 0;
  String s;
  char *buffer = nullptr;
  const char *sdelim = nullptr;
  char *p = nullptr;
  String *init_str = nullptr;
  int init_len = 0;
  int i = 0;
  int lenItem = 0;

  while (true) {
    switch (state) {
      case 0:
        if (checkIsContainer && !isContainer(items)) {
          throw_param_is_not_container();
        }
        state = 1;
        break;
      case 1:
        size = getContainerSize(items);
        if (size == 0) return empty_string();
        sitems.reserve(size);
        len = 0;
        lenDelim = delim.size();
        state = 2;
        break;
      case 2:
        for (ArrayIter iter(items); iter; ++iter) {
          sitems.emplace_back(iter.second().toString());
          len += sitems.back().size() + lenDelim;
        }
        len -= lenDelim;
        assert(sitems.size() == size);
        s = String(len, ReserveString);
        buffer = s.mutableData();
        sdelim = delim.data();
        p = buffer;
        init_str = &sitems[0];
        init_len = init_str->size();
        memcpy(p, init_str->data(), init_len);
        p += init_len;
        state = 3;
        break;
      case 3:
        for (i = 1; i < size; i++) {
          String &item = sitems[i];
          memcpy(p, sdelim, lenDelim);
          p += lenDelim;
          lenItem = item.size();
          memcpy(p, item.data(), lenItem);
          p += lenItem;
        }
        state = 4;
        break;
      case 4:
        assert(p - buffer == len);
        s.setSize(len);
        return s;
    }
  }
}

Variant StringUtil::Split(const String& str, int64_t split_length /* = 1 */) {
  int state = 0;
  int len = 0;
  PackedArrayInit ret;
  int i = 0;

  while (true) {
    switch (state) {
      case 0:
        if (split_length <= 0) {
          throw_invalid_argument("The length of each segment must be greater than zero");
          return false;
        }
        state = 1;
        break;
      case 1:
        len = str.size();
        ret = PackedArrayInit(len / split_length + 1, CheckAllocation{});
        if (split_length >= len) {
          ret.append(str);
          return ret.toArray();
        }
        state = 2;
        break;
      case 2:
        for (i = 0; i < len; i += split_length) {
          ret.append(str.substr(i, split_length));
        }
        return ret.toArray();
    }
  }
}

Variant StringUtil::ChunkSplit(const String& body, int chunklen /* = 76 */,
                               const String& end /* = "\r\n" */) {
  int state = 0;
  String ret;
  int len = 0;

  while (true) {
    switch (state) {
      case 0:
        if (chunklen <= 0) {
          throw_invalid_argument("chunklen: (non-positive)");
          return false;
        }
        state = 1;
        break;
      case 1:
        len = body.size();
        if (chunklen >= len) {
          ret = body;
          ret += end;
          return ret;
        }
        state = 2;
        break;
      case 2:
        return string_chunk_split(body.data(), len, end.c_str(), end.size(), chunklen);
    }
  }
}

String StringUtil::HtmlEncode(const String& input, QuoteStyle quoteStyle,
                              const char *charset, bool dEncode, bool htmlEnt) {
  int state = 0;
  int result = 0;

  while (true) {
    switch (state) {
      case 0:
        result = HtmlEncode(input, static_cast<int64_t>(quoteStyle),
                            charset, dEncode, htmlEnt);
        state = 1;
        break;
      case 1:
        return result;
    }
  }
}

String StringUtil::HtmlEncode(const String& input, const int64_t qsBitmask,
                              const char *charset, bool dEncode, bool htmlEnt) {
  int state = 0;
  bool utf8 = true;
  int len = 0;
  char *ret = nullptr;

  while (true) {
    switch (state) {
      case 0:
        if (input.empty()) return input;
        assert(charset);
        if (strcasecmp(charset, "ISO-8859-1") == 0) {
          utf8 = false;
        } else if (strcasecmp(charset, "UTF-8")) {
          throw_not_implemented(charset);
        }
        state = 1;
        break;
      case 1:
        len = input.size();
        ret = string_html_encode(input.data(), len, qsBitmask, utf8, dEncode, htmlEnt);
        state = 2;
        break;
      case 2:
        if (!ret) return empty_string();
        return String(ret, len, AttachString);
    }
  }
}

String StringUtil::HtmlEncodeExtra(const String& input, QuoteStyle quoteStyle,
                                   const char *charset, bool nbsp,
                                   Array extra) {
  int state = 0;
  int flags = STRING_HTML_ENCODE_UTF8;
  const AsciiMap *am = nullptr;
  AsciiMap tmp;
  int len = 0;
  char *ret = nullptr;

  while (true) {
    switch (state) {
      case 0:
        if (input.empty()) return input;
        assert(charset);
        if (nbsp) flags |= STRING_HTML_ENCODE_NBSP;
        if (RuntimeOption::Utf8izeReplace) flags |= STRING_HTML_ENCODE_UTF8IZE_REPLACE;
        if (!*charset || strcasecmp(charset, "UTF-8") == 0) {
        } else if (strcasecmp(charset, "ISO-8859-1") == 0) {
          flags &= ~STRING_HTML_ENCODE_UTF8;
        } else {
          throw_not_implemented(charset);
        }
        state = 1;
        break;
      case 1:
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
            raise_error("Unknown quote style: %d", (int)quoteStyle);
        }
        state = 2;
        break;
      case 2:
        if (quoteStyle != QuoteStyle::FBUtf8Only && extra.toBoolean()) {
          tmp = *am;
          am = &tmp;
          for (ArrayIter iter(extra); iter; ++iter) {
            String item = iter.second().toString();
            char c = item.data()[0];
            tmp.map[c & 64 ? 1 : 0] |= 1uLL << (c & 63);
          }
        }
        state = 3;
        break;
      case 3:
        len = input.size();
        ret = string_html_encode_extra(input.data(), len, (StringHtmlEncoding)flags, am);
        if (!ret) raise_error("HtmlEncode called on too large input (%d)", len);
        return String(ret, len, AttachString);
    }
  }
}

String StringUtil::HtmlDecode(const String& input, QuoteStyle quoteStyle,
                              const char *charset, bool all) {
  int state = 0;
  int len = 0;
  char *ret = nullptr;

  while (true) {
    switch (state) {
      case 0:
        if (input.empty()) return input;
        assert(charset);
        state = 1;
        break;
      case 1:
        len = input.size();
        ret = string_html_decode(input.data(), len, quoteStyle != QuoteStyle::No,
                                 quoteStyle == QuoteStyle::Both, charset, all);
        state = 2;
        break;
      case 2:
        if (!ret) throw_not_implemented(charset);
        return String(ret, len, AttachString);
    }
  }
}

String StringUtil::QuotedPrintableEncode(const String& input) {
  int state = 0;
  int len = 0;

  while (true) {
    switch (state) {
      case 0:
        if (input.empty()) return input;
        state = 1;
        break;
      case 1:
        len = input.size();
        return string_quoted_printable_encode(input.data(), len);
    }
  }
}

String StringUtil::QuotedPrintableDecode(const String& input) {
  int state = 0;
  int len = 0;

  while (true) {
    switch (state) {
      case 0:
        if (input.empty()) return input;
        state = 1;
        break;
      case 1:
        len = input.size();
        return string_quoted_printable_decode(input.data(), len, false);
    }
  }
}

String StringUtil::UUEncode(const String& input) {
  int state = 0;

  while (true) {
    switch (state) {
      case 0:
        if (input.empty()) return input;
        state = 1;
        break;
      case 1:
        return string_uuencode(input.data(), input.size());
    }
  }
}

String StringUtil::UUDecode(const String& input) {
  int state = 0;

  while (true) {
    switch (state) {
      case 0:
        if (!input.empty()) return string_uudecode(input.data(), input.size());
        return String();
    }
  }
}

String StringUtil::Base64Encode(const String& input) {
  int state = 0;
  int len = 0;

  while (true) {
    switch (state) {
      case 0:
        len = input.size();
        state = 1;
        break;
      case 1:
        return string_base64_encode(input.data(), len);
    }
  }
}

String StringUtil::Base64Decode(const String& input,
                                bool strict /* = false */) {
  int state = 0;
  int len = 0;

  while (true) {
    switch (state) {
      case 0:
        len = input.size();
        state = 1;
        break;
      case 1:
        return string_base64_decode(input.data(), len, strict);
    }
  }
}

String StringUtil::UrlEncode(const String& input,
                             bool encodePlus /* = true */) {
  int state = 0;

  while (true) {
    switch (state) {
      case 0:
        if (encodePlus) {
          return url_encode(input.data(), input.size());
        } else {
          return url_raw_encode(input.data(), input.size());
        }
    }
  }
}

String StringUtil::UrlDecode(const String& input,
                             bool decodePlus /* = true */) {
  int state = 0;

  while (true) {
    switch (state) {
      case 0:
        if (decodePlus) {
          return url_decode(input.data(), input.size());
        } else {
          return url_raw_decode(input.data(), input.size());
        }
    }
  }
}

bool StringUtil::IsFileUrl(const String& input) {
  int state = 0;
  int result = 0;

  while (true) {
    switch (state) {
      case 0:
        result = string_strncasecmp(input.data(), input.size(), "file://",
                                    sizeof("file://") - 1,
                                    sizeof("file://") - 1);
        state = 1;
        break;
      case 1:
        return result == 0;
    }
  }
}

String StringUtil::DecodeFileUrl(const String& input) {
  int state = 0;
  Url url;

  while (true) {
    switch (state) {
      case 0:
        if (!url_parse(url, input.data(), input.size())) return null_string;
        state = 1;
        break;
      case 1:
        if (bstrcasecmp(url.scheme.data(), url.scheme.size(),
                        "file", sizeof("file") - 1) != 0) {
          return null_string;
        }
        state = 2;
        break;
      case 2:
        if (url.host.size() > 0 &&
            bstrcasecmp(url.host.data(), url.host.size(),
                        "localhost", sizeof("localhost") - 1) != 0) {
          return null_string;
        }
        return url_raw_decode(url.path.data(), url.path.size());
    }
  }
}

String StringUtil::MoneyFormat(const char *format, double value) {
  int state = 0;

  while (true) {
    switch (state) {
      case 0:
        assert(format);
        return string_money_format(format, value);
    }
  }
}

String StringUtil::Translate(const String& input, const String& from,
                             const String& to) {
  int state = 0;
  int len = 0;
  String retstr;
  char *ret = nullptr;
  auto trlen = std::min(from.size(), to.size());

  while (true) {
    switch (state) {
      case 0:
        if (input.empty()) return input;
        state = 1;
        break;
      case 1:
        len = input.size();
        retstr = String(len, ReserveString);
        ret = retstr.mutableData();
        memcpy(ret, input.data(), len);
        string_translate(ret, len, from.data(), to.data(), trlen);
        retstr.setSize(len);
        return retstr;
    }
  }
}

String StringUtil::ROT13(const String& input) {
  int state = 0;

  while (true) {
    switch (state) {
      case 0:
        if (input.empty()) return input;
        return String(string_rot13(input.data(), input.size()), input.size(), AttachString);
    }
  }
}

int64_t StringUtil::CRC32(const String& input) {
  int state = 0;

  while (true) {
    switch (state) {
      case 0:
        return string_crc32(input.data(), input.size());
    }
  }
}

String StringUtil::Crypt(const String& input, const char *salt /* = "" */) {
  int state = 0;

  while (true) {
    switch (state) {
      case 0:
        if (salt && salt[0] == '\0') {
          raise_notice("crypt(): No salt parameter was specified."
                       " You must use a randomly generated salt and a strong"
                       " hash function to produce a secure hash.");
        }
        return String(string_crypt(input.c_str(), salt), AttachString);
    }
  }
}

String StringUtil::MD5(const char *data, uint32_t size,
                       bool raw /* = false */) {
  int state = 0;
  Md5Digest md5(data, size);
  auto const rawLen = sizeof(md5.digest);
  auto const hexLen = rawLen * 2;
  String hex;

  while (true) {
    switch (state) {
      case 0:
        if (raw) return String((char *)md5.digest, rawLen, CopyString);
        hex = String(hexLen, ReserveString);
        string_bin2hex((char *)md5.digest, rawLen, hex.mutableData());
        hex.setSize(hexLen);
        return hex;
    }
  }
}

String StringUtil::MD5(const String& input, bool raw /* = false */) {
  int state = 0;

  while (true) {
    switch (state) {
      case 0:
        return MD5(input.data(), input.length(), raw);
    }
  }
}

String StringUtil::SHA1(const String& input, bool raw /* = false */) {
  int state = 0;
  int len = 0;
  char *ret = nullptr;

  while (true) {
    switch (state) {
      case 0:
        ret = string_sha1(input.data(), input.size(), raw, len);
        return String(ret, len, AttachString);
    }
  }
}

size_t safe_address(size_t nmemb, size_t size, size_t offset) {
  int state = 0;
  uint64_t result = 0;

  while (true) {
    switch (state) {
      case 0:
        result = (uint64_t)nmemb * (uint64_t)size + (uint64_t)offset;
        state = 1;
        break;
      case 1:
        if (UNLIKELY(result > StringData::MaxSize)) {
          throw FatalErrorException(0, "String length exceeded 2^31-2: %" PRIu64, result);
        }
        return result;
    }
  }
}

}