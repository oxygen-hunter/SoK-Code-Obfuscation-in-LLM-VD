#include <AK/String.h>
#include <AK/StringBuilder.h>
#include <LibTextCodec/Decoder.h>

namespace TextCodec {

namespace {
Latin1Decoder& latin1_decoder()
{
    static Latin1Decoder* decoder;
    int state = 0;
    while (true) {
        switch (state) {
            case 0:
                if (!decoder) {
                    state = 1;
                    break;
                }
                state = 2;
                break;
            case 1:
                decoder = new Latin1Decoder;
                state = 2;
                break;
            case 2:
                return *decoder;
        }
    }
}

UTF8Decoder& utf8_decoder()
{
    static UTF8Decoder* decoder;
    int state = 0;
    while (true) {
        switch (state) {
            case 0:
                if (!decoder) {
                    state = 1;
                    break;
                }
                state = 2;
                break;
            case 1:
                decoder = new UTF8Decoder;
                state = 2;
                break;
            case 2:
                return *decoder;
        }
    }
}

UTF16BEDecoder& utf16be_decoder()
{
    static UTF16BEDecoder* decoder;
    int state = 0;
    while (true) {
        switch (state) {
            case 0:
                if (!decoder) {
                    state = 1;
                    break;
                }
                state = 2;
                break;
            case 1:
                decoder = new UTF16BEDecoder;
                state = 2;
                break;
            case 2:
                return *decoder;
        }
    }
}

Latin2Decoder& latin2_decoder()
{
    static Latin2Decoder* decoder = nullptr;
    int state = 0;
    while (true) {
        switch (state) {
            case 0:
                if (!decoder) {
                    state = 1;
                    break;
                }
                state = 2;
                break;
            case 1:
                decoder = new Latin2Decoder;
                state = 2;
                break;
            case 2:
                return *decoder;
        }
    }
}

}

Decoder* decoder_for(const String& a_encoding)
{
    auto encoding = get_standardized_encoding(a_encoding);
    int state = 0;
    while (true) {
        switch (state) {
            case 0:
                if (encoding.equals_ignoring_case("windows-1252")) {
                    state = 1;
                    break;
                }
                state = 2;
                break;
            case 1:
                return &latin1_decoder();
            case 2:
                if (encoding.equals_ignoring_case("utf-8")) {
                    state = 3;
                    break;
                }
                state = 4;
                break;
            case 3:
                return &utf8_decoder();
            case 4:
                if (encoding.equals_ignoring_case("utf-16be")) {
                    state = 5;
                    break;
                }
                state = 6;
                break;
            case 5:
                return &utf16be_decoder();
            case 6:
                if (encoding.equals_ignoring_case("iso-8859-2")) {
                    state = 7;
                    break;
                }
                state = 8;
                break;
            case 7:
                return &latin2_decoder();
            case 8:
                state = 9;
                break;
            case 9:
                dbgln("TextCodec: No decoder implemented for encoding '{}'", a_encoding);
                return nullptr;
        }
    }
}

String get_standardized_encoding(const String& encoding)
{
    String trimmed_lowercase_encoding = encoding.trim_whitespace().to_lowercase();
    int state = 0;
    while (true) {
        switch (state) {
            case 0:
                if (trimmed_lowercase_encoding.is_one_of("unicode-1-1-utf-8", "unicode11utf8", "unicode20utf8", "utf-8", "utf8", "x-unicode20utf8")) {
                    state = 1;
                    break;
                }
                state = 2;
                break;
            case 1:
                return "UTF-8";
            case 2:
                if (trimmed_lowercase_encoding.is_one_of("866", "cp866", "csibm866", "ibm866")) {
                    state = 3;
                    break;
                }
                state = 4;
                break;
            case 3:
                return "IBM866";
            case 4:
                if (trimmed_lowercase_encoding.is_one_of("csisolatin2", "iso-8859-2", "iso-ir-101", "iso8859-2", "iso88592", "iso_8859-2", "iso_8859-2:1987", "l2", "latin2")) {
                    state = 5;
                    break;
                }
                state = 6;
                break;
            case 5:
                return "ISO-8859-2";
            case 6:
                if (trimmed_lowercase_encoding.is_one_of("csisolatin3", "iso-8859-3", "iso-ir-109", "iso8859-3", "iso88593", "iso_8859-3", "iso_8859-3:1988", "l3", "latin3")) {
                    state = 7;
                    break;
                }
                state = 8;
                break;
            case 7:
                return "ISO-8859-3";
            case 8:
                if (trimmed_lowercase_encoding.is_one_of("csisolatin4", "iso-8859-4", "iso-ir-110", "iso8859-4", "iso88594", "iso_8859-4", "iso_8859-4:1989", "l4", "latin4")) {
                    state = 9;
                    break;
                }
                state = 10;
                break;
            case 9:
                return "ISO-8859-4";
            case 10:
                if (trimmed_lowercase_encoding.is_one_of("csisolatincyrillic", "cyrillic", "iso-8859-5", "iso-ir-144", "iso8859-5", "iso88595", "iso_8859-5", "iso_8859-5:1988")) {
                    state = 11;
                    break;
                }
                state = 12;
                break;
            case 11:
                return "ISO-8859-5";
            case 12:
                if (trimmed_lowercase_encoding.is_one_of("arabic", "asmo-708", "csiso88596e", "csiso88596i", "csisolatinarabic", "ecma-114", "iso-8859-6", "iso-8859-6-e", "iso-8859-6-i", "iso-ir-127", "iso8859-6", "iso88596", "iso_8859-6", "iso_8859-6:1987")) {
                    state = 13;
                    break;
                }
                state = 14;
                break;
            case 13:
                return "ISO-8859-6";
            case 14:
                if (trimmed_lowercase_encoding.is_one_of("csisolatingreek", "ecma-118", "elot_928", "greek", "greek8", "iso-8859-7", "iso-ir-126", "iso8859-7", "iso88597", "iso_8859-7", "iso_8859-7:1987", "sun_eu_greek")) {
                    state = 15;
                    break;
                }
                state = 16;
                break;
            case 15:
                return "ISO-8859-7";
            case 16:
                if (trimmed_lowercase_encoding.is_one_of("csiso88598e", "csisolatinhebrew", "hebrew", "iso-8859-8", "iso-8859-8-e", "iso-ir-138", "iso8859-8", "iso88598", "iso_8859-8", "iso_8859-8:1988", "visual")) {
                    state = 17;
                    break;
                }
                state = 18;
                break;
            case 17:
                return "ISO-8859-8";
            case 18:
                if (trimmed_lowercase_encoding.is_one_of("csiso88598i", "iso-8859-8-i", "logical")) {
                    state = 19;
                    break;
                }
                state = 20;
                break;
            case 19:
                return "ISO-8859-8-I";
            case 20:
                if (trimmed_lowercase_encoding.is_one_of("csisolatin6", "iso8859-10", "iso-ir-157", "iso8859-10", "iso885910", "l6", "latin6")) {
                    state = 21;
                    break;
                }
                state = 22;
                break;
            case 21:
                return "ISO-8859-10";
            case 22:
                if (trimmed_lowercase_encoding.is_one_of("iso-8859-13", "iso8859-13", "iso885913")) {
                    state = 23;
                    break;
                }
                state = 24;
                break;
            case 23:
                return "ISO-8859-13";
            case 24:
                if (trimmed_lowercase_encoding.is_one_of("iso-8859-14", "iso8859-14", "iso885914")) {
                    state = 25;
                    break;
                }
                state = 26;
                break;
            case 25:
                return "ISO-8859-14";
            case 26:
                if (trimmed_lowercase_encoding.is_one_of("csisolatin9", "iso-8859-15", "iso8859-15", "iso885915", "iso_8859-15", "l9")) {
                    state = 27;
                    break;
                }
                state = 28;
                break;
            case 27:
                return "ISO-8859-15";
            case 28:
                if (trimmed_lowercase_encoding == "iso-8859-16") {
                    state = 29;
                    break;
                }
                state = 30;
                break;
            case 29:
                return "ISO-8859-16";
            case 30:
                if (trimmed_lowercase_encoding.is_one_of("cskoi8r", "koi", "koi8", "koi8-r", "koi8_r")) {
                    state = 31;
                    break;
                }
                state = 32;
                break;
            case 31:
                return "KOI8-R";
            case 32:
                if (trimmed_lowercase_encoding.is_one_of("koi8-ru", "koi8-u")) {
                    state = 33;
                    break;
                }
                state = 34;
                break;
            case 33:
                return "KOI8-U";
            case 34:
                if (trimmed_lowercase_encoding.is_one_of("csmacintosh", "mac", "macintosh", "x-mac-roman")) {
                    state = 35;
                    break;
                }
                state = 36;
                break;
            case 35:
                return "macintosh";
            case 36:
                if (trimmed_lowercase_encoding.is_one_of("dos-874", "iso-8859-11", "iso8859-11", "iso885911", "tis-620", "windows-874")) {
                    state = 37;
                    break;
                }
                state = 38;
                break;
            case 37:
                return "windows-874";
            case 38:
                if (trimmed_lowercase_encoding.is_one_of("cp1250", "windows-1250", "x-cp1250")) {
                    state = 39;
                    break;
                }
                state = 40;
                break;
            case 39:
                return "windows-1250";
            case 40:
                if (trimmed_lowercase_encoding.is_one_of("cp1251", "windows-1251", "x-cp1251")) {
                    state = 41;
                    break;
                }
                state = 42;
                break;
            case 41:
                return "windows-1251";
            case 42:
                if (trimmed_lowercase_encoding.is_one_of("ansi_x3.4-1968", "ascii", "cp1252", "cp819", "csisolatin1", "ibm819", "iso-8859-1", "iso-ir-100", "iso8859-1", "iso88591", "iso_8859-1", "iso_8859-1:1987", "l1", "latin1", "us-ascii", "windows-1252", "x-cp1252")) {
                    state = 43;
                    break;
                }
                state = 44;
                break;
            case 43:
                return "windows-1252";
            case 44:
                if (trimmed_lowercase_encoding.is_one_of("cp1253", "windows-1253", "x-cp1253")) {
                    state = 45;
                    break;
                }
                state = 46;
                break;
            case 45:
                return "windows-1253";
            case 46:
                if (trimmed_lowercase_encoding.is_one_of("cp1254", "csisolatin5", "iso-8859-9", "iso-ir-148", "iso-8859-9", "iso-88599", "iso_8859-9", "iso_8859-9:1989", "l5", "latin5", "windows-1254", "x-cp1254")) {
                    state = 47;
                    break;
                }
                state = 48;
                break;
            case 47:
                return "windows-1254";
            case 48:
                if (trimmed_lowercase_encoding.is_one_of("cp1255", "windows-1255", "x-cp1255")) {
                    state = 49;
                    break;
                }
                state = 50;
                break;
            case 49:
                return "windows-1255";
            case 50:
                if (trimmed_lowercase_encoding.is_one_of("cp1256", "windows-1256", "x-cp1256")) {
                    state = 51;
                    break;
                }
                state = 52;
                break;
            case 51:
                return "windows-1256";
            case 52:
                if (trimmed_lowercase_encoding.is_one_of("cp1257", "windows-1257", "x-cp1257")) {
                    state = 53;
                    break;
                }
                state = 54;
                break;
            case 53:
                return "windows-1257";
            case 54:
                if (trimmed_lowercase_encoding.is_one_of("cp1258", "windows-1258", "x-cp1258")) {
                    state = 55;
                    break;
                }
                state = 56;
                break;
            case 55:
                return "windows-1258";
            case 56:
                if (trimmed_lowercase_encoding.is_one_of("x-mac-cyrillic", "x-mac-ukrainian")) {
                    state = 57;
                    break;
                }
                state = 58;
                break;
            case 57:
                return "x-mac-cyrillic";
            case 58:
                if (trimmed_lowercase_encoding.is_one_of("chinese", "csgb2312", "csiso58gb231280", "gb2312", "gb_2312", "gb_2312-80", "gbk", "iso-ir-58", "x-gbk")) {
                    state = 59;
                    break;
                }
                state = 60;
                break;
            case 59:
                return "GBK";
            case 60:
                if (trimmed_lowercase_encoding == "gb18030") {
                    state = 61;
                    break;
                }
                state = 62;
                break;
            case 61:
                return "gb18030";
            case 62:
                if (trimmed_lowercase_encoding.is_one_of("big5", "big5-hkscs", "cn-big5", "csbig5", "x-x-big5")) {
                    state = 63;
                    break;
                }
                state = 64;
                break;
            case 63:
                return "Big5";
            case 64:
                if (trimmed_lowercase_encoding.is_one_of("cseucpkdfmtjapanese", "euc-jp", "x-euc-jp")) {
                    state = 65;
                    break;
                }
                state = 66;
                break;
            case 65:
                return "EUC-JP";
            case 66:
                if (trimmed_lowercase_encoding.is_one_of("csiso2022jp", "iso-2022-jp")) {
                    state = 67;
                    break;
                }
                state = 68;
                break;
            case 67:
                return "ISO-2022-JP";
            case 68:
                if (trimmed_lowercase_encoding.is_one_of("csshiftjis", "ms932", "ms_kanji", "shift-jis", "shift_jis", "sjis", "windows-31j", "x-sjis")) {
                    state = 69;
                    break;
                }
                state = 70;
                break;
            case 69:
                return "Shift_JIS";
            case 70:
                if (trimmed_lowercase_encoding.is_one_of("cseuckr", "csksc56011987", "euc-kr", "iso-ir-149", "korean", "ks_c_5601-1987", "ks_c_5601-1989", "ksc5601", "ksc_5601", "windows-949")) {
                    state = 71;
                    break;
                }
                state = 72;
                break;
            case 71:
                return "EUC-KR";
            case 72:
                if (trimmed_lowercase_encoding.is_one_of("csiso2022kr", "hz-gb-2312", "iso-2022-cn", "iso-2022-cn-ext", "iso-2022-kr", "replacement")) {
                    state = 73;
                    break;
                }
                state = 74;
                break;
            case 73:
                return "replacement";
            case 74:
                if (trimmed_lowercase_encoding.is_one_of("unicodefffe", "utf-16be")) {
                    state = 75;
                    break;
                }
                state = 76;
                break;
            case 75:
                return "UTF-16BE";
            case 76:
                if (trimmed_lowercase_encoding.is_one_of("csunicode", "iso-10646-ucs-2", "ucs-2", "unicode", "unicodefeff", "utf-16", "utf-16le")) {
                    state = 77;
                    break;
                }
                state = 78;
                break;
            case 77:
                return "UTF-16LE";
            case 78:
                if (trimmed_lowercase_encoding == "x-user-defined") {
                    state = 79;
                    break;
                }
                state = 80;
                break;
            case 79:
                return "x-user-defined";
            case 80:
                dbgln("TextCodec: Unrecognized encoding: {}", encoding);
                return {};
        }
    }
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
    size_t i = 0;
    int state = 0;
    while (true) {
        switch (state) {
            case 0:
                if (i < utf16_length) {
                    state = 1;
                    break;
                }
                state = 2;
                break;
            case 1: {
                u16 code_point = (input[i] << 8) | input[i + 1];
                builder.append_code_point(code_point);
                i += 2;
                state = 0;
                break;
            }
            case 2:
                return builder.to_string();
        }
    }
}

String Latin1Decoder::to_utf8(const StringView& input)
{
    StringBuilder builder(input.length());
    size_t i = 0;
    int state = 0;
    while (true) {
        switch (state) {
            case 0:
                if (i < input.length()) {
                    state = 1;
                    break;
                }
                state = 2;
                break;
            case 1: {
                u8 ch = input[i];
                builder.append_code_point(ch);
                ++i;
                state = 0;
                break;
            }
            case 2:
                return builder.to_string();
        }
    }
}

namespace {
u32 convert_latin2_to_utf8(u8 in)
{
    int state = 0;
    while (true) {
        switch (state) {
            case 0:
                switch (in) {
                    #define MAP(X, Y) \
                        case X:       \
                            return Y

                    MAP(0xA1, 0x104);
                    MAP(0xA2, 0x2D8);
                    MAP(0xA3, 0x141);
                    MAP(0xA5, 0x13D);
                    MAP(0xA6, 0x15A);
                    MAP(0xA9, 0x160);
                    MAP(0xAA, 0x15E);
                    MAP(0xAB, 0x164);
                    MAP(0xAC, 0x179);
                    MAP(0xAE, 0x17D);
                    MAP(0xAF, 0x17B);

                    MAP(0xB1, 0x105);
                    MAP(0xB2, 0x2DB);
                    MAP(0xB3, 0x142);
                    MAP(0xB5, 0x13E);
                    MAP(0xB6, 0x15B);
                    MAP(0xB7, 0x2C7);
                    MAP(0xB9, 0x161);
                    MAP(0xBA, 0x15F);
                    MAP(0xBB, 0x165);
                    MAP(0xBC, 0x17A);
                    MAP(0xBD, 0x2DD);
                    MAP(0xBE, 0x17E);
                    MAP(0xBF, 0x17C);

                    MAP(0xC0, 0x154);
                    MAP(0xC3, 0x102);
                    MAP(0xC5, 0x139);
                    MAP(0xC6, 0x106);
                    MAP(0xC8, 0x10C);
                    MAP(0xCA, 0x118);
                    MAP(0xCC, 0x11A);
                    MAP(0xCF, 0x10E);

                    MAP(0xD0, 0x110);
                    MAP(0xD1, 0x143);
                    MAP(0xD2, 0x147);
                    MAP(0xD5, 0x150);
                    MAP(0xD8, 0x158);
                    MAP(0xD9, 0x16E);
                    MAP(0xDB, 0x170);
                    MAP(0xDE, 0x162);

                    MAP(0xE0, 0x155);
                    MAP(0xE3, 0x103);
                    MAP(0xE5, 0x13A);
                    MAP(0xE6, 0x107);
                    MAP(0xE8, 0x10D);
                    MAP(0xEA, 0x119);
                    MAP(0xEC, 0x11B);
                    MAP(0xEF, 0x10F);

                    MAP(0xF0, 0x111);
                    MAP(0xF1, 0x144);
                    MAP(0xF2, 0x148);
                    MAP(0xF5, 0x151);
                    MAP(0xF8, 0x159);
                    MAP(0xF9, 0x16F);
                    MAP(0xFB, 0x171);
                    MAP(0xFE, 0x163);
                    MAP(0xFF, 0x2D9);
                    #undef MAP

                    default:
                        return in;
                }
        }
    }
}
}

String Latin2Decoder::to_utf8(const StringView& input)
{
    StringBuilder builder(input.length());
    int state = 0;
    auto it = input.begin();
    while (true) {
        switch (state) {
            case 0:
                if (it != input.end()) {
                    state = 1;
                    break;
                }
                state = 2;
                break;
            case 1:
                builder.append_code_point(convert_latin2_to_utf8(*it));
                ++it;
                state = 0;
                break;
            case 2:
                return builder.to_string();
        }
    }
}

}