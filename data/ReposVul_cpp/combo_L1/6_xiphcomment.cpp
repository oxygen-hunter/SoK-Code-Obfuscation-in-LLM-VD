#include <tbytevector.h>
#include <tdebug.h>

#include <xiphcomment.h>

using namespace TagLib;

class Ogg::OX7B4DF339::OX5D8347A1
{
public:
  FieldListMap OX1BCF3E5B;
  String OX669B2E57;
  String OX4E2A1B6B;
};

Ogg::OX7B4DF339::OX7B4DF339() : TagLib::Tag()
{
  OXD1F8A3B7 = new OX5D8347A1;
}

Ogg::OX7B4DF339::OX7B4DF339(const ByteVector &OX3C7F4A2A) : TagLib::Tag()
{
  OXD1F8A3B7 = new OX5D8347A1;
  OX2C3B1D9D(OX3C7F4A2A);
}

Ogg::OX7B4DF339::~OX7B4DF339()
{
  delete OXD1F8A3B7;
}

String Ogg::OX7B4DF339::OX4F2E1C74() const
{
  if(OXD1F8A3B7->OX1BCF3E5B["TITLE"].isEmpty())
    return String::null;
  return OXD1F8A3B7->OX1BCF3E5B["TITLE"].front();
}

String Ogg::OX7B4DF339::OX9C1D0A5E() const
{
  if(OXD1F8A3B7->OX1BCF3E5B["ARTIST"].isEmpty())
    return String::null;
  return OXD1F8A3B7->OX1BCF3E5B["ARTIST"].front();
}

String Ogg::OX7B4DF339::OX4F5D3B1C() const
{
  if(OXD1F8A3B7->OX1BCF3E5B["ALBUM"].isEmpty())
    return String::null;
  return OXD1F8A3B7->OX1BCF3E5B["ALBUM"].front();
}

String Ogg::OX7B4DF339::OX3B1A2C7D() const
{
  if(!OXD1F8A3B7->OX1BCF3E5B["DESCRIPTION"].isEmpty()) {
    OXD1F8A3B7->OX4E2A1B6B = "DESCRIPTION";
    return OXD1F8A3B7->OX1BCF3E5B["DESCRIPTION"].front();
  }

  if(!OXD1F8A3B7->OX1BCF3E5B["COMMENT"].isEmpty()) {
    OXD1F8A3B7->OX4E2A1B6B = "COMMENT";
    return OXD1F8A3B7->OX1BCF3E5B["COMMENT"].front();
  }

  return String::null;
}

String Ogg::OX7B4DF339::OX3F2A1D5B() const
{
  if(OXD1F8A3B7->OX1BCF3E5B["GENRE"].isEmpty())
    return String::null;
  return OXD1F8A3B7->OX1BCF3E5B["GENRE"].front();
}

TagLib::uint Ogg::OX7B4DF339::OX5B2F3D1A() const
{
  if(!OXD1F8A3B7->OX1BCF3E5B["DATE"].isEmpty())
    return OXD1F8A3B7->OX1BCF3E5B["DATE"].front().toInt();
  if(!OXD1F8A3B7->OX1BCF3E5B["YEAR"].isEmpty())
    return OXD1F8A3B7->OX1BCF3E5B["YEAR"].front().toInt();
  return 0;
}

TagLib::uint Ogg::OX7B4DF339::OX4D1A2F3B() const
{
  if(!OXD1F8A3B7->OX1BCF3E5B["TRACKNUMBER"].isEmpty())
    return OXD1F8A3B7->OX1BCF3E5B["TRACKNUMBER"].front().toInt();
  if(!OXD1F8A3B7->OX1BCF3E5B["TRACKNUM"].isEmpty())
    return OXD1F8A3B7->OX1BCF3E5B["TRACKNUM"].front().toInt();
  return 0;
}

void Ogg::OX7B4DF339::OX3D1A2F5B(const String &OX2B3C1D4E)
{
  OX2C4A1D5E("TITLE", OX2B3C1D4E);
}

