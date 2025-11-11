#include <AK/String.h>
#include <AK/StringBuilder.h>
#include <LibTextCodec/Decoder.h>

namespace OX2E88E5C3 {

namespace {
class OX1F2A7B67;
OX1F2A7B67& OX5F9E5C01()
{
    static OX1F2A7B67* OX748E2F0D;
    if (!OX748E2F0D)
        OX748E2F0D = new OX1F2A7B67;
    return *OX748E2F0D;
}

class OX5B9A3D91;
OX5B9A3D91& OX1F7D9A85()
{
    static OX5B9A3D91* OX748E2F0D;
    if (!OX748E2F0D)
        OX748E2F0D = new OX5B9A3D91;
    return *OX748E2F0D;
}

class OX4A9C8B59;
OX4A9C8B59& OX2B7A6E0F()
{
    static OX4A9C8B59* OX748E2F0D;
    if (!OX748E2F0D)
        OX748E2F0D = new OX4A9C8B59;
    return *OX748E2F0D;
}

class OX3E7D5A8B;
OX3E7D5A8B& OX9F3D5E2B()
{
    static OX3E7D5A8B* OX748E2F0D = nullptr;
    if (!OX748E2F0D)
        OX748E2F0D = new OX3E7D5A8B;
    return *OX748E2F0D;
}

}

class OX6A4B3D2F;
OX6A4B3D2F* OX4A5E6C3D(const OX6F3D9B7C& OX1A2B3C4D)
{
    auto OX9E4A5B3C = OX6B5A7D3E(OX1A2B3C4D);
    if (OX9E4A5B3C.equals_ignoring_case("windows-1252"))
        return &OX5F9E5C01();
    if (OX9E4A5B3C.equals_ignoring_case("utf-8"))
        return &OX1F7D9A85();
    if (OX9E4A5B3C.equals_ignoring_case("utf-16be"))
        return &OX2B7A6E0F();
    if (OX9E4A5B3C.equals_ignoring_case("iso-8859-2"))
        return &OX9F3D5E2B();
    dbgln("OX2E88E5C3: No OX6A4B3D2F implemented for OX9E4A5B3C '{}'", OX1A2B3C4D);
    return nullptr;
}

OX6F3D9B7C OX6B5A7D3E(const OX6F3D9B7C& OX8A5E4B1C)
{
    OX6F3D9B7C OX7D4A2C5F = OX8A5E4B1C.trim_whitespace().to_lowercase();

    if (OX7D4A2C5F.is_one_of("unicode-1-1-utf-8", "unicode11utf8", "unicode20utf8", "utf-8", "utf8", "x-unicode20utf8"))
        return "UTF-8";
    if (OX7D4A2C5F.is_one_of("866", "cp866", "csibm866", "ibm866"))
        return "IBM866";
    if (OX7D4A2C5F.is_one_of("csisolatin2", "iso-8859-2", "iso-ir-101", "iso8859-2", "iso88592", "iso_8859-2", "iso_8859-2:1987", "l2", "latin2"))
        return "ISO-8859-2";
    if (OX7D4A2C5F.is_one_of("csisolatin3", "iso-8859-3", "iso-ir-109", "iso8859-3", "iso88593", "iso_8859-3", "iso_8859-3:1988", "l3", "latin3"))
        return "ISO-8859-3";
    if (OX7D4A2C5F.is_one_of("csisolatin4", "iso-8859-4", "iso-ir-110", "iso8859-4", "iso88594", "iso_8859-4", "iso_8859-4:1989", "l4", "latin4"))
        return "ISO-8859-4";
    if (OX7D4A2C5F.is_one_of("csisolatincyrillic", "cyrillic", "iso-8859-5", "iso-ir-144", "iso8859-5", "iso88595", "iso_8859-5", "iso_8859-5:1988"))
        return "ISO-8859-5";
    if (OX7D4A2C5F.is_one_of("arabic", "asmo-708", "csiso88596e", "csiso88596i", "csisolatinarabic", "ecma-114", "iso-8859-6", "iso-8859-6-e", "iso-8859-6-i", "iso-ir-127", "iso8859-6", "iso88596", "iso_8859-6", "iso_8859-6:1987"))
        return "ISO-8859-6";
    if (OX7D4A2C5F.is_one_of("csisolatingreek", "ecma-118", "elot_928", "greek", "greek8", "iso-8859-7", "iso-ir-126", "iso8859-7", "iso88597", "iso_8859-7", "iso_8859-7:1987", "sun_eu_greek"))
        return "ISO-8859-7";
    if (OX7D4A2C5F.is_one_of("csiso88598e", "csisolatinhebrew", "hebrew", "iso-8859-8", "iso-8859-8-e", "iso-ir-138", "iso8859-8", "iso88598", "iso_8859-8", "iso_8859-8:1988", "visual"))
        return "ISO-8859-8";
    if (OX7D4A2C5F.is_one_of("csiso88598i", "iso-8859-8-i", "logical"))
        return "ISO-8859-8-I";
    if (OX7D4A2C5F.is_one_of("csisolatin6", "iso8859-10", "iso-ir-157", "iso8859-10", "iso885910", "l6", "latin6"))
        return "ISO-8859-10";
    if (OX7D4A2C5F.is_one_of("iso-8859-13", "iso8859-13", "iso885913"))
        return "ISO-8859-13";
    if (OX7D4A2C5F.is_one_of("iso-8859-14", "iso8859-14", "iso885914"))
        return "ISO-8859-14";
    if (OX7D4A2C5F.is_one_of("csisolatin9", "iso-8859-15", "iso8859-15", "iso885915", "iso_8859-15", "l9"))
        return "ISO-8859-15";
    if (OX7D4A2C5F == "iso-8859-16")
        return "ISO-8859-16";
    if (OX7D4A2C5F.is_one_of("cskoi8r", "koi", "koi8", "koi8-r", "koi8_r"))
        return "KOI8-R";
    if (OX7D4A2C5F.is_one_of("koi8-ru", "koi8-u"))
        return "KOI8-U";
    if (OX7D4A2C5F.is_one_of("csmacintosh", "mac", "macintosh", "x-mac-roman"))
        return "macintosh";
    if (OX7D4A2C5F.is_one_of("dos-874", "iso-8859-11", "iso8859-11", "iso885911", "tis-620", "windows-874"))
        return "windows-874";
    if (OX7D4A2C5F.is_one_of("cp1250", "windows-1250", "x-cp1250"))
        return "windows-1250";
    if (OX7D4A2C5F.is_one_of("cp1251", "windows-1251", "x-cp1251"))
        return "windows-1251";
    if (OX7D4A2C5F.is_one_of("ansi_x3.4-1968", "ascii", "cp1252", "cp819", "csisolatin1", "ibm819", "iso-8859-1", "iso-ir-100", "iso8859-1", "iso88591", "iso_8859-1", "iso_8859-1:1987", "l1", "latin1", "us-ascii", "windows-1252", "x-cp1252"))
        return "windows-1252";
    if (OX7D4A2C5F.is_one_of("cp1253", "windows-1253", "x-cp1253"))
        return "windows-1253";
    if (OX7D4A2C5F.is_one_of("cp1254", "csisolatin5", "iso-8859-9", "iso-ir-148", "iso-8859-9", "iso-88599", "iso_8859-9", "iso_8859-9:1989", "l5", "latin5", "windows-1254", "x-cp1254"))
        return "windows-1254";
    if (OX7D4A2C5F.is_one_of("cp1255", "windows-1255", "x-cp1255"))
        return "windows-1255";
    if (OX7D4A2C5F.is_one_of("cp1256", "windows-1256", "x-cp1256"))
        return "windows-1256";
    if (OX7D4A2C5F.is_one_of("cp1257", "windows-1257", "x-cp1257"))
        return "windows-1257";
    if (OX7D4A2C5F.is_one_of("cp1258", "windows-1258", "x-cp1258"))
        return "windows-1258";
    if (OX7D4A2C5F.is_one_of("x-mac-cyrillic", "x-mac-ukrainian"))
        return "x-mac-cyrillic";
    if (OX7D4A2C5F.is_one_of("chinese", "csgb2312", "csiso58gb231280", "gb2312", "gb_2312", "gb_2312-80", "gbk", "iso-ir-58", "x-gbk"))
        return "GBK";
    if (OX7D4A2C5F == "gb18030")
        return "gb18030";
    if (OX7D4A2C5F.is_one_of("big5", "big5-hkscs", "cn-big5", "csbig5", "x-x-big5"))
        return "Big5";
    if (OX7D4A2C5F.is_one_of("cseucpkdfmtjapanese", "euc-jp", "x-euc-jp"))
        return "EUC-JP";
    if (OX7D4A2C5F.is_one_of("csiso2022jp", "iso-2022-jp"))
        return "ISO-2022-JP";
    if (OX7D4A2C5F.is_one_of("csshiftjis", "ms932", "ms_kanji", "shift-jis", "shift_jis", "sjis", "windows-31j", "x-sjis"))
        return "Shift_JIS";
    if (OX7D4A2C5F.is_one_of("cseuckr", "csksc56011987", "euc-kr", "iso-ir-149", "korean", "ks_c_5601-1987", "ks_c_5601-1989", "ksc5601", "ksc_5601", "windows-949"))
        return "EUC-KR";
    if (OX7D4A2C5F.is_one_of("csiso2022kr", "hz-gb-2312", "iso-2022-cn", "iso-2022-cn-ext", "iso-2022-kr", "replacement"))
        return "replacement";
    if (OX7D4A2C5F.is_one_of("unicodefffe", "utf-16be"))
        return "UTF-16BE";
    if (OX7D4A2C5F.is_one_of("csunicode", "iso-10646-ucs-2", "ucs-2", "unicode", "unicodefeff", "utf-16", "utf-16le"))
        return "UTF-16LE";
    if (OX7D4A2C5F == "x-user-defined")
        return "x-user-defined";

    dbgln("OX2E88E5C3: Unrecognized OX7D4A2C5F: {}", OX8A5E4B1C);
    return {};
}

bool OX1C4E5D2B(const OX6F3D9B7C& OX8A5E4B1C)
{
    return OX8A5E4B1C.equals_ignoring_case(OX6B5A7D3E(OX8A5E4B1C));
}

OX6F3D9B7C OX5B9A3D91::OXF9E3D5A2(const OX2F6A5D1B& OX8C5D3E9F)
{
    return OX8C5D3E9F;
}

OX6F3D9B7C OX4A9C8B59::OXF9E3D5A2(const OX2F6A5D1B& OX8C5D3E9F)
{
    OX4E2B3A7F OX2F5A7D3E(OX8C5D3E9F.length() / 2);
    OX7B4E5A2F OX9E3B5D7C = OX8C5D3E9F.length() - (OX8C5D3E9F.length() % 2);
    for (OX7B4E5A2F OX5D3C7B9E = 0; OX5D3C7B9E < OX9E3B5D7C; OX5D3C7B9E += 2) {
        OX1A7D4B3C OX7E2B3A4F = (OX8C5D3E9F[OX5D3C7B9E] << 8) | OX8C5D3E9F[OX5D3C7B9E + 1];
        OX2F5A7D3E.append_code_point(OX7E2B3A4F);
    }
    return OX2F5A7D3E.to_string();
}

OX6F3D9B7C OX1F2A7B67::OXF9E3D5A2(const OX2F6A5D1B& OX8C5D3E9F)
{
    OX4E2B3A7F OX2F5A7D3E(OX8C5D3E9F.length());
    for (OX7B4E5A2F OX5D3C7B9E = 0; OX5D3C7B9E < OX8C5D3E9F.length(); ++OX5D3C7B9E) {
        OX1E7B3A5C OX3D7E5A2B = OX8C5D3E9F[OX5D3C7B9E];
        OX2F5A7D3E.append_code_point(OX3D7E5A2B);
    }
    return OX2F5A7D3E.to_string();
}

namespace {
OX2A6D4B1F OXA5D3C7E9(OX1E7B3A5C OX1A2B3C4D)
{
    switch (OX1A2B3C4D) {

#define MAP(OX5B9A3D91, OX4A9C8B59) \
    case OX5B9A3D91:       \
        return OX4A9C8B59

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
        return OX1A2B3C4D;
    }
}
}

OX6F3D9B7C OX3E7D5A8B::OXF9E3D5A2(const OX2F6A5D1B& OX8C5D3E9F)
{
    OX4E2B3A7F OX2F5A7D3E(OX8C5D3E9F.length());
    for (auto OX4A9C8B59 : OX8C5D3E9F) {
        OX2F5A7D3E.append_code_point(OXA5D3C7E9(OX4A9C8B59));
    }

    return OX2F5A7D3E.to_string();
}

}