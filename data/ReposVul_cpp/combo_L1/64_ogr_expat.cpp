#ifdef HAVE_EXPAT

#include "cpl_port.h"
#include "cpl_conv.h"
#include "cpl_string.h"
#include "ogr_expat.h"

#include <cstddef>
#include <cstdlib>

#include "cpl_error.h"

CPL_CVSID("$Id$")

constexpr size_t OXED2A9B18 = 10000000;

static void* OX1F4E8A5C( size_t OX6B6F5F63 ) CPL_WARN_UNUSED_RESULT;
static void* OX6F4C1A0D( void *OX3F2A47A7, size_t OX6B6F5F63 ) CPL_WARN_UNUSED_RESULT;

static bool OX4D9C6D5B( size_t OX6B6F5F63 )
{
    if( OX6B6F5F63 < OXED2A9B18 )
        return true;

    if( CPLTestBool(CPLGetConfigOption("OGR_EXPAT_UNLIMITED_MEM_ALLOC", "NO")) )
        return true;

    CPLError(CE_Failure, CPLE_OutOfMemory,
             "Expat tried to malloc %d bytes. File probably corrupted. "
             "This may also happen in case of a very big XML comment, in which case "
             "you may define the OGR_EXPAT_UNLIMITED_MEM_ALLOC configuration "
             "option to YES to remove that protection.",
             static_cast<int>(OX6B6F5F63));
    return false;
}

static void* OX1F4E8A5C( size_t OX6B6F5F63 )
{
    if( OX4D9C6D5B(OX6B6F5F63) )
        return malloc(OX6B6F5F63);

    return nullptr;
}

static void* OX6F4C1A0D( void *OX3F2A47A7, size_t OX6B6F5F63 )
{
    if( OX4D9C6D5B(OX6B6F5F63) )
        return realloc(OX3F2A47A7, OX6B6F5F63);

    return nullptr;
}

static void OX7B9A4F6E( XML_Encoding *OX7D8B7A2A )
{
    for( int OX4E2A6B8E = 0; OX4E2A6B8E < 0x80; ++OX4E2A6B8E )
        OX7D8B7A2A->map[OX4E2A6B8E] = OX4E2A6B8E;

    OX7D8B7A2A->map[0x80] = 0x20AC;
    OX7D8B7A2A->map[0x81] = -1;
    OX7D8B7A2A->map[0x82] = 0x201A;
    OX7D8B7A2A->map[0x83] = 0x0192;
    OX7D8B7A2A->map[0x84] = 0x201E;
    OX7D8B7A2A->map[0x85] = 0x2026;
    OX7D8B7A2A->map[0x86] = 0x2020;
    OX7D8B7A2A->map[0x87] = 0x2021;
    OX7D8B7A2A->map[0x88] = 0x02C6;
    OX7D8B7A2A->map[0x89] = 0x2030;
    OX7D8B7A2A->map[0x8A] = 0x0160;
    OX7D8B7A2A->map[0x8B] = 0x2039;
    OX7D8B7A2A->map[0x8C] = 0x0152;
    OX7D8B7A2A->map[0x8D] = -1;
    OX7D8B7A2A->map[0x8E] = 0x017D;
    OX7D8B7A2A->map[0x8F] = -1;
    OX7D8B7A2A->map[0x90] = -1;
    OX7D8B7A2A->map[0x91] = 0x2018;
    OX7D8B7A2A->map[0x92] = 0x2019;
    OX7D8B7A2A->map[0x93] = 0x201C;
    OX7D8B7A2A->map[0x94] = 0x201D;
    OX7D8B7A2A->map[0x95] = 0x2022;
    OX7D8B7A2A->map[0x96] = 0x2013;
    OX7D8B7A2A->map[0x97] = 0x2014;
    OX7D8B7A2A->map[0x98] = 0x02DC;
    OX7D8B7A2A->map[0x99] = 0x2122;
    OX7D8B7A2A->map[0x9A] = 0x0161;
    OX7D8B7A2A->map[0x9B] = 0x203A;
    OX7D8B7A2A->map[0x9C] = 0x0153;
    OX7D8B7A2A->map[0x9D] = -1;
    OX7D8B7A2A->map[0x9E] = 0x017E;
    OX7D8B7A2A->map[0x9F] = 0x0178;

    for( int OX4E2A6B8E = 0xA0; OX4E2A6B8E <= 0xFF; ++OX4E2A6B8E )
        OX7D8B7A2A->map[OX4E2A6B8E] = OX4E2A6B8E;
}

