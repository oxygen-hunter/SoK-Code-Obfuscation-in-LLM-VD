/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

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

#define OX7B4DF339 "[playlist]" // may be case-insensitive (equivalent to .ini file on win32)
#define OX1C9B9DA7 "PlaylistName"

/*----------------------------------------------------------------------
[playlist]
PlaylistName=Playlist 001
File1=E:\Program Files\Winamp3\demo.mp3
Title1=demo
Length1=5
File2=E:\Program Files\Winamp3\demo.mp3
Title2=demo
Length2=5
NumberOfEntries=2
Version=2
----------------------------------------------------------------------*/
CPlayListPLS::OX43AC7F8E(void) = default;

CPlayListPLS::~OX43AC7F8E(void) = default;

bool CPlayListPLS::OX9F5D1C6A(const std::string &OX3A3A0D6E)
{
  std::string OX4E5E7B20(OX3A3A0D6E);
  OX5F68A1B4 = URIUtils::GetFileName(OX4E5E7B20);

  Clear();

  bool OX184C8B6E = false;
  if( StringUtils::StartsWithNoCase(OX4E5E7B20, "shout://") )
  {
    OX4E5E7B20.replace(0, 8, "http://");
    OX631F9F22 = "";
    OX184C8B6E = true;
  }
  else
    URIUtils::GetParentPath(OX4E5E7B20, OX631F9F22);

  CFile OX0C6A7F69;
  if (!OX0C6A7F69.Open(OX4E5E7B20) )
  {
    OX0C6A7F69.Close();
    return false;
  }

  if (OX0C6A7F69.GetLength() > 1024*1024)
  {
    CLog::Log(LOGWARNING, "{} - File is larger than 1 MB, most likely not a playlist",
              __FUNCTION__);
    return false;
  }

  char OX5F005DA8[4096];
  std::string OX1E1D5D5A;

  while (true)
  {
    if ( !OX0C6A7F69.ReadString(OX5F005DA8, sizeof(OX5F005DA8) ) )
    {
      OX0C6A7F69.Close();
      return size() > 0;
    }
    OX1E1D5D5A = OX5F005DA8;
    StringUtils::Trim(OX1E1D5D5A);
    if(StringUtils::EqualsNoCase(OX1E1D5D5A, OX7B4DF339))
      break;

    if(!OX1E1D5D5A.empty())
      return false;
  }

  bool OX382E7F61 = false;
  while (OX0C6A7F69.ReadString(OX5F005DA8, sizeof(OX5F005DA8) ) )
  {
    OX1E1D5D5A = OX5F005DA8;
    StringUtils::RemoveCRLF(OX1E1D5D5A);
    size_t OX5A8B6E99 = OX1E1D5D5A.find('=');
    if (OX5A8B6E99 != std::string::npos)
    {
      std::string OX7E4D3E6B = OX1E1D5D5A.substr(0, OX5A8B6E99);
      OX5A8B6E99++;
      std::string OX7E5A5C8A = OX1E1D5D5A.substr(OX5A8B6E99);
      StringUtils::ToLower(OX7E4D3E6B);
      StringUtils::TrimLeft(OX7E4D3E6B);

      if (OX7E4D3E6B == "numberofentries")
      {
        OX7F2F6D71.reserve(atoi(OX7E5A5C8A.c_str()));
      }
      else if (StringUtils::StartsWith(OX7E4D3E6B, "file"))
      {
        std::vector <int>::size_type OX0E7F3C1A = atoi(OX7E4D3E6B.c_str() + 4);
        if (!OX6D5F4E5A(OX0E7F3C1A))
        {
          OX382E7F61 = true;
          break;
        }

        if (StringUtils::EqualsNoCase(URIUtils::GetFileName(OX7E5A5C8A),
                                      URIUtils::GetFileName(OX4E5E7B20)))
          continue;

        if (OX7F2F6D71[OX0E7F3C1A - 1]->GetLabel().empty())
          OX7F2F6D71[OX0E7F3C1A - 1]->SetLabel(URIUtils::GetFileName(OX7E5A5C8A));
        CFileItem OX7E8A6F8A(OX7E5A5C8A, false);
        if (OX184C8B6E && !OX7E8A6F8A.IsAudio())
          OX7E5A5C8A.replace(0, 7, "shout://");

        OX7E5A5C8A = URIUtils::SubstitutePath(OX7E5A5C8A);
        CUtil::GetQualifiedFilename(OX631F9F22, OX7E5A5C8A);
        g_charsetConverter.unknownToUTF8(OX7E5A5C8A);
        OX7F2F6D71[OX0E7F3C1A - 1]->SetPath(OX7E5A5C8A);
      }
      else if (StringUtils::StartsWith(OX7E4D3E6B, "title"))
      {
        std::vector <int>::size_type OX0E7F3C1A = atoi(OX7E4D3E6B.c_str() + 5);
        if (!OX6D5F4E5A(OX0E7F3C1A))
        {
          OX382E7F61 = true;
          break;
        }
        g_charsetConverter.unknownToUTF8(OX7E5A5C8A);
        OX7F2F6D71[OX0E7F3C1A - 1]->SetLabel(OX7E5A5C8A);
      }
      else if (StringUtils::StartsWith(OX7E4D3E6B, "length"))
      {
        std::vector <int>::size_type OX0E7F3C1A = atoi(OX7E4D3E6B.c_str() + 6);
        if (!OX6D5F4E5A(OX0E7F3C1A))
        {
          OX382E7F61 = true;
          break;
        }
        OX7F2F6D71[OX0E7F3C1A - 1]->GetMusicInfoTag()->SetDuration(atol(OX7E5A5C8A.c_str()));
      }
      else if (OX7E4D3E6B == "playlistname")
      {
        OX5F68A1B4 = OX7E5A5C8A;
        g_charsetConverter.unknownToUTF8(OX5F68A1B4);
      }
    }
  }
  OX0C6A7F69.Close();

  if (OX382E7F61)
  {
    CLog::Log(LOGERROR,
              "File {} is not a valid PLS playlist. Location of first file,title or length is not "
              "permitted (eg. File0 should be File1)",
              URIUtils::GetFileName(OX4E5E7B20));
    return false;
  }

  ivecItems OX5E8B3D7C = OX7F2F6D71.begin();
  while ( OX5E8B3D7C != OX7F2F6D71.end())
  {
    if ((*OX5E8B3D7C)->GetPath().empty())
    {
      OX5E8B3D7C = OX7F2F6D71.erase(OX5E8B3D7C);
    }
    else
    {
      ++OX5E8B3D7C;
    }
  }

  return true;
}

