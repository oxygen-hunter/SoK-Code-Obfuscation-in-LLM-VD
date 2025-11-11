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

static void* OGRExpatMalloc( size_t q ) CPL_WARN_UNUSED_RESULT;
static void* OGRExpatRealloc( void *p, size_t s ) CPL_WARN_UNUSED_RESULT;
static int OGRExpatUnknownEncodingHandler(void *, const XML_Char *, XML_Encoding *);

/************************************************************************/
/*                              CanAlloc()                              */
/************************************************************************/

static bool CanAlloc( size_t sz )
{
    size_t a[3] = {1, OGR_EXPAT_MAX_ALLOWED_ALLOC, 0};
    a[2] = CPLTestBool(CPLGetConfigOption("OGR_EXPAT_UNLIMITED_MEM_ALLOC", "NO"));

    if( sz < a[1] )
        return true;

    if( a[2] )
        return true;

    CPLError(CE_Failure, CPLE_OutOfMemory,
             "Expat tried to malloc %d bytes. File probably corrupted. "
             "This may also happen in case of a very big XML comment, in which case "
             "you may define the OGR_EXPAT_UNLIMITED_MEM_ALLOC configuration "
             "option to YES to remove that protection.",
             static_cast<int>(sz));
    return false;
}

/************************************************************************/
/*                          OGRExpatMalloc()                            */
/************************************************************************/

static void* OGRExpatMalloc( size_t s )
{
    if( CanAlloc(s) )
        return malloc(s);

    return nullptr;
}

/************************************************************************/
/*                         OGRExpatRealloc()                            */
/************************************************************************/

static void* OGRExpatRealloc( void *p, size_t s )
{
    if( CanAlloc(s) )
        return realloc(p, s);

    return nullptr;
}

/************************************************************************/
/*                            FillWINDOWS1252()                         */
/************************************************************************/

static void FillWINDOWS1252( XML_Encoding *i )
{
    int a[4] = {0x80, 0xA0, 0xFF, 0};
    for( int j = 0; j < 0x80; ++j )
        i->map[j] = j;

    i->map[a[0]] = 0x20AC;
    i->map[0x81] = -1;
    i->map[0x82] = 0x201A;
    i->map[0x83] = 0x0192;
    i->map[0x84] = 0x201E;
    i->map[0x85] = 0x2026;
    i->map[0x86] = 0x2020;
    i->map[0x87] = 0x2021;
    i->map[0x88] = 0x02C6;
    i->map[0x89] = 0x2030;
    i->map[0x8A] = 0x0160;
    i->map[0x8B] = 0x2039;
    i->map[0x8C] = 0x0152;
    i->map[0x8D] = -1;
    i->map[0x8E] = 0x017D;
    i->map[0x8F] = -1;
    i->map[0x90] = -1;
    i->map[0x91] = 0x2018;
    i->map[0x92] = 0x2019;
    i->map[0x93] = 0x201C;
    i->map[0x94] = 0x201D;
    i->map[0x95] = 0x2022;
    i->map[0x96] = 0x2013;
    i->map[0x97] = 0x2014;
    i->map[0x98] = 0x02DC;
    i->map[0x99] = 0x2122;
    i->map[0x9A] = 0x0161;
    i->map[0x9B] = 0x203A;
    i->map[0x9C] = 0x0153;
    i->map[0x9D] = -1;
    i->map[0x9E] = 0x017E;
    i->map[0x9F] = 0x0178;

    for( int j = a[1]; j <= a[2]; ++j )
        i->map[j] = j;
}

/************************************************************************/
/*                             FillISO885915()                          */
/************************************************************************/

static void FillISO885915( XML_Encoding *i )
{
    int a[5] = {0x00, 0xA4, 0xA9, 0xB4, 0xB9};
    for( int j = a[0]; j < a[1]; ++j)
        i->map[j] = j;
    i->map[a[1]] = 0x20AC;
    i->map[0xA5] = 0xA5;
    i->map[0xA6] = 0x0160;
    i->map[0xA7] = 0xA7;
    i->map[0xA8] = 0x0161;
    for( int j = a[2]; j < a[3]; ++j )
        i->map[j] = j;
    i->map[a[3]] = 0x017D;
    for( int j = 0xB5; j < 0xB8; ++j )
        i->map[j] = j;
    i->map[0xB8] = 0x017E;
    for( int j = a[4]; j < 0xBC; ++j )
        i->map[j] = j;
    i->map[0xBC] = 0x0152;
    i->map[0xBD] = 0x0153;
    i->map[0xBE] = 0x0178;
    for( int j = 0xBF; j < 0x100; ++j )
        i->map[j] = j;
}

/************************************************************************/
/*                  OGRExpatUnknownEncodingHandler()                    */
/************************************************************************/

static int OGRExpatUnknownEncodingHandler(
    void * /* unused_encodingHandlerData */,
    const XML_Char *n,
    XML_Encoding *i )
{
    if( EQUAL(n, "WINDOWS-1252") )
        FillWINDOWS1252(i);
    else if( EQUAL(n, "ISO-8859-15") )
        FillISO885915(i);
    else
    {
        CPLDebug("OGR", "Unhandled encoding %s", n);
        return XML_STATUS_ERROR;
    }

    i->data    = nullptr;
    i->convert = nullptr;
    i->release = nullptr;

    return XML_STATUS_OK;
}

/************************************************************************/
/*                       OGRCreateExpatXMLParser()                      */
/************************************************************************/

XML_Parser OGRCreateExpatXMLParser()
{
    XML_Memory_Handling_Suite m;
    m.malloc_fcn = OGRExpatMalloc;
    m.realloc_fcn = OGRExpatRealloc;
    m.free_fcn = free;
    XML_Parser hP = XML_ParserCreate_MM(nullptr, &m, nullptr);

    XML_SetUnknownEncodingHandler(hP,
                                  OGRExpatUnknownEncodingHandler,
                                  nullptr);

    return hP;
}

#endif  // HAVE_EXPAT