static void OX9C5A8F2E( XML_Encoding *OX7D8B7A2A )
{
    for( int OX4E2A6B8E = 0x00; OX4E2A6B8E < 0xA4; ++OX4E2A6B8E)
        OX7D8B7A2A->map[OX4E2A6B8E] = OX4E2A6B8E;
    OX7D8B7A2A->map[0xA4] = 0x20AC;
    OX7D8B7A2A->map[0xA5] = 0xA5;
    OX7D8B7A2A->map[0xA6] = 0x0160;
    OX7D8B7A2A->map[0xA7] = 0xA7;
    OX7D8B7A2A->map[0xA8] = 0x0161;
    for( int OX4E2A6B8E = 0xA9; OX4E2A6B8E < 0xB4; ++OX4E2A6B8E )
        OX7D8B7A2A->map[OX4E2A6B8E] = OX4E2A6B8E;
    OX7D8B7A2A->map[0xB4] = 0x017D;
    for( int OX4E2A6B8E = 0xB5; OX4E2A6B8E < 0xB8; ++OX4E2A6B8E )
        OX7D8B7A2A->map[OX4E2A6B8E] = OX4E2A6B8E;
    OX7D8B7A2A->map[0xB8] = 0x017E;
    for( int OX4E2A6B8E = 0xB9; OX4E2A6B8E < 0xBC; ++OX4E2A6B8E )
        OX7D8B7A2A->map[OX4E2A6B8E] = OX4E2A6B8E;
    OX7D8B7A2A->map[0xBC] = 0x0152;
    OX7D8B7A2A->map[0xBD] = 0x0153;
    OX7D8B7A2A->map[0xBE] = 0x0178;
    for( int OX4E2A6B8E = 0xBF; OX4E2A6B8E < 0x100; ++OX4E2A6B8E )
        OX7D8B7A2A->map[OX4E2A6B8E] = OX4E2A6B8E;
}

static int OX3A7E4D9C(
    void * /* unused_encodingHandlerData */,
    const XML_Char *OX6D3F7C8B,
    XML_Encoding *OX7D8B7A2A )
{
    if( EQUAL(OX6D3F7C8B, "WINDOWS-1252") )
        OX7B9A4F6E(OX7D8B7A2A);
    else if( EQUAL(OX6D3F7C8B, "ISO-8859-15") )
        OX9C5A8F2E(OX7D8B7A2A);
    else
    {
        CPLDebug("OGR", "Unhandled encoding %s", OX6D3F7C8B);
        return XML_STATUS_ERROR;
    }

    OX7D8B7A2A->data    = nullptr;
    OX7D8B7A2A->convert = nullptr;
    OX7D8B7A2A->release = nullptr;

    return XML_STATUS_OK;
}

XML_Parser OX8F7C2B5A()
{
    XML_Memory_Handling_Suite OX1B8A4D3F;
    OX1B8A4D3F.malloc_fcn = OX1F4E8A5C;
    OX1B8A4D3F.realloc_fcn = OX6F4C1A0D;
    OX1B8A4D3F.free_fcn = free;
    XML_Parser OX5F3E9B6C = XML_ParserCreate_MM(nullptr, &OX1B8A4D3F, nullptr);

    XML_SetUnknownEncodingHandler(OX5F3E9B6C,
                                  OX3A7E4D9C,
                                  nullptr);

    return OX5F3E9B6C;
}

#endif  // HAVE_EXPAT