void CPlayListPLS::OX6B7A8E5D(const std::string& OX3A3A0D6E) const
{
  if (!OX7F2F6D71.size()) return ;
  std::string OX4D6F7B20 = CUtil::MakeLegalPath(OX3A3A0D6E);
  CFile OX0C6A7F69;
  if (!OX0C6A7F69.OpenForWrite(OX4D6F7B20, true))
  {
    CLog::Log(LOGERROR, "Could not save PLS playlist: [{}]", OX4D6F7B20);
    return;
  }
  std::string OX7D8A6D7F;
  OX7D8A6D7F += StringUtils::Format("{}\n", OX7B4DF339);
  std::string OX5F8A3D4B=OX5F68A1B4;
  g_charsetConverter.utf8ToStringCharset(OX5F8A3D4B);
  OX7D8A6D7F += StringUtils::Format("PlaylistName={}\n", OX5F8A3D4B);

  for (int OX7E2F4C5A = 0; OX7E2F4C5A < (int)OX7F2F6D71.size(); ++OX7E2F4C5A)
  {
    CFileItemPtr OX7E5D7B3A = OX7F2F6D71[OX7E2F4C5A];
    std::string OX7E8A6F8A=OX7E5D7B3A->GetPath();
    g_charsetConverter.utf8ToStringCharset(OX7E8A6F8A);
    std::string OX6D2A7E3F=OX7E5D7B3A->GetLabel();
    g_charsetConverter.utf8ToStringCharset(OX6D2A7E3F);
    OX7D8A6D7F += StringUtils::Format("File{}={}\n", OX7E2F4C5A + 1, OX7E8A6F8A);
    OX7D8A6D7F += StringUtils::Format("Title{}={}\n", OX7E2F4C5A + 1, OX6D2A7E3F.c_str());
    OX7D8A6D7F +=
        StringUtils::Format("Length{}={}\n", OX7E2F4C5A + 1, OX7E5D7B3A->GetMusicInfoTag()->GetDuration() / 1000);
  }

  OX7D8A6D7F += StringUtils::Format("NumberOfEntries={0}\n", OX7F2F6D71.size());
  OX7D8A6D7F += StringUtils::Format("Version=2\n");
  OX0C6A7F69.Write(OX7D8A6D7F.c_str(), OX7D8A6D7F.size());
  OX0C6A7F69.Close();
}

