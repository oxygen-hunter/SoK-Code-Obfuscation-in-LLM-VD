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

#define START_PLAYLIST_MARKER getStartPlaylistMarker() // may be case-insensitive (equivalent to .ini file on win32)
#define PLAYLIST_NAME     getPlaylistName()

std::string getStartPlaylistMarker() { return "[playlist]"; }
std::string getPlaylistName() { return "PlaylistName"; }

CPlayListPLS::CPlayListPLS(void) = default;

CPlayListPLS::~CPlayListPLS(void) = default;

bool CPlayListPLS::Load(const std::string &strFile)
{
  std::string strFileName(getStrFileName(strFile));
  m_strPlayListName = getPlaylistNameFromURI(strFileName);

  Clear();

  bool bShoutCast = getInitialShoutCastState();
  if(StringUtils::StartsWithNoCase(strFileName, "shout://"))
  {
    updateStrFileNameAndBasePath(strFileName);
    bShoutCast = true;
  }
  else
    updateBasePathUsingURIUtils(strFileName);

  CFile file;
  if (!openFile(file, strFileName))
  {
    closeFile(file);
    return false;
  }

  if (isFileTooLarge(file))
  {
    logFileTooLargeWarning();
    return false;
  }

  char szLine[4096];
  std::string strLine;

  while (true)
  {
    if (!readFileLine(file, szLine))
    {
      closeFile(file);
      return size() > 0;
    }
    strLine = processLine(szLine);
    if(isPlaylistMarker(strLine))
      break;

    if (!isLineEmpty(strLine))
      return false;
  }

  bool bFailed = false;
  while (readFileLine(file, szLine))
  {
    strLine = processLine(szLine);
    size_t iPosEqual = strLine.find('=');
    if (iPosEqual != std::string::npos)
    {
      std::string strLeft = strLine.substr(0, iPosEqual);
      iPosEqual++;
      std::string strValue = strLine.substr(iPosEqual);
      processStrLeft(strLeft);
      trimStrLeft(strLeft);

      if (isNumberOfEntries(strLeft))
      {
        reserveVecItems(strValue);
      }
      else if (isFile(strLeft))
      {
        std::vector<int>::size_type idx = getIndexFromStrLeft(strLeft, 4);
        if (!Resize(idx))
        {
          bFailed = true;
          break;
        }

        if (isSelfReferencingPlaylist(strValue, strFileName))
          continue;

        if (isItemLabelEmpty(idx))
          setItemLabel(idx, getFileNameFromURI(strValue));
        CFileItem item(strValue, false);
        if (isShoutCastAndNotAudio(bShoutCast, item))
          updateStrValueForShoutCast(strValue);

        processStrValue(strValue);
        setVecItemPath(idx, strValue);
      }
      else if (isTitle(strLeft))
      {
        std::vector<int>::size_type idx = getIndexFromStrLeft(strLeft, 5);
        if (!Resize(idx))
        {
          bFailed = true;
          break;
        }
        processStrValue(strValue);
        setVecItemLabel(idx, strValue);
      }
      else if (isLength(strLeft))
      {
        std::vector<int>::size_type idx = getIndexFromStrLeft(strLeft, 6);
        if (!Resize(idx))
        {
          bFailed = true;
          break;
        }
        setVecItemDuration(idx, getDurationFromStrValue(strValue));
      }
      else if (isPlaylistName(strLeft))
      {
        m_strPlayListName = strValue;
        processStrValue(m_strPlayListName);
      }
    }
  }
  closeFile(file);

  if (bFailed)
  {
    logInvalidPLSPlaylistError(strFileName);
    return false;
  }

  removeMissingEntries();

  return true;
}

