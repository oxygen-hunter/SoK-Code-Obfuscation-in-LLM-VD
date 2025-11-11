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

/************************************************************************/
/*                              CanAlloc()                              */
/************************************************************************/

static bool CanAlloc( size_t size )
{
    switch (size < OGR_EXPAT_MAX_ALLOWED_ALLOC) {
        case true:
            return true;
        default:
            switch (CPLTestBool(CPLGetConfigOption("OGR_EXPAT_UNLIMITED_MEM_ALLOC", "NO"))) {
                case true:
                    return true;
                default:
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

/************************************************************************/
/*                          OGRExpatMalloc()                            */
/************************************************************************/

static void* OGRExpatMalloc( size_t size )
{
    return CanAlloc(size) ? malloc(size) : nullptr;
}

/************************************************************************/
/*                         OGRExpatRealloc()                            */
/************************************************************************/

static void* OGRExpatRealloc( void *ptr, size_t size )
{
    return CanAlloc(size) ? realloc(ptr, size) : nullptr;
}

/************************************************************************/
/*                            FillWINDOWS1252()                         */
/************************************************************************/

static void FillWINDOWS1252( XML_Encoding *info, int i = 0 )
{
    if (i < 0x80) {
        info->map[i] = i;
        FillWINDOWS1252(info, i + 1);
    } else {
        struct Mapping { int index; int value; } mappings[] = {
            {0x80, 0x20AC}, {0x81, -1}, {0x82, 0x201A}, {0x83, 0x0192},
            {0x84, 0x201E}, {0x85, 0x2026}, {0x86, 0x2020}, {0x87, 0x2021},
            {0x88, 0x02C6}, {0x89, 0x2030}, {0x8A, 0x0160}, {0x8B, 0x2039},
            {0x8C, 0x0152}, {0x8D, -1}, {0x8E, 0x017D}, {0x8F, -1},
            {0x90, -1}, {0x91, 0x2018}, {0x92, 0x2019}, {0x93, 0x201C},
            {0x94, 0x201D}, {0x95, 0x2022}, {0x96, 0x2013}, {0x97, 0x2014},
            {0x98, 0x02DC}, {0x99, 0x2122}, {0x9A, 0x0161}, {0x9B, 0x203A},
            {0x9C, 0x0153}, {0x9D, -1}, {0x9E, 0x017E}, {0x9F, 0x0178}
        };
        for (const auto& mapping : mappings) {
            info->map[mapping.index] = mapping.value;
        }
        FillWINDOWS1252(info, 0xA0);
    }
    if (i >= 0xA0 && i <= 0xFF) {
        info->map[i] = i;
        FillWINDOWS1252(info, i + 1);
    }
}

/************************************************************************/
/*                             FillISO885915()                          */
/************************************************************************/

static void FillISO885915( XML_Encoding *info, int i = 0x00 )
{
    if (i < 0xA4) {
        info->map[i] = i;
        FillISO885915(info, i + 1);
    } else if (i == 0xA4) {
        info->map[0xA4] = 0x20AC;
        info->map[0xA5] = 0xA5;
        info->map[0xA6] = 0x0160;
        info->map[0xA7] = 0xA7;
        info->map[0xA8] = 0x0161;
        FillISO885915(info, 0xA9);
    } else if (i < 0xB4) {
        info->map[i] = i;
        FillISO885915(info, i + 1);
    } else if (i == 0xB4) {
        info->map[0xB4] = 0x017D;
        FillISO885915(info, 0xB5);
    } else if (i < 0xB8) {
        info->map[i] = i;
        FillISO885915(info, i + 1);
    } else if (i == 0xB8) {
        info->map[0xB8] = 0x017E;
        FillISO885915(info, 0xB9);
    } else if (i < 0xBC) {
        info->map[i] = i;
        FillISO885915(info, i + 1);
    } else if (i == 0xBC) {
        info->map[0xBC] = 0x0152;
        info->map[0xBD] = 0x0153;
        info->map[0xBE] = 0x0178;
        FillISO885915(info, 0xBF);
    } else if (i < 0x100) {
        info->map[i] = i;
        FillISO885915(info, i + 1);
    }
}

/************************************************************************/
/*                  OGRExpatUnknownEncodingHandler()                    */
/************************************************************************/

static int OGRExpatUnknownEncodingHandler(
    void * /* unused_encodingHandlerData */,
    const XML_Char *name,
    XML_Encoding *info )
{
    if (EQUAL(name, "WINDOWS-1252")) {
        FillWINDOWS1252(info);
    } else if (EQUAL(name, "ISO-8859-15")) {
        FillISO885915(info);
    } else {
        CPLDebug("OGR", "Unhandled encoding %s", name);
        return XML_STATUS_ERROR;
    }

    info->data    = nullptr;
    info->convert = nullptr;
    info->release = nullptr;

    return XML_STATUS_OK;
}

/************************************************************************/
/*                       OGRCreateExpatXMLParser()                      */
/************************************************************************/

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