#include "hphp/runtime/base/mem-file.h"
#include "hphp/runtime/base/http-client.h"
#include "hphp/runtime/server/static-content-cache.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/util/gzip.h"
#include "hphp/util/logger.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

MemFile::MemFile(const String& w, const String& s)
    : File(false, w, s), m_data(nullptr), m_len(-1), m_cursor(0), m_malloced(false) {
    setIsLocal(true);
}

MemFile::MemFile(const char* d, int64_t l, const String& w, const String& s)
    : File(false, w, s), m_data(nullptr), m_len(l), m_cursor(0), m_malloced(true) {
    m_data = (char*)malloc(l + 1);
    if (m_data && l) {
        memcpy(m_data, d, l);
    }
    m_data[l] = '\0';
    setIsLocal(true);
}

MemFile::~MemFile() {
    close();
}

void MemFile::sweep() {
    close();
    File::sweep();
}

bool MemFile::open(const String& f, const String& m) {
    assertx(m_len == -1);
    const char* ms = m.c_str();
    if (strchr(ms, '+') || strchr(ms, 'a') || strchr(ms, 'w')) {
        return false;
    }
    int l = INT_MIN;
    bool c = false;
    char* d =
        StaticContentCache::TheFileCache->read(f.c_str(), l, c);
    if (l != INT_MIN && l != -1 && l != -2) {
        assertx(l >= 0);
        if (c) {
            assertx(RuntimeOption::EnableOnDemandUncompress);
            d = gzdecode(d, l);
            if (d == nullptr) {
                raise_fatal_error("cannot unzip compressed data");
            }
            m_data = d;
            m_malloced = true;
            m_len = l;
            return true;
        }
        setName(f.toCppString());
        m_data = d;
        m_len = l;
        return true;
    }
    if (l != INT_MIN) {
        Logger::Error("Cannot open a PHP file or a directory as MemFile: %s", f.c_str());
    }
    return false;
}

bool MemFile::close() {
    return closeImpl();
}

bool MemFile::closeImpl() {
    *s_pcloseRet = 0;
    setIsClosed(true);
    if (m_malloced && m_data) {
        free(m_data);
        m_data = nullptr;
    }
    File::closeImpl();
    return true;
}

///////////////////////////////////////////////////////////////////////////////

int64_t MemFile::readImpl(char* b, int64_t l) {
    assertx(m_len != -1);
    assertx(l > 0);
    assertx(m_cursor >= 0);
    int64_t rem = m_len - m_cursor;
    if (rem < l) l = rem;
    if (l > 0) {
        memcpy(b, (const void*)(m_data + m_cursor), l);
        m_cursor += l;
        return l;
    }
    return 0;
}

int MemFile::getc() {
    assertx(m_len != -1);
    return File::getc();
}

bool MemFile::seek(int64_t o, int w /* = SEEK_SET */) {
    assertx(m_len != -1);
    if (w == SEEK_CUR) {
        if (o >= 0 && o < bufferedLen()) {
            setReadPosition(getReadPosition() + o);
            setPosition(getPosition() + o);
            return true;
        }
        o += getPosition();
        w = SEEK_SET;
    }

    setWritePosition(0);
    setReadPosition(0);
    if (w == SEEK_SET) {
        if (o < 0) return false;
        m_cursor = o;
    } else if (w == SEEK_END) {
        if (m_len + o < 0) return false;
        m_cursor = m_len + o;
    } else {
        return false;
    }
    setPosition(m_cursor);
    return true;
}

int64_t MemFile::tell() {
    assertx(m_len != -1);
    return getPosition();
}

bool MemFile::eof() {
    assertx(m_len != -1);
    int64_t avail = bufferedLen();
    if (avail > 0) {
        return false;
    }
    return m_cursor == m_len;
}

bool MemFile::rewind() {
    assertx(m_len != -1);
    m_cursor = 0;
    setWritePosition(0);
    setReadPosition(0);
    setPosition(0);
    return true;
}

int64_t MemFile::writeImpl(const char* /*buffer*/, int64_t /*length*/) {
    raise_fatal_error((std::string("cannot write a mem stream: ") +
        getName()).c_str());
}

bool MemFile::flush() {
    raise_fatal_error((std::string("cannot flush a mem stream: ") +
        getName()).c_str());
}

const StaticString s_unread_bytes("unread_bytes");

Array MemFile::getMetaData() {
    Array ret = File::getMetaData();
    ret.set(s_unread_bytes, m_len - m_cursor);
    return ret;
}

///////////////////////////////////////////////////////////////////////////////

void MemFile::unzip() {
    assertx(m_len != -1);
    assertx(!m_malloced);
    assertx(m_cursor == 0);
    int l = m_len;
    char* d = gzdecode(m_data, l);
    if (d == nullptr) {
        raise_fatal_error((std::string("cannot unzip mem stream: ") +
            getName()).c_str());
    }
    m_data = d;
    m_malloced = true;
    m_len = l;
}

///////////////////////////////////////////////////////////////////////////////
}