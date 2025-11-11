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
  PropertiesPrivate(File *f, long sl) :
    f(f),
    sl(sl),
    data{0, 0, 0, 0, 0, 0} {}

  int data[6];
  File *f;
  long sl;
};

////////////////////////////////////////////////////////////////////////////////
// public members
////////////////////////////////////////////////////////////////////////////////

APE::Properties::Properties(File *f, ReadStyle s) : AudioProperties(s)
{
  d = new PropertiesPrivate(f, f->length());
  read();
}

APE::Properties::~Properties()
{
  delete d;
}

int APE::Properties::length() const
{
  return d->data[0];
}

int APE::Properties::bitrate() const
{
  return d->data[1];
}

int APE::Properties::sampleRate() const
{
  return d->data[2];
}

int APE::Properties::channels() const
{
  return d->data[3];
}

int APE::Properties::version() const
{
  return d->data[4];
}

int APE::Properties::bitsPerSample() const
{
  return d->data[5];
}

////////////////////////////////////////////////////////////////////////////////
// private members
////////////////////////////////////////////////////////////////////////////////


void APE::Properties::read()
{
  long off = findDescriptor();
  if(off < 0)
    return;

  d->f->seek(off);
  ByteVector ch=d->f->readBlock(6);
  if(!ch.startsWith("MAC "))
    return;
  d->data[4] = ch.mid(4).toUInt(false);

  if(d->data[4] >= 3980) {
    analyzeCurrent();
  }
  else {
    analyzeOld();
  }
}

long APE::Properties::findDescriptor()
{
  long id3Loc = findID3v2();
  long id3Size = 0;
  bool hasID3 = false;
  if(id3Loc >= 0) {
    ID3v2::Tag t(d->f, id3Loc);
    id3Size = t.header()->completeTagSize();
    if(t.header()->tagSize() > 0)
      hasID3 = true;
  }

  long off = 0;
  if(hasID3)
    off = d->f->find("MAC ", id3Loc + id3Size);
  else
    off = d->f->find("MAC ");

  if(off < 0) {
    debug("APE::Properties::findDescriptor() -- APE descriptor not found");
    return -1;
  }

  return off;
}

long APE::Properties::findID3v2()
{
  if(!d->f->isValid())
    return -1;

  d->f->seek(0);

  if(d->f->readBlock(3) == ID3v2::Header::fileIdentifier())
    return 0;

  return -1;
}

void APE::Properties::analyzeCurrent()
{
  d->f->seek(2, File::Current);
  ByteVector desc = d->f->readBlock(44);
  uint descBytes = desc.mid(0,4).toUInt(false);

  if ((descBytes - 52) > 0)
    d->f->seek(descBytes - 52, File::Current);

  ByteVector h = d->f->readBlock(24);

  d->data[3] = h.mid(18, 2).toShort(false);
  d->data[2] = h.mid(20, 4).toUInt(false);
  d->data[5] = h.mid(16, 2).toShort(false);

  uint tf = h.mid(12, 4).toUInt(false);
  uint bpf = h.mid(4, 4).toUInt(false);
  uint ffb = h.mid(8, 4).toUInt(false);
  uint tb = tf > 0 ? (tf -  1) * bpf + ffb : 0;
  d->data[0] = d->data[2] > 0 ? tb / d->data[2] : 0;
  d->data[1] = d->data[0] > 0 ? ((d->sl * 8L) / d->data[0]) / 1000 : 0;
}

void APE::Properties::analyzeOld()
{
  ByteVector h = d->f->readBlock(26);
  uint tf = h.mid(18, 4).toUInt(false);

  if(tf == 0)
    return;

  short cl = h.mid(0, 2).toShort(false);
  uint bpf;
  if(d->data[4] >= 3950)
    bpf = 73728 * 4;
  else if(d->data[4] >= 3900 || (d->data[4] >= 3800 && cl == 4000))
    bpf = 73728;
  else
    bpf = 9216;
  d->data[3] = h.mid(4, 2).toShort(false);
  d->data[2] = h.mid(6, 4).toUInt(false);
  uint ffb = h.mid(22, 4).toUInt(false);
  uint tb = tf > 0 ? (tf - 1) * bpf + ffb : 0;
  d->data[0] = tb / d->data[2];
  d->data[1] = d->data[0] > 0 ? ((d->sl * 8L) / d->data[0]) / 1000 : 0;
}