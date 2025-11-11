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

#define START_PLAYLIST_MARKER "[playlist]" 
#define PLAYLIST_NAME     "PlaylistName"

CPlayListPLS::CPlayListPLS(void) = default;

CPlayListPLS::~CPlayListPLS(void) = default;

bool CPlayListPLS::Load(const std::string &strFile)
{
  int state = 0;
  bool continueLoop = true;
  bool result = false;
  while (continueLoop) 
  {
    switch (state) 
    {
      case 0: {
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
          result = false;
          continueLoop = false;
          break;
        }

        if (file.GetLength() > 1024*1024)
        {
          CLog::Log(LOGWARNING, "{} - File is larger than 1 MB, most likely not a playlist",
                    __FUNCTION__);
          result = false;
          continueLoop = false;
          break;
        }
        state = 1;
        break;
      }
      case 1: {
        char szLine[4096];
        std::string strLine;
        while (true)
        {
          if ( !file.ReadString(szLine, sizeof(szLine) ) )
          {
            file.Close();
            result = size() > 0;
            continueLoop = false;
            break;
          }
          strLine = szLine;
          StringUtils::Trim(strLine);
          if(StringUtils::EqualsNoCase(strLine, START_PLAYLIST_MARKER))
            break;

          if(!strLine.empty())
          {
            result = false;
            continueLoop = false;
            break;
          }
        }
        if (!continueLoop) break;
        state = 2;
        break;
      }
      case 2: {
        char szLine[4096];
        std::string strLine;
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
          result = false;
          continueLoop = false;
          break;
        }
        state = 3;
        break;
      }
      case 3: {
        ivecItems p = m_vecItems.begin();
        while ( p != m_vecItems.end())
        {
          if ((*p)->GetPath().empty())
          {
            p = m_vecItems.erase(p);
          }
          else
          {
            ++p;
          }
        }
        result = true;
        continueLoop = false;
        break;
      }
    }
  }
  return result;
}

bool CPlayListASX::LoadAsxIniInfo(std::istream &stream)
{
  int state = 0;
  bool continueLoop = true;
  bool result = false;
  while (continueLoop) 
  {
    switch (state) 
    {
      case 0: {
        CLog::Log(LOGINFO, "Parsing INI style ASX");

        std::string name, value;

        while( stream.good() )
        {
          while((stream.peek() == '\r' || stream.peek() == '\n' || stream.peek() == ' ') && stream.good())
            stream.get();

          if(stream.peek() == '[')
          {
            while(stream.good() && stream.peek() != '\r' && stream.peek() != '\n')
              stream.get();
            continue;
          }
          name = "";
          value = "";
          while(stream.peek() != '\r' && stream.peek() != '\n' && stream.peek() != '=' && stream.good())
            name += stream.get();

          if(stream.get() != '=')
            continue;

          while(stream.peek() != '\r' && stream.peek() != '\n' && stream.good())
            value += stream.get();

          CLog::Log(LOGINFO, "Adding element {}={}", name, value);
          CFileItemPtr newItem(new CFileItem(value));
          newItem->SetPath(value);
          if (newItem->IsVideo() && !newItem->HasVideoInfoTag()) 
            newItem->GetVideoInfoTag()->Reset();
          Add(newItem);
        }
        result = true;
        continueLoop = false;
        break;
      }
    }
  }
  return result;
}

bool CPlayListASX::LoadData(std::istream& stream)
{
  int state = 0;
  bool continueLoop = true;
  bool result = false;
  while (continueLoop) 
  {
    switch (state) 
    {
      case 0: {
        CLog::Log(LOGINFO, "Parsing ASX");

        if(stream.peek() == '[')
        {
          state = 1;
        }
        else
        {
          state = 2;
        }
        break;
      }
      case 1: {
        result = LoadAsxIniInfo(stream);
        continueLoop = false;
        break;
      }
      case 2: {
        std::string asxstream(std::istreambuf_iterator<char>(stream), {});
        CXBMCTinyXML xmlDoc;
        xmlDoc.Parse(asxstream, TIXML_DEFAULT_ENCODING);

        if (xmlDoc.Error())
        {
          CLog::Log(LOGERROR, "Unable to parse ASX info Error: {}", xmlDoc.ErrorDesc());
          result = false;
          continueLoop = false;
          break;
        }

        TiXmlElement *pRootElement = xmlDoc.RootElement();

        if (!pRootElement)
        {
          result = false;
          continueLoop = false;
          break;
        }

        TiXmlNode *pNode = pRootElement;
        TiXmlNode *pChild = NULL;
        std::string value;
        value = pNode->Value();
        StringUtils::ToLower(value);
        pNode->SetValue(value);
        while(pNode)
        {
          pChild = pNode->IterateChildren(pChild);
          if(pChild)
          {
            if (pChild->Type() == TiXmlNode::TINYXML_ELEMENT)
            {
              value = pChild->Value();
              StringUtils::ToLower(value);
              pChild->SetValue(value);

              TiXmlAttribute* pAttr = pChild->ToElement()->FirstAttribute();
              while(pAttr)
              {
                value = pAttr->Name();
                StringUtils::ToLower(value);
                pAttr->SetName(value);
                pAttr = pAttr->Next();
              }
            }

            pNode = pChild;
            pChild = NULL;
            continue;
          }

          pChild = pNode;
          pNode = pNode->Parent();
        }
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
        result = true;
        continueLoop = false;
        break;
      }
    }
  }
  return result;
}

bool CPlayListRAM::LoadData(std::istream& stream)
{
  int state = 0;
  bool continueLoop = true;
  bool result = false;
  while (continueLoop) 
  {
    switch (state) 
    {
      case 0: {
        CLog::Log(LOGINFO, "Parsing RAM");

        std::string strMMS;
        while( stream.peek() != '\n' && stream.peek() != '\r' )
          strMMS += stream.get();

        CLog::Log(LOGINFO, "Adding element {}", strMMS);
        CFileItemPtr newItem(new CFileItem(strMMS));
        newItem->SetPath(strMMS);
        Add(newItem);
        result = true;
        continueLoop = false;
        break;
      }
    }
  }
  return result;
}