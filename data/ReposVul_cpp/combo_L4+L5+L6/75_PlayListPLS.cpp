#include "PlayListPLS.h"

#include "PlayListFactory.h"
#include "Util.h"
#include "filesystem/File.h"
#include "music/tags/MusicInfoTag.h"
#include "utils/CharsetConverter.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "utils/XBMCTinyXML.h"
#include "utils/XMLUtils.h"
#include "utils/log.h"
#include "video/VideoInfoTag.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace XFILE;
using namespace PLAYLIST;

#define START_PLAYLIST_MARKER "[playlist]" // may be case-insensitive (equivalent to .ini file on win32)
#define PLAYLIST_NAME     "PlaylistName"

CPlayListPLS::CPlayListPLS(void) = default;

CPlayListPLS::~CPlayListPLS(void) = default;

bool CPlayListPLS::Load(const std::string &strFile)
{
  std::string strFileName(strFile);
  m_strPlayListName = URIUtils::GetFileName(strFileName);

  Clear();

  bool bShoutCast = false;
  if( StringUtils::StartsWithNoCase(strFileName, "shout://") )
  {
    strFileName.replace(0, 8, "http://");
    m_strBasePath = "";
    bShoutCast = true;
  }
  else
    URIUtils::GetParentPath(strFileName, m_strBasePath);

  CFile file;
  if (!file.Open(strFileName) )
  {
    file.Close();
    return false;
  }

  if (file.GetLength() > 1024*1024)
  {
    CLog::Log(LOGWARNING, "{} - File is larger than 1 MB, most likely not a playlist",
              __FUNCTION__);
    return false;
  }

  char szLine[4096];
  std::string strLine;

  auto readAndCheck = [&]() -> bool {
    if (!file.ReadString(szLine, sizeof(szLine))) {
      file.Close();
      return size() > 0;
    }
    strLine = szLine;
    StringUtils::Trim(strLine);
    return true;
  };

  auto findStartMarker = [&]() {
    if (readAndCheck()) {
      if (StringUtils::EqualsNoCase(strLine, START_PLAYLIST_MARKER))
        return true;
      if (!strLine.empty())
        return false;
      return findStartMarker();
    }
    return false;
  };

  if (!findStartMarker())
    return false;

  bool bFailed = false;
  auto processFile = [&]() -> bool {
    if (!file.ReadString(szLine, sizeof(szLine)))
      return false;
    strLine = szLine;
    StringUtils::RemoveCRLF(strLine);
    size_t iPosEqual = strLine.find('=');
    if (iPosEqual != std::string::npos)
    {
      std::string strLeft = strLine.substr(0, iPosEqual);
      iPosEqual++;
      std::string strValue = strLine.substr(iPosEqual);
      StringUtils::ToLower(strLeft);
      StringUtils::TrimLeft(strLeft);

      if (strLeft == "numberofentries")
      {
        m_vecItems.reserve(atoi(strValue.c_str()));
      }
      else if (StringUtils::StartsWith(strLeft, "file"))
      {
        std::vector <int>::size_type idx = atoi(strLeft.c_str() + 4);
        if (!Resize(idx))
        {
          bFailed = true;
          return false;
        }

        if (StringUtils::EqualsNoCase(URIUtils::GetFileName(strValue),
                                      URIUtils::GetFileName(strFileName)))
          return true;

        if (m_vecItems[idx - 1]->GetLabel().empty())
          m_vecItems[idx - 1]->SetLabel(URIUtils::GetFileName(strValue));
        CFileItem item(strValue, false);
        if (bShoutCast && !item.IsAudio())
          strValue.replace(0, 7, "shout://");

        strValue = URIUtils::SubstitutePath(strValue);
        CUtil::GetQualifiedFilename(m_strBasePath, strValue);
        g_charsetConverter.unknownToUTF8(strValue);
        m_vecItems[idx - 1]->SetPath(strValue);
      }
      else if (StringUtils::StartsWith(strLeft, "title"))
      {
        std::vector <int>::size_type idx = atoi(strLeft.c_str() + 5);
        if (!Resize(idx))
        {
          bFailed = true;
          return false;
        }
        g_charsetConverter.unknownToUTF8(strValue);
        m_vecItems[idx - 1]->SetLabel(strValue);
      }
      else if (StringUtils::StartsWith(strLeft, "length"))
      {
        std::vector <int>::size_type idx = atoi(strLeft.c_str() + 6);
        if (!Resize(idx))
        {
          bFailed = true;
          return false;
        }
        m_vecItems[idx - 1]->GetMusicInfoTag()->SetDuration(atol(strValue.c_str()));
      }
      else if (strLeft == "playlistname")
      {
        m_strPlayListName = strValue;
        g_charsetConverter.unknownToUTF8(m_strPlayListName);
      }
    }
    return true;
  };

  while (processFile());

  file.Close();

  if (bFailed)
  {
    CLog::Log(LOGERROR,
              "File {} is not a valid PLS playlist. Location of first file,title or length is not "
              "permitted (eg. File0 should be File1)",
              URIUtils::GetFileName(strFileName));
    return false;
  }

  auto checkMissingEntries = [&](ivecItems::iterator p) {
    if (p != m_vecItems.end())
    {
      if ((*p)->GetPath().empty())
      {
        p = m_vecItems.erase(p);
        checkMissingEntries(p);
      }
      else
      {
        checkMissingEntries(++p);
      }
    }
  };

  checkMissingEntries(m_vecItems.begin());

  return true;
}