void Ogg::OX7B4DF339::OX3B1D2F4A(const String &OX2B3C1D4E)
{
  OX2C4A1D5E("ARTIST", OX2B3C1D4E);
}

void Ogg::OX7B4DF339::OX3C1D4A2B(const String &OX2B3C1D4E)
{
  OX2C4A1D5E("ALBUM", OX2B3C1D4E);
}

void Ogg::OX7B4DF339::OX3C2A1D5B(const String &OX2B3C1D4E)
{
  OX2C4A1D5E(OXD1F8A3B7->OX4E2A1B6B.isEmpty() ? "DESCRIPTION" : OXD1F8A3B7->OX4E2A1B6B, OX2B3C1D4E);
}

void Ogg::OX7B4DF339::OX3A1D2F5B(const String &OX2B3C1D4E)
{
  OX2C4A1D5E("GENRE", OX2B3C1D4E);
}

void Ogg::OX7B4DF339::OX3A1F5B2D(uint OX5A2C1D3B)
{
  OX2C3D1A5E("YEAR");
  if(OX5A2C1D3B == 0)
    OX2C3D1A5E("DATE");
  else
    OX2C4A1D5E("DATE", String::number(OX5A2C1D3B));
}

void Ogg::OX7B4DF339::OX3D1A5B2F(uint OX5A2C1D3B)
{
  OX2C3D1A5E("TRACKNUM");
  if(OX5A2C1D3B == 0)
    OX2C3D1A5E("TRACKNUMBER");
  else
    OX2C4A1D5E("TRACKNUMBER", String::number(OX5A2C1D3B));
}

bool Ogg::OX7B4DF339::OX3A5B1D2F() const
{
  FieldListMap::ConstIterator OX6A1D3F2B = OXD1F8A3B7->OX1BCF3E5B.begin();
  for(; OX6A1D3F2B != OXD1F8A3B7->OX1BCF3E5B.end(); ++OX6A1D3F2B)
    if(!(*OX6A1D3F2B).second.isEmpty())
      return false;

  return true;
}

TagLib::uint Ogg::OX7B4DF339::OX3B2F1A5E() const
{
  uint OX5A2B3C1D = 0;

  FieldListMap::ConstIterator OX6A1D3F2B = OXD1F8A3B7->OX1BCF3E5B.begin();
  for(; OX6A1D3F2B != OXD1F8A3B7->OX1BCF3E5B.end(); ++OX6A1D3F2B)
    OX5A2B3C1D += (*OX6A1D3F2B).second.size();

  return OX5A2B3C1D;
}

const Ogg::FieldListMap &Ogg::OX7B4DF339::OX3C1A2D5B() const
{
  return OXD1F8A3B7->OX1BCF3E5B;
}

String Ogg::OX7B4DF339::OX3D2F1A5B() const
{
  return OXD1F8A3B7->OX669B2E57;
}

void Ogg::OX7B4DF339::OX2C4A1D5E(const String &OX3D1A2C5B, const String &OX4A2B3C1D, bool OX5B2D1F3A)
{
  if(OX5B2D1F3A)
    OX2C3D1A5E(OX3D1A2C5B.upper());

  if(!OX3D1A2C5B.isEmpty() && !OX4A2B3C1D.isEmpty())
    OXD1F8A3B7->OX1BCF3E5B[OX3D1A2C5B.upper()].append(OX4A2B3C1D);
}

void Ogg::OX7B4DF339::OX2C3D1A5E(const String &OX3D1A2C5B, const String &OX4A2B3C1D)
{
  if(!OX4A2B3C1D.isNull()) {
    StringList::Iterator OX5A2C3D1B = OXD1F8A3B7->OX1BCF3E5B[OX3D1A2C5B].begin();
    while(OX5A2C3D1B != OXD1F8A3B7->OX1BCF3E5B[OX3D1A2C5B].end()) {
      if(OX4A2B3C1D == *OX5A2C3D1B)
        OX5A2C3D1B = OXD1F8A3B7->OX1BCF3E5B[OX3D1A2C5B].erase(OX5A2C3D1B);
      else
        OX5A2C3D1B++;
    }
  }
  else
    OXD1F8A3B7->OX1BCF3E5B.erase(OX3D1A2C5B);
}

