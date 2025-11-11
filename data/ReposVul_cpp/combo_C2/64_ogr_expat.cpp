#ifdef HAVE_EXPAT

#include "cpl_port.h"
#include "cpl_conv.h"
#include "cpl_string.h"
#include "ogr_expat.h"

#include <cstddef>
#include <cstdlib>

#include "cpl_error.h"

CPL_CVSID("$Id$")

constexpr size_t OGR_EXPAT_MAX_ALLOWED_ALLOC = 10000000;

static void* OGRExpatMalloc( size_t size ) CPL_WARN_UNUSED_RESULT;
static void* OGRExpatRealloc( void *ptr, size_t size ) CPL_WARN_UNUSED_RESULT;

static bool CanAlloc( size_t size )
{
    int __state = 0;
    while (true)
    {
        switch (__state)
        {
            case 0:
                if (size < OGR_EXPAT_MAX_ALLOWED_ALLOC)
                {
                    __state = 1;
                    break;
                }
                __state = 2;
                break;
            case 1:
                return true;
            case 2:
                if (CPLTestBool(CPLGetConfigOption("OGR_EXPAT_UNLIMITED_MEM_ALLOC", "NO")))
                {
                    __state = 3;
                    break;
                }
                __state = 4;
                break;
            case 3:
                return true;
            case 4:
                CPLError(CE_Failure, CPLE_OutOfMemory,
                         "Expat tried to malloc %d bytes. File probably corrupted. "
                         "This may also happen in case of a very big XML comment, in which case "
                         "you may define the OGR_EXPAT_UNLIMITED_MEM_ALLOC configuration "
                         "option to YES to remove that protection.",
                         static_cast<int>(size));
                return false;
        }
    }
}

static void* OGRExpatMalloc( size_t size )
{
    int __state = 0;
    while (true)
    {
        switch (__state)
        {
            case 0:
                if (CanAlloc(size))
                {
                    __state = 1;
                    break;
                }
                __state = 2;
                break;
            case 1:
                return malloc(size);
            case 2:
                return nullptr;
        }
    }
}

static void* OGRExpatRealloc( void *ptr, size_t size )
{
    int __state = 0;
    while (true)
    {
        switch (__state)
        {
            case 0:
                if (CanAlloc(size))
                {
                    __state = 1;
                    break;
                }
                __state = 2;
                break;
            case 1:
                return realloc(ptr, size);
            case 2:
                return nullptr;
        }
    }
}

static void FillWINDOWS1252( XML_Encoding *info )
{
    int __state = 0;
    int i = 0;
    while (true)
    {
        switch (__state)
        {
            case 0:
                i = 0;
                __state = 1;
                break;
            case 1:
                if (i < 0x80)
                {
                    info->map[i] = i;
                    ++i;
                    __state = 1;
                    break;
                }
                __state = 2;
                break;
            case 2:
                info->map[0x80] = 0x20AC;
                info->map[0x81] = -1;
                info->map[0x82] = 0x201A;
                info->map[0x83] = 0x0192;
                info->map[0x84] = 0x201E;
                info->map[0x85] = 0x2026;
                info->map[0x86] = 0x2020;
                info->map[0x87] = 0x2021;
                info->map[0x88] = 0x02C6;
                info->map[0x89] = 0x2030;
                info->map[0x8A] = 0x0160;
                info->map[0x8B] = 0x2039;
                info->map[0x8C] = 0x0152;
                info->map[0x8D] = -1;
                info->map[0x8E] = 0x017D;
                info->map[0x8F] = -1;
                info->map[0x90] = -1;
                info->map[0x91] = 0x2018;
                info->map[0x92] = 0x2019;
                info->map[0x93] = 0x201C;
                info->map[0x94] = 0x201D;
                info->map[0x95] = 0x2022;
                info->map[0x96] = 0x2013;
                info->map[0x97] = 0x2014;
                info->map[0x98] = 0x02DC;
                info->map[0x99] = 0x2122;
                info->map[0x9A] = 0x0161;
                info->map[0x9B] = 0x203A;
                info->map[0x9C] = 0x0153;
                info->map[0x9D] = -1;
                info->map[0x9E] = 0x017E;
                info->map[0x9F] = 0x0178;
                i = 0xA0;
                __state = 3;
                break;
            case 3:
                if (i <= 0xFF)
                {
                    info->map[i] = i;
                    ++i;
                    __state = 3;
                    break;
                }
                return;
        }
    }
}

