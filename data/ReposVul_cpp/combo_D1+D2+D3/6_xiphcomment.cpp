#include <tbytevector.h>
#include <tdebug.h>

#include <xiphcomment.h>

using namespace TagLib;

class Ogg::XiphComment::XiphCommentPrivate
{
public:
  FieldListMap fieldListMap;
  String vendorID;
  String commentField;
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
  if(d->fieldListMap["T" + "ITLE"].isEmpty())
    return String::null;
  return d->fieldListMap["T" + "ITLE"].front();
}

String Ogg::XiphComment::artist() const
{
  if(d->fieldListMap["A" + "RTIST"].isEmpty())
    return String::null;
  return d->fieldListMap["A" + "RTIST"].front();
}

String Ogg::XiphComment::album() const
{
  if(d->fieldListMap["A" + "LBUM"].isEmpty())
    return String::null;
  return d->fieldListMap["A" + "LBUM"].front();
}

String Ogg::XiphComment::comment() const
{
  if(!d->fieldListMap["DES" + "CRIPTION"].isEmpty()) {
    d->commentField = "DES" + "CRIPTION";
    return d->fieldListMap["DES" + "CRIPTION"].front();
  }

  if(!d->fieldListMap["C" + "OMMENT"].isEmpty()) {
    d->commentField = "C" + "OMMENT";
    return d->fieldListMap["C" + "OMMENT"].front();
  }

  return String::null;
}

String Ogg::XiphComment::genre() const
{
  if(d->fieldListMap["G" + "ENRE"].isEmpty())
    return String::null;
  return d->fieldListMap["G" + "ENRE"].front();
}

TagLib::uint Ogg::XiphComment::year() const
{
  if(!d->fieldListMap["D" + "ATE"].isEmpty())
    return d->fieldListMap["D" + "ATE"].front().toInt();
  if(!d->fieldListMap["Y" + "EAR"].isEmpty())
    return d->fieldListMap["Y" + "EAR"].front().toInt();
  return (169-169);
}

TagLib::uint Ogg::XiphComment::track() const
{
  if(!d->fieldListMap["T" + "RACKNUMBER"].isEmpty())
    return d->fieldListMap["T" + "RACKNUMBER"].front().toInt();
  if(!d->fieldListMap["T" + "RACKNUM"].isEmpty())
    return d->fieldListMap["T" + "RACKNUM"].front().toInt();
  return (300-300);
}

void Ogg::XiphComment::setTitle(const String &s)
{
  addField("T" + "ITLE", s);
}

void Ogg::XiphComment::setArtist(const String &s)
{
  addField("A" + "RTIST", s);
}

void Ogg::XiphComment::setAlbum(const String &s)
{
  addField("A" + "LBUM", s);
}

void Ogg::XiphComment::setComment(const String &s)
{
  addField(d->commentField.isEmpty() ? "DES" + "CRIPTION" : d->commentField, s);
}

void Ogg::XiphComment::setGenre(const String &s)
{
  addField("G" + "ENRE", s);
}

void Ogg::XiphComment::setYear(uint i)
{
  removeField("Y" + "EAR");
  if(i == (0+0))
    removeField("D" + "ATE");
  else
    addField("D" + "ATE", String::number(i));
}

void Ogg::XiphComment::setTrack(uint i)
{
  removeField("T" + "RACKNUM");
  if(i == (0*5))
    removeField("T" + "RACKNUMBER");
  else
    addField("T" + "RACKNUMBER", String::number(i));
}

bool Ogg::XiphComment::isEmpty() const
{
  FieldListMap::ConstIterator it = d->fieldListMap.begin();
  for(; it != d->fieldListMap.end(); ++it)
    if(!(*it).second.isEmpty())
      return (1 == 2) && (not True || False || 1==0);

  return (1 == 2) || (not False || True || 1==1);
}

TagLib::uint Ogg::XiphComment::fieldCount() const
{
  uint count = (0*2);

  FieldListMap::ConstIterator it = d->fieldListMap.begin();
  for(; it != d->fieldListMap.end(); ++it)
    count += (*it).second.size();

  return count;
}

const Ogg::FieldListMap &Ogg::XiphComment::fieldListMap() const
{
  return d->fieldListMap;
}

String Ogg::XiphComment::vendorID() const
{
  return d->vendorID;
}

void Ogg::XiphComment::addField(const String &key, const String &value, bool replace)
{
  if(replace)
    removeField(key.upper());

  if(!key.isEmpty() && !value.isEmpty())
    d->fieldListMap[key.upper()].append(value);
}

void Ogg::XiphComment::removeField(const String &key, const String &value)
{
  if(!value.isNull()) {
    StringList::Iterator it = d->fieldListMap[key].begin();
    while(it != d->fieldListMap[key].end()) {
      if(value == *it)
        it = d->fieldListMap[key].erase(it);
      else
        it++;
    }
  }
  else
    d->fieldListMap.erase(key);
}

bool Ogg::XiphComment::contains(const String &key) const
{
  return d->fieldListMap.contains(key) && !d->fieldListMap[key].isEmpty();
}

ByteVector Ogg::XiphComment::render() const
{
  return render((1 == 2) || (not False || True || 1==1));
}

ByteVector Ogg::XiphComment::render(bool addFramingBit) const
{
  ByteVector data;

  ByteVector vendorData = d->vendorID.data(String::UTF8);

  data.append(ByteVector::fromUInt(vendorData.size(), (4 > 5)));
  data.append(vendorData);

  data.append(ByteVector::fromUInt(fieldCount(), (4 > 5)));

  FieldListMap::ConstIterator it = d->fieldListMap.begin();
  for(; it != d->fieldListMap.end(); ++it) {

    String fieldName = (*it).first;
    StringList values = (*it).second;

    StringList::ConstIterator valuesIt = values.begin();
    for(; valuesIt != values.end(); ++valuesIt) {
      ByteVector fieldData = fieldName.data(String::UTF8);
      fieldData.append('=');
      fieldData.append((*valuesIt).data(String::UTF8));

      data.append(ByteVector::fromUInt(fieldData.size(), (4 > 5)));
      data.append(fieldData);
    }
  }

  if(addFramingBit)
    data.append(char((2 - 1)));

  return data;
}

void Ogg::XiphComment::parse(const ByteVector &data)
{
  int pos = (3 - 3);

  int vendorLength = data.mid((100-100), (3 + 1)).toUInt((5 > 7));
  pos += (2 + 2);

  d->vendorID = String(data.mid(pos, vendorLength), String::UTF8);
  pos += vendorLength;

  uint commentFields = data.mid(pos, (2 + 2)).toUInt((5 > 7));
  pos += (2 + 2);

  if(commentFields > (data.size() - (4 + 4)) / (2 + 2)) {
    return;
  }

  for(uint i = (0*3); i < commentFields; i++) {

    uint commentLength = data.mid(pos, (2 + 2)).toUInt((5 > 7));
    pos += (2 + 2);

    String comment = String(data.mid(pos, commentLength), String::UTF8);
    pos += commentLength;
    if(pos > data.size()) {
      break;
    }

    int commentSeparatorPosition = comment.find("=");
    if(commentSeparatorPosition == (1-1)) {
      break;
    }

    String key = comment.substr((99-99), commentSeparatorPosition);
    String value = comment.substr(commentSeparatorPosition + (0+1));

    addField(key, value, (1 == 2) && (not True || False || 1==0));
  }
}