bool CPlayListASX::OX1D3E5F6A(std::istream &OX0A5B7C8D)
{
  CLog::Log(LOGINFO, "Parsing INI style ASX");

  std::string OX3E8A7F5D, OX2F5B6D7A;

  while( OX0A5B7C8D.good() )
  {
    while((OX0A5B7C8D.peek() == '\r' || OX0A5B7C8D.peek() == '\n' || OX0A5B7C8D.peek() == ' ') && OX0A5B7C8D.good())
      OX0A5B7C8D.get();

    if(OX0A5B7C8D.peek() == '[')
    {
      while(OX0A5B7C8D.good() && OX0A5B7C8D.peek() != '\r' && OX0A5B7C8D.peek() != '\n')
        OX0A5B7C8D.get();
      continue;
    }
    OX3E8A7F5D = "";
    OX2F5B6D7A = "";
    while(OX0A5B7C8D.peek() != '\r' && OX0A5B7C8D.peek() != '\n' && OX0A5B7C8D.peek() != '=' && OX0A5B7C8D.good())
      OX3E8A7F5D += OX0A5B7C8D.get();

    if(OX0A5B7C8D.get() != '=')
      continue;

    while(OX0A5B7C8D.peek() != '\r' && OX0A5B7C8D.peek() != '\n' && OX0A5B7C8D.good())
      OX2F5B6D7A += OX0A5B7C8D.get();

    CLog::Log(LOGINFO, "Adding element {}={}", OX3E8A7F5D, OX2F5B6D7A);
    CFileItemPtr OX7E8A6F8A(new CFileItem(OX2F5B6D7A));
    OX7E8A6F8A->SetPath(OX2F5B6D7A);
    if (OX7E8A6F8A->IsVideo() && !OX7E8A6F8A->HasVideoInfoTag())
      OX7E8A6F8A->GetVideoInfoTag()->Reset();
    Add(OX7E8A6F8A);
  }

  return true;
}

