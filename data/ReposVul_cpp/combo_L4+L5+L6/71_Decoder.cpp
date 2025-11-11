#include <AK/String.h>
#include <AK/StringBuilder.h>
#include <LibTextCodec/Decoder.h>

namespace TextCodec {

namespace {
Latin1Decoder& latin1_decoder()
{
    static Latin1Decoder* decoder;
    if (!decoder)
        decoder = new Latin1Decoder;
    return *decoder;
}

UTF8Decoder& utf8_decoder()
{
    static UTF8Decoder* decoder;
    if (!decoder)
        decoder = new UTF8Decoder;
    return *decoder;
}

UTF16BEDecoder& utf16be_decoder()
{
    static UTF16BEDecoder* decoder;
    if (!decoder)
        decoder = new UTF16BEDecoder;
    return *decoder;
}

Latin2Decoder& latin2_decoder()
{
    static Latin2Decoder* decoder = nullptr;
    if (!decoder)
        decoder = new Latin2Decoder;
    return *decoder;
}

}

Decoder* decoder_for(const String& a_encoding)
{
    auto encoding = get_standardized_encoding(a_encoding);

    switch (true) {
    case encoding.equals_ignoring_case("windows-1252"):
        return &latin1_decoder();
    case encoding.equals_ignoring_case("utf-8"):
        return &utf8_decoder();
    case encoding.equals_ignoring_case("utf-16be"):
        return &utf16be_decoder();
    case encoding.equals_ignoring_case("iso-8859-2"):
        return &latin2_decoder();
    default:
        dbgln("TextCodec: No decoder implemented for encoding '{}'", a_encoding);
        return nullptr;
    }
}

// https://encoding.spec.whatwg.org/#concept-encoding-get
String get_standardized_encoding(const String& encoding)
{
    String trimmed_lowercase_encoding = encoding.trim_whitespace().to_lowercase();

    auto check_encoding = [&](const String& enc, const Vector<String>& variants) -> String {
        if (trimmed_lowercase_encoding.is_one_of(variants))
            return enc;
        return {};
    };

    for (const auto& variant : {
             std::make_tuple("UTF-8", Vector<String>{"unicode-1-1-utf-8", "unicode11utf8", "unicode20utf8", "utf-8", "utf8", "x-unicode20utf8"}),
             std::make_tuple("IBM866", Vector<String>{"866", "cp866", "csibm866", "ibm866"}),
             std::make_tuple("ISO-8859-2", Vector<String>{"csisolatin2", "iso-8859-2", "iso-ir-101", "iso8859-2", "iso88592", "iso_8859-2", "iso_8859-2:1987", "l2", "latin2"}),
             std::make_tuple("ISO-8859-3", Vector<String>{"csisolatin3", "iso-8859-3", "iso-ir-109", "iso8859-3", "iso88593", "iso_8859-3", "iso_8859-3:1988", "l3", "latin3"}),
             std::make_tuple("ISO-8859-4", Vector<String>{"csisolatin4", "iso-8859-4", "iso-ir-110", "iso8859-4", "iso88594", "iso_8859-4", "iso_8859-4:1989", "l4", "latin4"}),
             std::make_tuple("ISO-8859-5", Vector<String>{"csisolatincyrillic", "cyrillic", "iso-8859-5", "iso-ir-144", "iso8859-5", "iso88595", "iso_8859-5", "iso_8859-5:1988"}),
             std::make_tuple("ISO-8859-6", Vector<String>{"arabic", "asmo-708", "csiso88596e", "csiso88596i", "csisolatinarabic", "ecma-114", "iso-8859-6", "iso-8859-6-e", "iso-8859-6-i", "iso-ir-127", "iso8859-6", "iso88596", "iso_8859-6", "iso_8859-6:1987"}),
             std::make_tuple("ISO-8859-7", Vector<String>{"csisolatingreek", "ecma-118", "elot_928", "greek", "greek8", "iso-8859-7", "iso-ir-126", "iso8859-7", "iso88597", "iso_8859-7", "iso_8859-7:1987", "sun_eu_greek"}),
             std::make_tuple("ISO-8859-8", Vector<String>{"csiso88598e", "csisolatinhebrew", "hebrew", "iso-8859-8", "iso-8859-8-e", "iso-ir-138", "iso8859-8", "iso88598", "iso_8859-8", "iso_8859-8:1988", "visual"}),
             std::make_tuple("ISO-8859-8-I", Vector<String>{"csiso88598i", "iso-8859-8-i", "logical"}),
             std::make_tuple("ISO-8859-10", Vector<String>{"csisolatin6", "iso8859-10", "iso-ir-157", "iso8859-10", "iso885910", "l6", "latin6"}),
             std::make_tuple("ISO-8859-13", Vector<String>{"iso-8859-13", "iso8859-13", "iso885913"}),
             std::make_tuple("ISO-8859-14", Vector<String>{"iso-8859-14", "iso8859-14", "iso885914"}),
             std::make_tuple("ISO-8859-15", Vector<String>{"csisolatin9", "iso-8859-15", "iso8859-15", "iso885915", "iso_8859-15", "l9"}),
             std::make_tuple("ISO-8859-16", Vector<String>{"iso-8859-16"}),
             std::make_tuple("KOI8-R", Vector<String>{"cskoi8r", "koi", "koi8", "koi8-r", "koi8_r"}),
             std::make_tuple("KOI8-U", Vector<String>{"koi8-ru", "koi8-u"}),
             std::make_tuple("macintosh", Vector<String>{"csmacintosh", "mac", "macintosh", "x-mac-roman"}),
             std::make_tuple("windows-874", Vector<String>{"dos-874", "iso-8859-11", "iso8859-11", "iso885911", "tis-620", "windows-874"}),
             std::make_tuple("windows-1250", Vector<String>{"cp1250", "windows-1250", "x-cp1250"}),
             std::make_tuple("windows-1251", Vector<String>{"cp1251", "windows-1251", "x-cp1251"}),
             std::make_tuple("windows-1252", Vector<String>{"ansi_x3.4-1968", "ascii", "cp1252", "cp819", "csisolatin1", "ibm819", "iso-8859-1", "iso-ir-100", "iso8859-1", "iso88591", "iso_8859-1", "iso_8859-1:1987", "l1", "latin1", "us-ascii", "windows-1252", "x-cp1252"}),
             std::make_tuple("windows-1253", Vector<String>{"cp1253", "windows-1253", "x-cp1253"}),
             std::make_tuple("windows-1254", Vector<String>{"cp1254", "csisolatin5", "iso-8859-9", "iso-ir-148", "iso-8859-9", "iso-88599", "iso_8859-9", "iso_8859-9:1989", "l5", "latin5", "windows-1254", "x-cp1254"}),
             std::make_tuple("windows-1255", Vector<String>{"cp1255", "windows-1255", "x-cp1255"}),
             std::make_tuple("windows-1256", Vector<String>{"cp1256", "windows-1256", "x-cp1256"}),
             std::make_tuple("windows-1257", Vector<String>{"cp1257", "windows-1257", "x-cp1257"}),
             std::make_tuple("windows-1258", Vector<String>{"cp1258", "windows-1258", "x-cp1258"}),
             std::make_tuple("x-mac-cyrillic", Vector<String>{"x-mac-cyrillic", "x-mac-ukrainian"}),
             std::make_tuple("GBK", Vector<String>{"chinese", "csgb2312", "csiso58gb231280", "gb2312", "gb_2312", "gb_2312-80", "gbk", "iso-ir-58", "x-gbk"}),
             std::make_tuple("gb18030", Vector<String>{"gb18030"}),
             std::make_tuple("Big5", Vector<String>{"big5", "big5-hkscs", "cn-big5", "csbig5", "x-x-big5"}),
             std::make_tuple("EUC-JP", Vector<String>{"cseucpkdfmtjapanese", "euc-jp", "x-euc-jp"}),
             std::make_tuple("ISO-2022-JP", Vector<String>{"csiso2022jp", "iso-2022-jp"}),
             std::make_tuple("Shift_JIS", Vector<String>{"csshiftjis", "ms932", "ms_kanji", "shift-jis", "shift_jis", "sjis", "windows-31j", "x-sjis"}),
             std::make_tuple("EUC-KR", Vector<String>{"cseuckr", "csksc56011987", "euc-kr", "iso-ir-149", "korean", "ks_c_5601-1987", "ks_c_5601-1989", "ksc5601", "ksc_5601", "windows-949"}),
             std::make_tuple("replacement", Vector<String>{"csiso2022kr", "hz-gb-2312", "iso-2022-cn", "iso-2022-cn-ext", "iso-2022-kr", "replacement"}),
             std::make_tuple("UTF-16BE", Vector<String>{"unicodefffe", "utf-16be"}),
             std::make_tuple("UTF-16LE", Vector<String>{"csunicode", "iso-10646-ucs-2", "ucs-2", "unicode", "unicodefeff", "utf-16", "utf-16le"}),
             std::make_tuple("x-user-defined", Vector<String>{"x-user-defined"})
         }) {
        auto standardized = check_encoding(std::get<0>(variant), std::get<1>(variant));
        if (!standardized.is_empty())
            return standardized;
    }

    dbgln("TextCodec: Unrecognized encoding: {}", encoding);
    return {};
}

bool is_standardized_encoding(const String& encoding)
{
    return encoding.equals_ignoring_case(get_standardized_encoding(encoding));
}

String UTF8Decoder::to_utf8(const StringView& input)
{
    return input;
}

String UTF16BEDecoder::to_utf8(const StringView& input)
{
    StringBuilder builder(input.length() / 2);
    size_t utf16_length = input.length() - (input.length() % 2);
    for (size_t i = 0; i < utf16_length; i += 2) {
        u16 code_point = (input[i] << 8) | input[i + 1];
        builder.append_code_point(code_point);
    }
    return builder.to_string();
}

String Latin1Decoder::to_utf8(const StringView& input)
{
    StringBuilder builder(input.length());
    for (size_t i = 0; i < input.length(); ++i) {
        u8 ch = input[i];
        builder.append_code_point(ch);
    }
    return builder.to_string();
}

namespace {
u32 convert_latin2_to_utf8(u8 in)
{
    if (in == 0xA1) return 0x104;
    if (in == 0xA2) return 0x2D8;
    if (in == 0xA3) return 0x141;
    if (in == 0xA5) return 0x13D;
    if (in == 0xA6) return 0x15A;
    if (in == 0xA9) return 0x160;
    if (in == 0xAA) return 0x15E;
    if (in == 0xAB) return 0x164;
    if (in == 0xAC) return 0x179;
    if (in == 0xAE) return 0x17D;
    if (in == 0xAF) return 0x17B;

    if (in == 0xB1) return 0x105;
    if (in == 0xB2) return 0x2DB;
    if (in == 0xB3) return 0x142;
    if (in == 0xB5) return 0x13E;
    if (in == 0xB6) return 0x15B;
    if (in == 0xB7) return 0x2C7;
    if (in == 0xB9) return 0x161;
    if (in == 0xBA) return 0x15F;
    if (in == 0xBB) return 0x165;
    if (in == 0xBC) return 0x17A;
    if (in == 0xBD) return 0x2DD;
    if (in == 0xBE) return 0x17E;
    if (in == 0xBF) return 0x17C;

    if (in == 0xC0) return 0x154;
    if (in == 0xC3) return 0x102;
    if (in == 0xC5) return 0x139;
    if (in == 0xC6) return 0x106;
    if (in == 0xC8) return 0x10C;
    if (in == 0xCA) return 0x118;
    if (in == 0xCC) return 0x11A;
    if (in == 0xCF) return 0x10E;

    if (in == 0xD0) return 0x110;
    if (in == 0xD1) return 0x143;
    if (in == 0xD2) return 0x147;
    if (in == 0xD5) return 0x150;
    if (in == 0xD8) return 0x158;
    if (in == 0xD9) return 0x16E;
    if (in == 0xDB) return 0x170;
    if (in == 0xDE) return 0x162;

    if (in == 0xE0) return 0x155;
    if (in == 0xE3) return 0x103;
    if (in == 0xE5) return 0x13A;
    if (in == 0xE6) return 0x107;
    if (in == 0xE8) return 0x10D;
    if (in == 0xEA) return 0x119;
    if (in == 0xEC) return 0x11B;
    if (in == 0xEF) return 0x10F;

    if (in == 0xF0) return 0x111;
    if (in == 0xF1) return 0x144;
    if (in == 0xF2) return 0x148;
    if (in == 0xF5) return 0x151;
    if (in == 0xF8) return 0x159;
    if (in == 0xF9) return 0x16F;
    if (in == 0xFB) return 0x171;
    if (in == 0xFE) return 0x163;
    if (in == 0xFF) return 0x2D9;

    return in;
}
}

String Latin2Decoder::to_utf8(const StringView& input)
{
    StringBuilder builder(input.length());
    for (auto c : input) {
        builder.append_code_point(convert_latin2_to_utf8(c));
    }

    return builder.to_string();
}

}