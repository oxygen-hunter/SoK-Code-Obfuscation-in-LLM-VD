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
    int dispatcher = 0;
    while (dispatcher != -1) {
        switch (dispatcher) {
            case 0:
                if (true == bLoad) {
                    dispatcher = 1;
                } else {
                    dispatcher = -1;
                }
                break;
            case 1:
                Load();
                dispatcher = -1;
                break;
        }
    }
    return;
}

CPCIDSKBinarySegment::~CPCIDSKBinarySegment()
{
}

void CPCIDSKBinarySegment::Load()
{
    int dispatcher = 0;
    while (dispatcher != -1) {
        switch (dispatcher) {
            case 0:
                if (loaded_) {
                    dispatcher = -1;
                } else {
                    dispatcher = 1;
                }
                break;
            case 1:
                if( data_size - 1024 > static_cast<uint64_t>(std::numeric_limits<int>::max()) ) {
                    dispatcher = 2;
                } else {
                    dispatcher = 3;
                }
                break;
            case 2:
                return ThrowPCIDSKException("too large data_size");
                dispatcher = -1;
                break;
            case 3:
                seg_data.SetSize((int)(data_size - 1024));
                ReadFromFile(seg_data.buffer, 0, data_size - 1024);
                loaded_ = true;
                dispatcher = -1;
                break;
        }
    }
}

void CPCIDSKBinarySegment::Write(void)
{
    int dispatcher = 0;
    while (dispatcher != -1) {
        switch (dispatcher) {
            case 0:
                if (!loaded_) {
                    dispatcher = -1;
                } else {
                    dispatcher = 1;
                }
                break;
            case 1:
                WriteToFile(seg_data.buffer, 0, seg_data.buffer_size);
                mbModified = false;
                dispatcher = -1;
                break;
        }
    }
}

void CPCIDSKBinarySegment::Synchronize()
{
    int dispatcher = 0;
    while (dispatcher != -1) {
        switch (dispatcher) {
            case 0:
                if(mbModified) {
                    dispatcher = 1;
                } else {
                    dispatcher = -1;
                }
                break;
            case 1:
                this->Write();
                dispatcher = -1;
                break;
        }
    }
}

void CPCIDSKBinarySegment::SetBuffer(const char* pabyBuf,
                                unsigned int nBufSize)
{
    int dispatcher = 0;
    while (dispatcher != -1) {
        switch (dispatcher) {
            case 0:
                int nNumBlocks = nBufSize / 512 + ((0 == nBufSize % 512) ? 0 : 1);
                unsigned int nAllocBufSize = 512 * nNumBlocks;
                seg_data.SetSize((int)nAllocBufSize);
                data_size = nAllocBufSize + 1024;
                memcpy(seg_data.buffer, pabyBuf, nBufSize);
                dispatcher = 1;
                break;
            case 1:
                if (nBufSize < nAllocBufSize) {
                    dispatcher = 2;
                } else {
                    dispatcher = -1;
                }
                break;
            case 2:
                memset(seg_data.buffer + nBufSize, 0, nAllocBufSize - nBufSize);
                mbModified = true;
                dispatcher = -1;
                break;
        }
    }
}