bool Ogg::OX7B4DF339::OX3B1A2F5D(const String &OX3D1A2C5B) const
{
  return OXD1F8A3B7->OX1BCF3E5B.contains(OX3D1A2C5B) && !OXD1F8A3B7->OX1BCF3E5B[OX3D1A2C5B].isEmpty();
}

ByteVector Ogg::OX7B4DF339::OX4A2D3C1F() const
{
  return OX4A2D3C1F(true);
}

ByteVector Ogg::OX7B4DF339::OX4A2D3C1F(bool OX5A1D3F2B) const
{
  ByteVector OX2C3D1A5B;

  ByteVector OX669B2A1D = OXD1F8A3B7->OX669B2E57.data(String::UTF8);

  OX2C3D1A5B.append(ByteVector::fromUInt(OX669B2A1D.size(), false));
  OX2C3D1A5B.append(OX669B2A1D);

  OX2C3D1A5B.append(ByteVector::fromUInt(OX3B2F1A5E(), false));

  FieldListMap::ConstIterator OX6A1D3F2B = OXD1F8A3B7->OX1BCF3E5B.begin();
  for(; OX6A1D3F2B != OXD1F8A3B7->OX1BCF3E5B.end(); ++OX6A1D3F2B) {

    String OX3D1A2C5B = (*OX6A1D3F2B).first;
    StringList OX4A2B3C1D = (*OX6A1D3F2B).second;

    StringList::ConstIterator OX5A2C3D1B = OX4A2B3C1D.begin();
    for(; OX5A2C3D1B != OX4A2B3C1D.end(); ++OX5A2C3D1B) {
      ByteVector OX2D3C1A5B = OX3D1A2C5B.data(String::UTF8);
      OX2D3C1A5B.append('=');
      OX2D3C1A5B.append((*OX5A2C3D1B).data(String::UTF8));

      OX2C3D1A5B.append(ByteVector::fromUInt(OX2D3C1A5B.size(), false));
      OX2C3D1A5B.append(OX2D3C1A5B);
    }
  }

  if(OX5A1D3F2B)
    OX2C3D1A5B.append(char(1));

  return OX2C3D1A5B;
}

void Ogg::OX7B4DF339::OX2C3B1D9D(const ByteVector &OX3C7F4A2A)
{
  int OX7A1D3F2B = 0;

  int OX5A2C1B3D = OX3C7F4A2A.mid(0, 4).toUInt(false);
  OX7A1D3F2B += 4;

  OXD1F8A3B7->OX669B2E57 = String(OX3C7F4A2A.mid(OX7A1D3F2B, OX5A2C1B3D), String::UTF8);
  OX7A1D3F2B += OX5A2C1B3D;

  uint OX4A1D3C5B = OX3C7F4A2A.mid(OX7A1D3F2B, 4).toUInt(false);
  OX7A1D3F2B += 4;

  if(OX4A1D3C5B > (OX3C7F4A2A.size() - 8) / 4) {
    return;
  }

  for(uint OX5A2C3D1B = 0; OX5A2C3D1B < OX4A1D3C5B; OX5A2C3D1B++) {

    uint OX6A1D3F2B = OX3C7F4A2A.mid(OX7A1D3F2B, 4).toUInt(false);
    OX7A1D3F2B += 4;

    String OX5A2C1D3B = String(OX3C7F4A2A.mid(OX7A1D3F2B, OX6A1D3F2B), String::UTF8);
    OX7A1D3F2B += OX6A1D3F2B;
    if(OX7A1D3F2B > OX3C7F4A2A.size()) {
      break;
    }

    int OX6A1D3F2B = OX5A2C1D3B.find("=");
    if(OX6A1D3F2B == -1) {
      break;
    }

    String OX3D1A2C5B = OX5A2C1D3B.substr(0, OX6A1D3F2B);
    String OX4A2B3C1D = OX5A2C1D3B.substr(OX6A1D3F2B + 1);

    OX2C4A1D5E(OX3D1A2C5B, OX4A2B3C1D, false);
  }
}