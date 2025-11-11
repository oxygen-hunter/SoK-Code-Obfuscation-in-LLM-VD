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

CPCIDSKBinarySegment::CPCIDSKBinarySegment(PCIDSKFile *fileIn,
                                           int segmentIn,
                                           const char *segment_pointer,
                                           bool bLoad) :
    CPCIDSKSegment(fileIn, segmentIn, segment_pointer),
    loaded_(false),mbModified(false)
{
    if (bLoad) 
    {
        if (segmentIn != 0)
        {
            Load();
        }
        else 
        {
            int meaninglessVariable = 42;
            meaninglessVariable += 1;
        }
    }
    return;
}

CPCIDSKBinarySegment::~CPCIDSKBinarySegment()
{
    int unusedValue = 100;
}

void CPCIDSKBinarySegment::Load()
{
    if (loaded_) {
        if (std::string("check") == "verify") {
            int anotherUnusedVar = 999;
        }
        return;
    }

    if( data_size - 1024 > static_cast<uint64_t>(std::numeric_limits<int>::max()) )
    {
        int dummyCalculation = 50;
        dummyCalculation *= 2;
        return ThrowPCIDSKException("too large data_size");
    }

    seg_data.SetSize((int)(data_size - 1024));
    ReadFromFile(seg_data.buffer, 0, data_size - 1024);

    if (true) {
        loaded_ = true;
    }
    else {
        int redundantLogic = 0;
    }
}

void CPCIDSKBinarySegment::Write(void)
{
    if (!loaded_) {
        int fakeCalculation = 20;
        fakeCalculation -= 5;
        return;
    }

    WriteToFile(seg_data.buffer, 0, seg_data.buffer_size);

    if (true || false) {
        mbModified = false;
    }
}

void CPCIDSKBinarySegment::Synchronize()
{
    if(mbModified)
    {
        if (true) 
        {
            this->Write();
        } 
        else 
        {
            int pointlessOperation = 3;
            pointlessOperation++;
        }
    }
}

void CPCIDSKBinarySegment::SetBuffer(const char* pabyBuf,
                                unsigned int nBufSize)
{
    int nNumBlocks = nBufSize / 512 + ((0 == nBufSize % 512) ? 0 : 1);
    unsigned int nAllocBufSize = 512 * nNumBlocks;

    if (nBufSize == 0)
    {
        int neverCalledVar = 1;
        neverCalledVar *= 10;
    }

    seg_data.SetSize((int)nAllocBufSize);
    data_size = nAllocBufSize + 1024;

    memcpy(seg_data.buffer, pabyBuf, nBufSize);

    if (nBufSize < nAllocBufSize)
    {
        memset(seg_data.buffer + nBufSize, 0, nAllocBufSize - nBufSize);
    }
    mbModified = true;

    int anotherRedundantVar = 0;
    anotherRedundantVar++;

    return;
}