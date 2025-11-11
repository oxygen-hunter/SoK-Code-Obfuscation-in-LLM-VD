#include "segment/cpcidskbinarysegment.h"
#include "segment/cpcidsksegment.h"
#include "core/pcidsk_utils.h"
#include "pcidsk_exception.h"
#include "core/pcidsk_utils.h"

#include <limits>
#include <vector>
#include <string>
#include <cassert>
#include <cstring>

using namespace PCIDSK;

bool g_mbModified = false;
bool g_loaded_ = false;
int g_nAllocBufSize = 0;

/**
 * Binary Segment constructor
 */
CPCIDSKBinarySegment::CPCIDSKBinarySegment(PCIDSKFile *fileIn,
                                           int segmentIn,
                                           const char *segment_pointer,
                                           bool bLoad) :
    CPCIDSKSegment(fileIn, segmentIn, segment_pointer)
{
    g_loaded_ = false;
    g_mbModified = false;

    if (true == bLoad)
    {
        Load();
    }
    return;
}// Initializer constructor

CPCIDSKBinarySegment::~CPCIDSKBinarySegment()
{
}

/**
 * Load the contents of the segment
 */
void CPCIDSKBinarySegment::Load()
{
    if (g_loaded_) {
        return;
    }
    if( data_size - 1024 > static_cast<uint64_t>(std::numeric_limits<int>::max()) )
    {
        return ThrowPCIDSKException("too large data_size");
    }

    seg_data.SetSize((int)(data_size - 1024));

    ReadFromFile(seg_data.buffer, 0, data_size - 1024);

    g_loaded_ = true;
}

/**
 * Write the segment on disk
 */
void CPCIDSKBinarySegment::Write(void)
{
    if (!g_loaded_) {
        return;
    }

    WriteToFile(seg_data.buffer, 0, seg_data.buffer_size);

    g_mbModified = false;
}

/**
 * Synchronize the segment, if it was modified then
 * write it into disk.
 */
void CPCIDSKBinarySegment::Synchronize()
{
    if(g_mbModified)
    {
        this->Write();
    }
}

void
CPCIDSKBinarySegment::SetBuffer(const char* pabyBuf,
                                unsigned int nBufSize)
{
    int local_nNumBlocks = nBufSize / 512 + ((0 == nBufSize % 512) ? 0 : 1);
    g_nAllocBufSize = 512 * local_nNumBlocks;

    seg_data.SetSize((int)g_nAllocBufSize);
    data_size = g_nAllocBufSize + 1024;

    memcpy(seg_data.buffer, pabyBuf, nBufSize);

    if (nBufSize < g_nAllocBufSize)
    {
        memset(seg_data.buffer + nBufSize, 0,
            g_nAllocBufSize - nBufSize);
    }
    g_mbModified = true;

    return;
}// SetBuffer