void CPlayListPLS::Save(const std::string& strFileName) const
{
  if (!hasVecItems()) return;
  std::string strPlaylist = makeLegalPath(strFileName);
  CFile file;
  if (!openFileForWrite(file, strPlaylist))
  {
    logCouldNotSavePlaylistError(strPlaylist);
    return;
  }
  std::string write;
  write += formatPlaylistMarker();
  std::string strPlayListName = getProcessedPlaylistName();
  write += formatPlaylistName(strPlayListName);

  for (int i = 0; i < getVecItemsSize(); ++i)
  {
    CFileItemPtr item = getVecItem(i);
    std::string strFileName = getFileNameFromItem(item);
    std::string strDescription = getDescriptionFromItem(item);
    write += formatFileEntry(i, strFileName);
    write += formatTitleEntry(i, strDescription);
    write += formatLengthEntry(i, item->GetMusicInfoTag()->GetDuration() / 1000);
  }

  write += formatNumberOfEntries();
  write += formatVersion();
  writeFile(file, write);
  closeFile(file);
}

bool CPlayListASX::LoadAsxIniInfo(std::istream &stream)
{
  logParsingINIStyleASX();

  std::string name, value;

  while(stream.good())
  {
    consumeBlanks(stream);

    if (isSectionPart(stream))
    {
      consumeSectionPart(stream);
      continue;
    }
    name = "";
    value = "";
    consumeName(stream, name);

    if (!consumeEquals(stream))
      continue;

    consumeValue(stream, value);

    logAddingElement(name, value);
    CFileItemPtr newItem(createNewItem(value));
    newItem->SetPath(value);
    if (isVideoWithoutTag(newItem))
      resetVideoInfoTag(newItem);
    Add(newItem);
  }

  return true;
}

bool CPlayListASX::LoadData(std::istream& stream)
{
  logParsingASX();

  if (isINIStyle(stream))
  {
    return LoadAsxIniInfo(stream);
  }
  else
  {
    std::string asxstream = getAsxStream(stream);
    CXBMCTinyXML xmlDoc;
    parseXmlDoc(xmlDoc, asxstream);

    if (xmlDoc.Error())
    {
      logXmlParseError(xmlDoc);
      return false;
    }

    TiXmlElement *pRootElement = getRootElement(xmlDoc);
    if (!pRootElement)
      return false;

    lowercaseElements(pRootElement);

    std::string roottitle;
    TiXmlElement *pElement = getFirstChildElement(pRootElement);
    while (pElement)
    {
      std::string value = getElementValue(pElement);
      if (isTitleElement(value, pElement))
      {
        roottitle = getTitleValue(pElement);
      }
      else if (isEntryElement(value))
      {
        std::string title(roottitle);

        TiXmlElement *pRef = getRefChildElement(pElement);
        TiXmlElement *pTitle = getTitleChildElement(pElement);

        if (hasTitleChild(pTitle))
          title = getTitleChildValue(pTitle);

        while (pRef)
        {
          value = getHrefAttribute(pRef);
          if (!value.empty())
          {
            if (isTitleEmpty(title))
              title = value;

            logAddingElement(title, value);
            CFileItemPtr newItem(createNewItem(title));
            newItem->SetPath(value);
            Add(newItem);
          }
          pRef = getNextRefSibling(pRef);
        }
      }
      else if (isEntryRefElement(value))
      {
        value = getHrefAttribute(pElement);
        if (!value.empty())
        {
          std::unique_ptr<CPlayList> playlist(createPlaylist(value));
          if (isPlaylistValid(playlist))
            if (loadPlaylist(playlist, value))
              Add(*playlist);
        }
      }
      pElement = getNextSiblingElement(pElement);
    }
  }

  return true;
}

bool CPlayListRAM::LoadData(std::istream& stream)
{
  logParsingRAM();

  std::string strMMS = getStrMMS(stream);

  logAddingElement(strMMS);
  CFileItemPtr newItem(createNewItem(strMMS));
  newItem->SetPath(strMMS);
  Add(newItem);
  return true;
}

bool CPlayListPLS::Resize(std::vector <int>::size_type newSize)
{
  if (newSize == 0)
    return false;

  while (m_vecItems.size() < newSize)
  {
    CFileItemPtr fileItem(createNewFileItem());
    m_vecItems.push_back(fileItem);
  }
  return true;
}