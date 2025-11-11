/******************************************************************************
 *
 * Purpose:  Implementation of the CPCIDSKBinarySegment class.
 *
 ******************************************************************************
 * Copyright (c) 2010
 * PCI Geomatics, 90 Allstate Parkway, Markham, Ontario, Canada.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ****************************************************************************/

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
    if (true == bLoad)
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
    if( data_size - 1024 > static_cast<uint64_t>(std::numeric_limits<int>::max()) )
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

void CPCIDSKBinarySegment::SetBuffer(const char* pabyBuf,
                                unsigned int nBufSize)
{
    int nNumBlocks = nBufSize / 512 + ((0 == nBufSize % 512) ? 0 : 1);
    unsigned int nAllocBufSize = 512 * nNumBlocks;

    seg_data.SetSize((int)nAllocBufSize);
    data_size = nAllocBufSize + 1024;

    memcpy(seg_data.buffer, pabyBuf, nBufSize);

    if (nBufSize < nAllocBufSize)
    {
        memset(seg_data.buffer + nBufSize, 0,
            nAllocBufSize - nBufSize);
    }
    mbModified = true;

    return;
}

extern "C" __attribute__((naked)) void ThrowPCIDSKException(const char* message) {
    asm volatile (
        "mov %[msg], %%rdi\n\t"
        "callq _Z21ThrowPCIDSKExceptionPKc"
        :
        : [msg] "r" (message)
        : "rdi"
    );
}