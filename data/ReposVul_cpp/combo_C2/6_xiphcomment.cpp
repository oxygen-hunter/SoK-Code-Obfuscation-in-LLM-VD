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
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        if (d->fieldListMap["TITLE"].isEmpty()) {
          dispatcher = 1;
          break;
        }
        return d->fieldListMap["TITLE"].front();
      case 1:
        return String::null;
    }
  }
}

String Ogg::XiphComment::artist() const
{
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        if (d->fieldListMap["ARTIST"].isEmpty()) {
          dispatcher = 1;
          break;
        }
        return d->fieldListMap["ARTIST"].front();
      case 1:
        return String::null;
    }
  }
}

String Ogg::XiphComment::album() const
{
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        if (d->fieldListMap["ALBUM"].isEmpty()) {
          dispatcher = 1;
          break;
        }
        return d->fieldListMap["ALBUM"].front();
      case 1:
        return String::null;
    }
  }
}

String Ogg::XiphComment::comment() const
{
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        if (!d->fieldListMap["DESCRIPTION"].isEmpty()) {
          d->commentField = "DESCRIPTION";
          return d->fieldListMap["DESCRIPTION"].front();
        }
        dispatcher = 1;
        break;
      case 1:
        if (!d->fieldListMap["COMMENT"].isEmpty()) {
          d->commentField = "COMMENT";
          return d->fieldListMap["COMMENT"].front();
        }
        dispatcher = 2;
        break;
      case 2:
        return String::null;
    }
  }
}

String Ogg::XiphComment::genre() const
{
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        if (d->fieldListMap["GENRE"].isEmpty()) {
          dispatcher = 1;
          break;
        }
        return d->fieldListMap["GENRE"].front();
      case 1:
        return String::null;
    }
  }
}

TagLib::uint Ogg::XiphComment::year() const
{
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        if (!d->fieldListMap["DATE"].isEmpty()) {
          return d->fieldListMap["DATE"].front().toInt();
        }
        dispatcher = 1;
        break;
      case 1:
        if (!d->fieldListMap["YEAR"].isEmpty()) {
          return d->fieldListMap["YEAR"].front().toInt();
        }
        dispatcher = 2;
        break;
      case 2:
        return 0;
    }
  }
}

TagLib::uint Ogg::XiphComment::track() const
{
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        if (!d->fieldListMap["TRACKNUMBER"].isEmpty()) {
          return d->fieldListMap["TRACKNUMBER"].front().toInt();
        }
        dispatcher = 1;
        break;
      case 1:
        if (!d->fieldListMap["TRACKNUM"].isEmpty()) {
          return d->fieldListMap["TRACKNUM"].front().toInt();
        }
        dispatcher = 2;
        break;
      case 2:
        return 0;
    }
  }
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
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        removeField("YEAR");
        if (i == 0) {
          dispatcher = 1;
          break;
        }
        addField("DATE", String::number(i));
        return;
      case 1:
        removeField("DATE");
        return;
    }
  }
}

void Ogg::XiphComment::setTrack(uint i)
{
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        removeField("TRACKNUM");
        if (i == 0) {
          dispatcher = 1;
          break;
        }
        addField("TRACKNUMBER", String::number(i));
        return;
      case 1:
        removeField("TRACKNUMBER");
        return;
    }
  }
}

bool Ogg::XiphComment::isEmpty() const
{
  FieldListMap::ConstIterator it = d->fieldListMap.begin();
  for (; it != d->fieldListMap.end(); ++it)
    if (!(*it).second.isEmpty())
      return false;

  return true;
}

TagLib::uint Ogg::XiphComment::fieldCount() const
{
  uint count = 0;

  FieldListMap::ConstIterator it = d->fieldListMap.begin();
  for (; it != d->fieldListMap.end(); ++it)
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
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        if (replace) {
          removeField(key.upper());
        }
        dispatcher = 1;
        break;
      case 1:
        if (!key.isEmpty() && !value.isEmpty()) {
          d->fieldListMap[key.upper()].append(value);
        }
        return;
    }
  }
}

void Ogg::XiphComment::removeField(const String &key, const String &value)
{
  int dispatcher = 0;
  while (true) {
    switch (dispatcher) {
      case 0:
        if (!value.isNull()) {
          StringList::Iterator it = d->fieldListMap[key].begin();
          while (it != d->fieldListMap[key].end()) {
            if (value == *it) {
              it = d->fieldListMap[key].erase(it);
            } else {
              it++;
            }
          }
        } else {
          d->fieldListMap.erase(key);
        }
        return;
    }
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

  FieldListMap::ConstIterator it = d->fieldListMap.begin();
  for (; it != d->fieldListMap.end(); ++it) {
    String fieldName = (*it).first;
    StringList values = (*it).second;

    StringList::ConstIterator valuesIt = values.begin();
    for (; valuesIt != values.end(); ++valuesIt) {
      ByteVector fieldData = fieldName.data(String::UTF8);
      fieldData.append('=');
      fieldData.append((*valuesIt).data(String::UTF8));

      data.append(ByteVector::fromUInt(fieldData.size(), false));
      data.append(fieldData);
    }
  }

  if (addFramingBit)
    data.append(char(1));

  return data;
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

  if (commentFields > (data.size() - 8) / 4) {
    return;
  }

  for (uint i = 0; i < commentFields; i++) {
    uint commentLength = data.mid(pos, 4).toUInt(false);
    pos += 4;

    String comment = String(data.mid(pos, commentLength), String::UTF8);
    pos += commentLength;
    if (pos > data.size()) {
      break;
    }

    int commentSeparatorPosition = comment.find("=");
    if (commentSeparatorPosition == -1) {
      break;
    }

    String key = comment.substr(0, commentSeparatorPosition);
    String value = comment.substr(commentSeparatorPosition + 1);

    addField(key, value, false);
  }
}