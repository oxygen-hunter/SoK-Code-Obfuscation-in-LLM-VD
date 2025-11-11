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

  if (!readUntilPlaylistMarker(file, szLine, strLine))
    return false;

  bool bFailed = false;
  while (file.ReadString(szLine, sizeof(szLine) ) )
  {
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
          break;
        }

        if (StringUtils::EqualsNoCase(URIUtils::GetFileName(strValue),
                                      URIUtils::GetFileName(strFileName)))
          continue;

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
          break;
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
          break;
        }
        m_vecItems[idx - 1]->GetMusicInfoTag()->SetDuration(atol(strValue.c_str()));
      }
      else if (strLeft == "playlistname")
      {
        m_strPlayListName = strValue;
        g_charsetConverter.unknownToUTF8(m_strPlayListName);
      }
    }
  }
  file.Close();

  if (bFailed)
  {
    CLog::Log(LOGERROR,
              "File {} is not a valid PLS playlist. Location of first file,title or length is not "
              "permitted (eg. File0 should be File1)",
              URIUtils::GetFileName(strFileName));
    return false;
  }

  checkForMissingEntries();

  return true;
}

bool CPlayListPLS::readUntilPlaylistMarker(CFile& file, char* szLine, std::string& strLine)
{
  if (!file.ReadString(szLine, sizeof(szLine)))
  {
    file.Close();
    return size() > 0;
  }
  strLine = szLine;
  StringUtils::Trim(strLine);
  if (!StringUtils::EqualsNoCase(strLine, START_PLAYLIST_MARKER))
  {
    if (!strLine.empty())
      return false;
    return readUntilPlaylistMarker(file, szLine, strLine);
  }
  return true;
}

void CPlayListPLS::checkForMissingEntries()
{
  checkForMissingEntriesRec(m_vecItems.begin());
}

void CPlayListPLS::checkForMissingEntriesRec(ivecItems::iterator p)
{
  if (p != m_vecItems.end())
  {
    if ((*p)->GetPath().empty())
      m_vecItems.erase(p);
    else
      ++p;
    checkForMissingEntriesRec(p);
  }
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

  saveItemsRecursively(write, 0);

  write += StringUtils::Format("NumberOfEntries={0}\n", m_vecItems.size());
  write += StringUtils::Format("Version=2\n");
  file.Write(write.c_str(), write.size());
  file.Close();
}

void CPlayListPLS::saveItemsRecursively(std::string& write, int i) const
{
  if (i < static_cast<int>(m_vecItems.size()))
  {
    CFileItemPtr item = m_vecItems[i];
    std::string strFileName = item->GetPath();
    g_charsetConverter.utf8ToStringCharset(strFileName);
    std::string strDescription = item->GetLabel();
    g_charsetConverter.utf8ToStringCharset(strDescription);
    write += StringUtils::Format("File{}={}\n", i + 1, strFileName);
    write += StringUtils::Format("Title{}={}\n", i + 1, strDescription.c_str());
    write += StringUtils::Format("Length{}={}\n", i + 1, item->GetMusicInfoTag()->GetDuration() / 1000);

    saveItemsRecursively(write, i + 1);
  }
}

bool CPlayListASX::LoadAsxIniInfo(std::istream &stream)
{
  CLog::Log(LOGINFO, "Parsing INI style ASX");

  std::string name, value;

  if (!stream.good())
    return true;

  consumeBlanks(stream);
  if (stream.peek() == '[')
  {
    consumeSection(stream);
    return LoadAsxIniInfo(stream);
  }
  name = "";
  value = "";
  consumeName(stream, name);
  if (stream.get() != '=')
    return LoadAsxIniInfo(stream);
  consumeValue(stream, value);
  CLog::Log(LOGINFO, "Adding element {}={}", name, value);
  CFileItemPtr newItem(new CFileItem(value));
  newItem->SetPath(value);
  if (newItem->IsVideo() && !newItem->HasVideoInfoTag())
    newItem->GetVideoInfoTag()->Reset();
  Add(newItem);

  return LoadAsxIniInfo(stream);
}

void CPlayListASX::consumeBlanks(std::istream& stream)
{
  while ((stream.peek() == '\r' || stream.peek() == '\n' || stream.peek() == ' ') && stream.good())
    stream.get();
}

void CPlayListASX::consumeSection(std::istream& stream)
{
  while (stream.good() && stream.peek() != '\r' && stream.peek() != '\n')
    stream.get();
}

void CPlayListASX::consumeName(std::istream& stream, std::string& name)
{
  while (stream.peek() != '\r' && stream.peek() != '\n' && stream.peek() != '=' && stream.good())
    name += stream.get();
}

void CPlayListASX::consumeValue(std::istream& stream, std::string& value)
{
  while (stream.peek() != '\r' && stream.peek() != '\n' && stream.good())
    value += stream.get();
}

bool CPlayListRAM::LoadData(std::istream& stream)
{
  CLog::Log(LOGINFO, "Parsing RAM");

  std::string strMMS;
  consumeUntilNewline(stream, strMMS);

  CLog::Log(LOGINFO, "Adding element {}", strMMS);
  CFileItemPtr newItem(new CFileItem(strMMS));
  newItem->SetPath(strMMS);
  Add(newItem);
  return true;
}

void CPlayListRAM::consumeUntilNewline(std::istream& stream, std::string& str)
{
  if (stream.peek() != '\n' && stream.peek() != '\r')
  {
    str += stream.get();
    consumeUntilNewline(stream, str);
  }
}

bool CPlayListPLS::Resize(std::vector <int>::size_type newSize)
{
  if (newSize == 0)
    return false;

  resizeRecursively(newSize);

  return true;
}

void CPlayListPLS::resizeRecursively(std::vector<int>::size_type newSize)
{
  if (m_vecItems.size() < newSize)
  {
    CFileItemPtr fileItem(new CFileItem());
    m_vecItems.push_back(fileItem);
    resizeRecursively(newSize);
  }
}