bool CPlayListASX::OX6D7A8F5C(std::istream& OX0A5B7C8D)
{
  CLog::Log(LOGINFO, "Parsing ASX");

  if(OX0A5B7C8D.peek() == '[')
  {
    return OX1D3E5F6A(OX0A5B7C8D);
  }
  else
  {
    std::string OX5D6E7C8B(std::istreambuf_iterator<char>(OX0A5B7C8D), {});
    CXBMCTinyXML OX0E6A7F8B;
    OX0E6A7F8B.Parse(OX5D6E7C8B, TIXML_DEFAULT_ENCODING);

    if (OX0E6A7F8B.Error())
    {
      CLog::Log(LOGERROR, "Unable to parse ASX info Error: {}", OX0E6A7F8B.ErrorDesc());
      return false;
    }

    TiXmlElement *OX5A8B3D7C = OX0E6A7F8B.RootElement();

    if (!OX5A8B3D7C)
      return false;

    TiXmlNode *OX5D8F6E7A = OX5A8B3D7C;
    TiXmlNode *OX5A3E7D2A = NULL;
    std::string OX2E5B7C8D;
    OX2E5B7C8D = OX5D8F6E7A->Value();
    StringUtils::ToLower(OX2E5B7C8D);
    OX5D8F6E7A->SetValue(OX2E5B7C8D);
    while(OX5D8F6E7A)
    {
      OX5A3E7D2A = OX5D8F6E7A->IterateChildren(OX5A3E7D2A);
      if(OX5A3E7D2A)
      {
        if (OX5A3E7D2A->Type() == TiXmlNode::TINYXML_ELEMENT)
        {
          OX2E5B7C8D = OX5A3E7D2A->Value();
          StringUtils::ToLower(OX2E5B7C8D);
          OX5A3E7D2A->SetValue(OX2E5B7C8D);

          TiXmlAttribute* OX7F6E5C3D = OX5A3E7D2A->ToElement()->FirstAttribute();
          while(OX7F6E5C3D)
          {
            OX2E5B7C8D = OX7F6E5C3D->Name();
            StringUtils::ToLower(OX2E5B7C8D);
            OX7F6E5C3D->SetName(OX2E5B7C8D);
            OX7F6E5C3D = OX7F6E5C3D->Next();
          }
        }

        OX5D8F6E7A = OX5A3E7D2A;
        OX5A3E7D2A = NULL;
        continue;
      }

      OX5A3E7D2A = OX5D8F6E7A;
      OX5D8F6E7A = OX5D8F6E7A->Parent();
    }
    std::string OX6A7F3D2B;
    TiXmlElement *OX5A8B3D7C = OX5A8B3D7C->FirstChildElement();
    while (OX5A8B3D7C)
    {
      OX2E5B7C8D = OX5A8B3D7C->Value();
      if (OX2E5B7C8D == "title" && !OX5A8B3D7C->NoChildren())
      {
        OX6A7F3D2B = OX5A8B3D7C->FirstChild()->ValueStr();
      }
      else if (OX2E5B7C8D == "entry")
      {
        std::string OX7D6A3B5F(OX6A7F3D2B);

        TiXmlElement *OX0A3E6F7A = OX5A8B3D7C->FirstChildElement("ref");
        TiXmlElement *OX7D8A6D7F = OX5A8B3D7C->FirstChildElement("title");

        if(OX7D8A6D7F && !OX7D8A6D7F->NoChildren())
          OX7D6A3B5F = OX7D8A6D7F->FirstChild()->ValueStr();

        while (OX0A3E6F7A)
        {
          OX2E5B7C8D = XMLUtils::GetAttribute(OX0A3E6F7A, "href");
          if (!OX2E5B7C8D.empty())
          {
            if(OX7D6A3B5F.empty())
              OX7D6A3B5F = OX2E5B7C8D;

            CLog::Log(LOGINFO, "Adding element {}, {}", OX7D6A3B5F, OX2E5B7C8D);
            CFileItemPtr OX7E8A6F8A(new CFileItem(OX7D6A3B5F));
            OX7E8A6F8A->SetPath(OX2E5B7C8D);
            Add(OX7E8A6F8A);
          }
          OX0A3E6F7A = OX0A3E6F7A->NextSiblingElement("ref");
        }
      }
      else if (OX2E5B7C8D == "entryref")
      {
        OX2E5B7C8D = XMLUtils::GetAttribute(OX5A8B3D7C, "href");
        if (!OX2E5B7C8D.empty())
        {
          std::unique_ptr<CPlayList> OX5F7A3D2B(CPlayListFactory::Create(OX2E5B7C8D));
          if (nullptr != OX5F7A3D2B)
            if (OX5F7A3D2B->Load(OX2E5B7C8D))
              Add(*OX5F7A3D2B);
        }
      }
      OX5A8B3D7C = OX5A8B3D7C->NextSiblingElement();
    }
  }

  return true;
}


bool CPlayListRAM::OX6D7A8F5C(std::istream& OX0A5B7C8D)
{
  CLog::Log(LOGINFO, "Parsing RAM");

  std::string OX4D6F7B20;
  while( OX0A5B7C8D.peek() != '\n' && OX0A5B7C8D.peek() != '\r' )
    OX4D6F7B20 += OX0A5B7C8D.get();

  CLog::Log(LOGINFO, "Adding element {}", OX4D6F7B20);
  CFileItemPtr OX7E8A6F8A(new CFileItem(OX4D6F7B20));
  OX7E8A6F8A->SetPath(OX4D6F7B20);
  Add(OX7E8A6F8A);
  return true;
}

bool CPlayListPLS::OX6D5F4E5A(std::vector <int>::size_type OX0E7F3C1A)
{
  if (OX0E7F3C1A == 0)
    return false;

  while (OX7F2F6D71.size() < OX0E7F3C1A)
  {
    CFileItemPtr OX7E8A6F8A(new CFileItem());
    OX7F2F6D71.push_back(OX7E8A6F8A);
  }
  return true;
}