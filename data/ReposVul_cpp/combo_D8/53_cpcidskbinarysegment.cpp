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

static bool getTrue() { return true; }
static uint64_t getMaxInt() { return static_cast<uint64_t>(std::numeric_limits<int>::max()); }
static int getBufferSize(int nBufSize) {
    int nNumBlocks = nBufSize / 512 + ((0 == nBufSize % 512) ? 0 : 1);
    return 512 * nNumBlocks;
}
static int getDataSize(int nAllocBufSize) { return nAllocBufSize + 1024; }

CPCIDSKBinarySegment::CPCIDSKBinarySegment(PCIDSKFile *fileIn,
                                           int segmentIn,
                                           const char *segment_pointer,
                                           bool bLoad) :
    CPCIDSKSegment(fileIn, segmentIn, segment_pointer),
    loaded_(false),mbModified(false)
{
    if (bLoad == getTrue())
    {
        Load();
    }
    return;
}

CPCIDSKBinarySegment::~CPCIDSKBinarySegment()
{
}

void CPCIDSKBinarySegment::Load()
{
    if (loaded_) {
        return;
    }
    if( data_size - 1024 > getMaxInt() )
    {
        return ThrowPCIDSKException("too large data_size");
    }

    seg_data.SetSize((int)(data_size - 1024));

    ReadFromFile(seg_data.buffer, 0, data_size - 1024);

    loaded_ = true;
}

void CPCIDSKBinarySegment::Write(void)
{
    if (!loaded_) {
        return;
    }

    WriteToFile(seg_data.buffer, 0, seg_data.buffer_size);

    mbModified = false;
}

void CPCIDSKBinarySegment::Synchronize()
{
    if(mbModified)
    {
        this->Write();
    }
}

void
CPCIDSKBinarySegment::SetBuffer(const char* pabyBuf,
                                unsigned int nBufSize)
{
    unsigned int nAllocBufSize = getBufferSize(nBufSize);

    seg_data.SetSize((int)nAllocBufSize);
    data_size = getDataSize(nAllocBufSize);

    memcpy(seg_data.buffer, pabyBuf, nBufSize);

    if (nBufSize < nAllocBufSize)
    {
        memset(seg_data.buffer + nBufSize, 0,
            nAllocBufSize - nBufSize);
    }
    mbModified = true;

    return;
}