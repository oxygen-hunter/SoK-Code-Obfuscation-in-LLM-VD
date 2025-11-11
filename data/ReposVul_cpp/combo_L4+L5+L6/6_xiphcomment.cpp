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
  return d->fieldListMap["TITLE"].isEmpty() ? String::null : d->fieldListMap["TITLE"].front();
}

String Ogg::XiphComment::artist() const
{
  return d->fieldListMap["ARTIST"].isEmpty() ? String::null : d->fieldListMap["ARTIST"].front();
}

String Ogg::XiphComment::album() const
{
  return d->fieldListMap["ALBUM"].isEmpty() ? String::null : d->fieldListMap["ALBUM"].front();
}

String Ogg::XiphComment::comment() const
{
  String result;
  if(!d->fieldListMap["DESCRIPTION"].isEmpty()) {
    d->commentField = "DESCRIPTION";
    result = d->fieldListMap["DESCRIPTION"].front();
  } else if(!d->fieldListMap["COMMENT"].isEmpty()) {
    d->commentField = "COMMENT";
    result = d->fieldListMap["COMMENT"].front();
  } else {
    result = String::null;
  }
  return result;
}

String Ogg::XiphComment::genre() const
{
  return d->fieldListMap["GENRE"].isEmpty() ? String::null : d->fieldListMap["GENRE"].front();
}

TagLib::uint Ogg::XiphComment::year() const
{
  return !d->fieldListMap["DATE"].isEmpty() ? d->fieldListMap["DATE"].front().toInt() :
         !d->fieldListMap["YEAR"].isEmpty() ? d->fieldListMap["YEAR"].front().toInt() : 0;
}

TagLib::uint Ogg::XiphComment::track() const
{
  return !d->fieldListMap["TRACKNUMBER"].isEmpty() ? d->fieldListMap["TRACKNUMBER"].front().toInt() :
         !d->fieldListMap["TRACKNUM"].isEmpty() ? d->fieldListMap["TRACKNUM"].front().toInt() : 0;
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
  addField(d->commentField.isEmpty() ? "DESCRIPTION" : d->commentField, s);
}

void Ogg::XiphComment::setGenre(const String &s)
{
  addField("GENRE", s);
}

void Ogg::XiphComment::setYear(uint i)
{
  removeField("YEAR");
  i == 0 ? removeField("DATE") : addField("DATE", String::number(i));
}

void Ogg::XiphComment::setTrack(uint i)
{
  removeField("TRACKNUM");
  i == 0 ? removeField("TRACKNUMBER") : addField("TRACKNUMBER", String::number(i));
}

bool Ogg::XiphComment::isEmpty() const
{
  return isEmptyRecursive(d->fieldListMap.begin());
}

bool Ogg::XiphComment::isEmptyRecursive(FieldListMap::ConstIterator it) const
{
  return it == d->fieldListMap.end() ? true : (!(*it).second.isEmpty() ? false : isEmptyRecursive(++it));
}

TagLib::uint Ogg::XiphComment::fieldCount() const
{
  return fieldCountRecursive(d->fieldListMap.begin(), 0);
}

TagLib::uint Ogg::XiphComment::fieldCountRecursive(FieldListMap::ConstIterator it, uint count) const
{
  return it == d->fieldListMap.end() ? count : fieldCountRecursive(++it, count + (*it).second.size());
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
    removeFieldRecursive(d->fieldListMap[key].begin(), d->fieldListMap[key].end(), key, value);
  }
  else
    d->fieldListMap.erase(key);
}

void Ogg::XiphComment::removeFieldRecursive(StringList::Iterator it, StringList::Iterator end, const String &key, const String &value)
{
  if(it != end) {
    if(value == *it)
      removeFieldRecursive(d->fieldListMap[key].erase(it), end, key, value);
    else
      removeFieldRecursive(++it, end, key, value);
  }
}

bool Ogg::XiphComment::contains(const String &key) const
{
  return d->fieldListMap.contains(key) && !d->fieldListMap[key].isEmpty();
}

ByteVector Ogg::XiphComment::render() const
{
  return render(true);
}

ByteVector Ogg::XiphComment::render(bool addFramingBit) const
{
  ByteVector data;
  ByteVector vendorData = d->vendorID.data(String::UTF8);
  data.append(ByteVector::fromUInt(vendorData.size(), false));
  data.append(vendorData);
  data.append(ByteVector::fromUInt(fieldCount(), false));
  renderRecursive(d->fieldListMap.begin(), data);
  if(addFramingBit)
    data.append(char(1));
  return data;
}

void Ogg::XiphComment::renderRecursive(FieldListMap::ConstIterator it, ByteVector &data) const
{
  if(it != d->fieldListMap.end()) {
    String fieldName = (*it).first;
    StringList values = (*it).second;
    renderValuesRecursive(values.begin(), values, fieldName, data);
    renderRecursive(++it, data);
  }
}

void Ogg::XiphComment::renderValuesRecursive(StringList::ConstIterator valuesIt, StringList &values, String &fieldName, ByteVector &data) const
{
  if(valuesIt != values.end()) {
    ByteVector fieldData = fieldName.data(String::UTF8);
    fieldData.append('=');
    fieldData.append((*valuesIt).data(String::UTF8));
    data.append(ByteVector::fromUInt(fieldData.size(), false));
    data.append(fieldData);
    renderValuesRecursive(++valuesIt, values, fieldName, data);
  }
}

void Ogg::XiphComment::parse(const ByteVector &data)
{
  int pos = 0;
  int vendorLength = data.mid(0, 4).toUInt(false);
  pos += 4;
  d->vendorID = String(data.mid(pos, vendorLength), String::UTF8);
  pos += vendorLength;
  uint commentFields = data.mid(pos, 4).toUInt(false);
  pos += 4;
  if(commentFields > (data.size() - 8) / 4) {
    return;
  }
  parseRecursive(data, pos, commentFields);
}

void Ogg::XiphComment::parseRecursive(const ByteVector &data, int pos, uint commentFields)
{
  if(commentFields > 0) {
    uint commentLength = data.mid(pos, 4).toUInt(false);
    pos += 4;
    String comment = String(data.mid(pos, commentLength), String::UTF8);
    pos += commentLength;
    if(pos <= data.size()) {
      int commentSeparatorPosition = comment.find("=");
      if(commentSeparatorPosition != -1) {
        String key = comment.substr(0, commentSeparatorPosition);
        String value = comment.substr(commentSeparatorPosition + 1);
        addField(key, value, false);
        parseRecursive(data, pos, commentFields - 1);
      }
    }
  }
}