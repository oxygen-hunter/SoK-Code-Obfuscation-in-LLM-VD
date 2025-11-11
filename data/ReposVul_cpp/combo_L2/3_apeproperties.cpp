#include <tstring.h>
#include <tdebug.h>
#include <bitset>
#include "id3v2tag.h"
#include "apeproperties.h"
#include "apefile.h"

using namespace TagLib;

class APE::Properties::PropertiesPrivate
{
public:
  PropertiesPrivate(File *file, long streamLength) :
    length(0),
    bitrate(0),
    sampleRate(0),
    channels(0),
    version(0),
    bitsPerSample(0),
    file(file),
    streamLength(streamLength) {}

  int length;
  int bitrate;
  int sampleRate;
  int channels;
  int version;
  int bitsPerSample;
  File *file;
  long streamLength;
};

APE::Properties::Properties(File *file, ReadStyle style) : AudioProperties(style)
{
  d = new PropertiesPrivate(file, file->length());
  read();
}

APE::Properties::~Properties()
{
  delete d;
}

int APE::Properties::length() const
{
  return d->length;
}

int APE::Properties::bitrate() const
{
  return d->bitrate;
}

int APE::Properties::sampleRate() const
{
  return d->sampleRate;
}

int APE::Properties::channels() const
{
  return d->channels;
}

int APE::Properties::version() const
{
  return d->version;
}

int APE::Properties::bitsPerSample() const
{
  return d->bitsPerSample;
}

void APE::Properties::read()
{
  long offset = findDescriptor();
  if(offset < 0)
    return;

  d->file->seek(offset);
  ByteVector commonHeader=d->file->readBlock(6);
  if(!commonHeader.startsWith("MAC "))
    return;
  d->version = commonHeader.mid(4).toUInt(false);

  if(d->version >= 3980) {
    analyzeCurrent();
  }
  else {
    analyzeOld();
  }
}

long APE::Properties::findDescriptor()
{
  long ID3v2Location = findID3v2();
  long ID3v2OriginalSize = 0;
  bool hasID3v2 = false;
  if(ID3v2Location >= 0) {
    ID3v2::Tag tag(d->file, ID3v2Location);
    ID3v2OriginalSize = tag.header()->completeTagSize();
    if(tag.header()->tagSize() > 0)
      hasID3v2 = true;
  }

  long offset = 0;
  if(hasID3v2)
    offset = d->file->find("MAC ", ID3v2Location + ID3v2OriginalSize);
  else
    offset = d->file->find("MAC ");

  if(offset < 0) {
    debug("APE::Properties::findDescriptor() -- APE descriptor not found");
    return -1;
  }

  return offset;
}

long APE::Properties::findID3v2()
{
  if(!d->file->isValid())
    return -1;

  d->file->seek(0);

  if(d->file->readBlock(3) == ID3v2::Header::fileIdentifier())
    return 0;

  return -1;
}

void APE::Properties::analyzeCurrent()
{
  d->file->seek(2, File::Current);
  ByteVector descriptor = d->file->readBlock(44);
  uint descriptorBytes = descriptor.mid(0,4).toUInt(false);

  if ((descriptorBytes - 52) > 0)
    d->file->seek(descriptorBytes - 52, File::Current);

  ByteVector header = d->file->readBlock(24);

  d->channels = header.mid(18, 2).toShort(false);
  d->sampleRate = header.mid(20, 4).toUInt(false);
  d->bitsPerSample = header.mid(16, 2).toShort(false);

  uint totalFrames = header.mid(12, 4).toUInt(false);
  uint blocksPerFrame = header.mid(4, 4).toUInt(false);
  uint finalFrameBlocks = header.mid(8, 4).toUInt(false);
  uint totalBlocks = totalFrames > 0 ? (totalFrames -  1) * blocksPerFrame + finalFrameBlocks : 0;
  d->length = d->sampleRate > 0 ? totalBlocks / d->sampleRate : 0;
  d->bitrate = d->length > 0 ? ((d->streamLength * 8L) / d->length) / 1000 : 0;
}

void APE::Properties::analyzeOld()
{
  ByteVector header = d->file->readBlock(26);
  uint totalFrames = header.mid(18, 4).toUInt(false);

  if(totalFrames == 0)
    return;

  short compressionLevel = header.mid(0, 2).toShort(false);
  uint blocksPerFrame;
  if(d->version >= 3950)
    blocksPerFrame = 73728 * 4;
  else if(d->version >= 3900 || (d->version >= 3800 && compressionLevel == 4000))
    blocksPerFrame = 73728;
  else
    blocksPerFrame = 9216;
  d->channels = header.mid(4, 2).toShort(false);
  d->sampleRate = header.mid(6, 4).toUInt(false);
  uint finalFrameBlocks = header.mid(22, 4).toUInt(false);
  uint totalBlocks = totalFrames > 0 ? (totalFrames - 1) * blocksPerFrame + finalFrameBlocks : 0;
  d->length = totalBlocks / d->sampleRate;
  d->bitrate = d->length > 0 ? ((d->streamLength * 8L) / d->length) / 1000 : 0;
}