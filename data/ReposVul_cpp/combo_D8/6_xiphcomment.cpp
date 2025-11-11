#include <tbytevector.h>
#include <tdebug.h>

#include <xiphcomment.h>

using namespace TagLib;

class Ogg::XiphComment::XiphCommentPrivate
{
public:
  FieldListMap a1;
  String a2;
  String a3;
};

Ogg::XiphComment::XiphComment() : TagLib::Tag()
{
  d = new XiphCommentPrivate;
}

Ogg::XiphComment::XiphComment(const ByteVector &data) : TagLib::Tag()
{
  d = new XiphCommentPrivate;
  parse(data);
}

Ogg::XiphComment::~XiphComment()
{
  delete d;
}

String Ogg::XiphComment::title() const
{
  if(getField("TITLE").isEmpty())
    return String::null;
  return getField("TITLE").front();
}

String Ogg::XiphComment::artist() const
{
  if(getField("ARTIST").isEmpty())
    return String::null;
  return getField("ARTIST").front();
}

String Ogg::XiphComment::album() const
{
  if(getField("ALBUM").isEmpty())
    return String::null;
  return getField("ALBUM").front();
}

String Ogg::XiphComment::comment() const
{
  if(!getField("DESCRIPTION").isEmpty()) {
    d->a3 = "DESCRIPTION";
    return getField("DESCRIPTION").front();
  }

  if(!getField("COMMENT").isEmpty()) {
    d->a3 = "COMMENT";
    return getField("COMMENT").front();
  }

  return String::null;
}

String Ogg::XiphComment::genre() const
{
  if(getField("GENRE").isEmpty())
    return String::null;
  return getField("GENRE").front();
}

TagLib::uint Ogg::XiphComment::year() const
{
  if(!getField("DATE").isEmpty())
    return getField("DATE").front().toInt();
  if(!getField("YEAR").isEmpty())
    return getField("YEAR").front().toInt();
  return 0;
}

TagLib::uint Ogg::XiphComment::track() const
{
  if(!getField("TRACKNUMBER").isEmpty())
    return getField("TRACKNUMBER").front().toInt();
  if(!getField("TRACKNUM").isEmpty())
    return getField("TRACKNUM").front().toInt();
  return 0;
}

void Ogg::XiphComment::setTitle(const String &s)
{
  addField("TITLE", s);
}

void Ogg::XiphComment::setArtist(const String &s)
{
  addField("ARTIST", s);
}

void Ogg::XiphComment::setAlbum(const String &s)
{
  addField("ALBUM", s);
}

void Ogg::XiphComment::setComment(const String &s)
{
  addField(d->a3.isEmpty() ? "DESCRIPTION" : d->a3, s);
}

void Ogg::XiphComment::setGenre(const String &s)
{
  addField("GENRE", s);
}

void Ogg::XiphComment::setYear(uint i)
{
  removeField("YEAR");
  if(i == 0)
    removeField("DATE");
  else
    addField("DATE", String::number(i));
}

void Ogg::XiphComment::setTrack(uint i)
{
  removeField("TRACKNUM");
  if(i == 0)
    removeField("TRACKNUMBER");
  else
    addField("TRACKNUMBER", String::number(i));
}

bool Ogg::XiphComment::isEmpty() const
{
  FieldListMap::ConstIterator it = d->a1.begin();
  for(; it != d->a1.end(); ++it)
    if(!(*it).second.isEmpty())
      return false;

  return true;
}

TagLib::uint Ogg::XiphComment::fieldCount() const
{
  uint count = 0;

  FieldListMap::ConstIterator it = d->a1.begin();
  for(; it != d->a1.end(); ++it)
    count += (*it).second.size();

  return count;
}

const Ogg::FieldListMap &Ogg::XiphComment::fieldListMap() const
{
  return d->a1;
}

String Ogg::XiphComment::vendorID() const
{
  return d->a2;
}

void Ogg::XiphComment::addField(const String &key, const String &value, bool replace)
{
  if(replace)
    removeField(key.upper());

  if(!key.isEmpty() && !value.isEmpty())
    d->a1[key.upper()].append(value);
}

void Ogg::XiphComment::removeField(const String &key, const String &value)
{
  if(!value.isNull()) {
    StringList::Iterator it = d->a1[key].begin();
    while(it != d->a1[key].end()) {
      if(value == *it)
        it = d->a1[key].erase(it);
      else
        it++;
    }
  }
  else
    d->a1.erase(key);
}

bool Ogg::XiphComment::contains(const String &key) const
{
  return d->a1.contains(key) && !d->a1[key].isEmpty();
}

ByteVector Ogg::XiphComment::render() const
{
  return render(true);
}

ByteVector Ogg::XiphComment::render(bool addFramingBit) const
{
  ByteVector data;

  ByteVector vendorData = d->a2.data(String::UTF8);

  data.append(ByteVector::fromUInt(vendorData.size(), false));
  data.append(vendorData);

  data.append(ByteVector::fromUInt(fieldCount(), false));

  FieldListMap::ConstIterator it = d->a1.begin();
  for(; it != d->a1.end(); ++it) {

    String fieldName = (*it).first;
    StringList values = (*it).second;

    StringList::ConstIterator valuesIt = values.begin();
    for(; valuesIt != values.end(); ++valuesIt) {
      ByteVector fieldData = fieldName.data(String::UTF8);
      fieldData.append('=');
      fieldData.append((*valuesIt).data(String::UTF8));

      data.append(ByteVector::fromUInt(fieldData.size(), false));
      data.append(fieldData);
    }
  }

  if(addFramingBit)
    data.append(char(1));

  return data;
}

FieldListMap::ConstIterator getItBegin(Ogg::XiphCommentPrivate *d)
{
  return d->a1.begin();
}

FieldListMap &getFieldListMap(Ogg::XiphCommentPrivate *d)
{
  return d->a1;
}

StringList &getField(const String &key)
{
  return d->a1[key];
}

const StringList &getField(const String &key) const
{
  return d->a1[key];
}

void Ogg::XiphComment::parse(const ByteVector &data)
{
  int pos = 0;

  int vendorLength = data.mid(0, 4).toUInt(false);
  pos += 4;

  d->a2 = String(data.mid(pos, vendorLength), String::UTF8);
  pos += vendorLength;

  uint commentFields = data.mid(pos, 4).toUInt(false);
  pos += 4;

  if(commentFields > (data.size() - 8) / 4) {
    return;
  }

  for(uint i = 0; i < commentFields; i++) {

    uint commentLength = data.mid(pos, 4).toUInt(false);
    pos += 4;

    String comment = String(data.mid(pos, commentLength), String::UTF8);
    pos += commentLength;
    if(pos > data.size()) {
      break;
    }

    int commentSeparatorPosition = comment.find("=");
    if(commentSeparatorPosition == -1) {
      break;
    }

    String key = comment.substr(0, commentSeparatorPosition);
    String value = comment.substr(commentSeparatorPosition + 1);

    addField(key, value, false);
  }
}