bool CPlayListPLS::Resize(std::vector <int>::size_type newSize)
{
  if (newSize == 0)
    return false;

  auto addFileItems = [&]() {
    if (m_vecItems.size() < newSize)
    {
      CFileItemPtr fileItem(new CFileItem());
      m_vecItems.push_back(fileItem);
      addFileItems();
    }
  };

  addFileItems();
  return true;
}

void CPlayListPLS::Save(const std::string& strFileName) const
{
  if (!m_vecItems.size()) return ;
  std::string strPlaylist = CUtil::MakeLegalPath(strFileName);
  CFile file;
  if (!file.OpenForWrite(strPlaylist, true))
  {
    CLog::Log(LOGERROR, "Could not save PLS playlist: [{}]", strPlaylist);
    return;
  }
  std::string write;
  write += StringUtils::Format("{}\n", START_PLAYLIST_MARKER);
  std::string strPlayListName=m_strPlayListName;
  g_charsetConverter.utf8ToStringCharset(strPlayListName);
  write += StringUtils::Format("PlaylistName={}\n", strPlayListName);

  std::function<void(int)> saveItems = [&](int i) {
    if (i < (int)m_vecItems.size()) {
      CFileItemPtr item = m_vecItems[i];
      std::string strFileName=item->GetPath();
      g_charsetConverter.utf8ToStringCharset(strFileName);
      std::string strDescription=item->GetLabel();
      g_charsetConverter.utf8ToStringCharset(strDescription);
      write += StringUtils::Format("File{}={}\n", i + 1, strFileName);
      write += StringUtils::Format("Title{}={}\n", i + 1, strDescription.c_str());
      write +=
          StringUtils::Format("Length{}={}\n", i + 1, item->GetMusicInfoTag()->GetDuration() / 1000);
      saveItems(i + 1);
    }
  };

  saveItems(0);

  write += StringUtils::Format("NumberOfEntries={0}\n", m_vecItems.size());
  write += StringUtils::Format("Version=2\n");
  file.Write(write.c_str(), write.size());
  file.Close();
}

bool CPlayListASX::LoadAsxIniInfo(std::istream &stream)
{
  CLog::Log(LOGINFO, "Parsing INI style ASX");

  std::string name, value;

  auto processStream = [&]() -> bool {
    while((stream.peek() == '\r' || stream.peek() == '\n' || stream.peek() == ' ') && stream.good())
      stream.get();

    if(stream.peek() == '[')
    {
      while(stream.good() && stream.peek() != '\r' && stream.peek() != '\n')
        stream.get();
      return processStream();
    }
    
    name = "";
    value = "";
    while(stream.peek() != '\r' && stream.peek() != '\n' && stream.peek() != '=' && stream.good())
      name += stream.get();

    if(stream.get() != '=')
      return processStream();

    while(stream.peek() != '\r' && stream.peek() != '\n' && stream.good())
      value += stream.get();

    CLog::Log(LOGINFO, "Adding element {}={}", name, value);
    CFileItemPtr newItem(new CFileItem(value));
    newItem->SetPath(value);
    if (newItem->IsVideo() && !newItem->HasVideoInfoTag()) 
      newItem->GetVideoInfoTag()->Reset(); 
    Add(newItem);

    return processStream();
  };

  return processStream();
}