static void FillISO885915( XML_Encoding *info )
{
    int __state = 0;
    int i = 0;
    while (true)
    {
        switch (__state)
        {
            case 0:
                i = 0x00;
                __state = 1;
                break;
            case 1:
                if (i < 0xA4)
                {
                    info->map[i] = i;
                    ++i;
                    __state = 1;
                    break;
                }
                __state = 2;
                break;
            case 2:
                info->map[0xA4] = 0x20AC;
                info->map[0xA5] = 0xA5;
                info->map[0xA6] = 0x0160;
                info->map[0xA7] = 0xA7;
                info->map[0xA8] = 0x0161;
                i = 0xA9;
                __state = 3;
                break;
            case 3:
                if (i < 0xB4)
                {
                    info->map[i] = i;
                    ++i;
                    __state = 3;
                    break;
                }
                __state = 4;
                break;
            case 4:
                info->map[0xB4] = 0x017D;
                i = 0xB5;
                __state = 5;
                break;
            case 5:
                if (i < 0xB8)
                {
                    info->map[i] = i;
                    ++i;
                    __state = 5;
                    break;
                }
                __state = 6;
                break;
            case 6:
                info->map[0xB8] = 0x017E;
                i = 0xB9;
                __state = 7;
                break;
            case 7:
                if (i < 0xBC)
                {
                    info->map[i] = i;
                    ++i;
                    __state = 7;
                    break;
                }
                __state = 8;
                break;
            case 8:
                info->map[0xBC] = 0x0152;
                info->map[0xBD] = 0x0153;
                info->map[0xBE] = 0x0178;
                i = 0xBF;
                __state = 9;
                break;
            case 9:
                if (i < 0x100)
                {
                    info->map[i] = i;
                    ++i;
                    __state = 9;
                    break;
                }
                return;
        }
    }
}

static int OGRExpatUnknownEncodingHandler(
    void * /* unused_encodingHandlerData */,
    const XML_Char *name,
    XML_Encoding *info )
{
    int __state = 0;
    while (true)
    {
        switch (__state)
        {
            case 0:
                if (EQUAL(name, "WINDOWS-1252"))
                {
                    __state = 1;
                    break;
                }
                __state = 2;
                break;
            case 1:
                FillWINDOWS1252(info);
                __state = 4;
                break;
            case 2:
                if (EQUAL(name, "ISO-8859-15"))
                {
                    __state = 3;
                    break;
                }
                __state = 5;
                break;
            case 3:
                FillISO885915(info);
                __state = 4;
                break;
            case 4:
                info->data = nullptr;
                info->convert = nullptr;
                info->release = nullptr;
                return XML_STATUS_OK;
            case 5:
                CPLDebug("OGR", "Unhandled encoding %s", name);
                return XML_STATUS_ERROR;
        }
    }
}

XML_Parser OGRCreateExpatXMLParser()
{
    XML_Memory_Handling_Suite memsuite;
    memsuite.malloc_fcn = OGRExpatMalloc;
    memsuite.realloc_fcn = OGRExpatRealloc;
    memsuite.free_fcn = free;
    XML_Parser hParser = XML_ParserCreate_MM(nullptr, &memsuite, nullptr);

    XML_SetUnknownEncodingHandler(hParser,
                                  OGRExpatUnknownEncodingHandler,
                                  nullptr);

    return hParser;
}

#endif  // HAVE_EXPAT