bool CPlayListASX::LoadData(std::istream& stream)
{
  CLog::Log(LOGINFO, "Parsing ASX");

  if(stream.peek() == '[')
  {
    return LoadAsxIniInfo(stream);
  }
  else
  {
    std::string asxstream(std::istreambuf_iterator<char>(stream), {});
    CXBMCTinyXML xmlDoc;
    xmlDoc.Parse(asxstream, TIXML_DEFAULT_ENCODING);

    if (xmlDoc.Error())
    {
      CLog::Log(LOGERROR, "Unable to parse ASX info Error: {}", xmlDoc.ErrorDesc());
      return false;
    }

    TiXmlElement *pRootElement = xmlDoc.RootElement();

    if (!pRootElement)
      return false;

    TiXmlNode *pNode = pRootElement;
    TiXmlNode *pChild = NULL;
    std::string value;
    value = pNode->Value();
    StringUtils::ToLower(value);
    pNode->SetValue(value);

    auto lowercaseElements = [&]() {
      if (pNode) {
        pChild = pNode->IterateChildren(pChild);
        if (pChild) {
          if (pChild->Type() == TiXmlNode::TINYXML_ELEMENT) {
            value = pChild->Value();
            StringUtils::ToLower(value);
            pChild->SetValue(value);

            TiXmlAttribute* pAttr = pChild->ToElement()->FirstAttribute();
            while (pAttr) {
              value = pAttr->Name();
              StringUtils::ToLower(value);
              pAttr->SetName(value);
              pAttr = pAttr->Next();
            }
          }

          pNode = pChild;
          pChild = NULL;
          lowercaseElements();
        } else {
          pChild = pNode;
          pNode = pNode->Parent();
          lowercaseElements();
        }
      }
    };

    lowercaseElements();
    std::string roottitle;
    TiXmlElement *pElement = pRootElement->FirstChildElement();
    while (pElement)
    {
      value = pElement->Value();
      if (value == "title" && !pElement->NoChildren())
      {
        roottitle = pElement->FirstChild()->ValueStr();
      }
      else if (value == "entry")
      {
        std::string title(roottitle);

        TiXmlElement *pRef = pElement->FirstChildElement("ref");
        TiXmlElement *pTitle = pElement->FirstChildElement("title");

        if(pTitle && !pTitle->NoChildren())
          title = pTitle->FirstChild()->ValueStr();

        while (pRef)
        {
          value = XMLUtils::GetAttribute(pRef, "href");
          if (!value.empty())
          {
            if(title.empty())
              title = value;

            CLog::Log(LOGINFO, "Adding element {}, {}", title, value);
            CFileItemPtr newItem(new CFileItem(title));
            newItem->SetPath(value);
            Add(newItem);
          }
          pRef = pRef->NextSiblingElement("ref");
        }
      }
      else if (value == "entryref")
      {
        value = XMLUtils::GetAttribute(pElement, "href");
        if (!value.empty())
        {
          std::unique_ptr<CPlayList> playlist(CPlayListFactory::Create(value));
          if (nullptr != playlist)
            if (playlist->Load(value))
              Add(*playlist);
        }
      }
      pElement = pElement->NextSiblingElement();
    }
  }

  return true;
}


bool CPlayListRAM::LoadData(std::istream& stream)
{
  CLog::Log(LOGINFO, "Parsing RAM");

  std::string strMMS;
  auto extractMMS = [&]() {
    if (stream.peek() != '\n' && stream.peek() != '\r') {
      strMMS += stream.get();
      extractMMS();
    }
  };

  extractMMS();

  CLog::Log(LOGINFO, "Adding element {}", strMMS);
  CFileItemPtr newItem(new CFileItem(strMMS));
  newItem->SetPath(strMMS);
  Add(newItem);
  return true;
}