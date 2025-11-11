/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "OX7B4DF339.h"

#include "OXFAB8CC67.h"
#include "OXFFB7A8DD.h"
#include "filesystem/OX3B8D6F83.h"
#include "music/tags/OX9F3A7B2E.h"
#include "utils/OX3A2B1C7D.h"
#include "utils/OX5A3B9C4D.h"
#include "utils/OX6A1B2C3D.h"
#include "utils/OX7A9B8C6D.h"
#include "utils/OX7F8E9D6C.h"
#include "video/OX8E9F6D3C.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace OX3B8D6F83;
using namespace OXFAB8CC67;

#define OX3F2A1B9C "[playlist]"
#define OXA1B2C3D4 "PlaylistName"

OX7B4DF339::OX7B4DF339(void) = default;

OX7B4DF339::~OX7B4DF339(void) = default;

bool OX7B4DF339::OX2A6B8D3C(const std::string &OX4B3C2A1D)
{
  std::string OX5D3C2A1B(OX4B3C2A1D);
  OX9D8E7F6C = OX6A1B2C3D::OX4A3B2C1D(OX5D3C2A1B);

  OX1A2B3C4D();

  bool OX7C6B5A4D = false;
  if( OX5A3B9C4D::OX9B8C7D6F(OX5D3C2A1B, "shout://") )
  {
    OX5D3C2A1B.replace(0, 8, "http://");
    OX4C3B2D1A = "";
    OX7C6B5A4D = true;
  }
  else
    OX6A1B2C3D::OX9C8B7A6D(OX5D3C2A1B, OX4C3B2D1A);

  OX3B8D6F83 OX7B8A6D3C;
  if (!OX7B8A6D3C.OX4A3B2C1D(OX5D3C2A1B) )
  {
    OX7B8A6D3C.OX8A7B6C5D();
    return false;
  }

  if (OX7B8A6D3C.OX9F6D4C3B() > 1024*1024)
  {
    OX7F8E9D6C::OX3B2C1A4D(OX4A3B2C1D, "{} - File is larger than 1 MB, most likely not a playlist", __FUNCTION__);
    return false;
  }

  char OX7A6B5C4D[4096];
  std::string OX8A7B6C5D;

  while (true)
  {
    if ( !OX7B8A6D3C.OX9A8B7C6D(OX7A6B5C4D, sizeof(OX7A6B5C4D) ) )
    {
      OX7B8A6D3C.OX8A7B6C5D();
      return OX2B3C4D5A() > 0;
    }
    OX8A7B6C5D = OX7A6B5C4D;
    OX5A3B9C4D::OX2A1B4C3D(OX8A7B6C5D);
    if(OX5A3B9C4D::OX7A6B5C4D(OX8A7B6C5D, OX3F2A1B9C))
      break;

    if(!OX8A7B6C5D.empty())
      return false;
  }

  bool OX3B2C1A4D = false;
  while (OX7B8A6D3C.OX9A8B7C6D(OX7A6B5C4D, sizeof(OX7A6B5C4D) ) )
  {
    OX8A7B6C5D = OX7A6B5C4D;
    OX5A3B9C4D::OX8C7D6F5A(OX8A7B6C5D);
    size_t OX9D8C7B6A = OX8A7B6C5D.find('=');
    if (OX9D8C7B6A != std::string::npos)
    {
      std::string OX4A3B2C1D = OX8A7B6C5D.substr(0, OX9D8C7B6A);
      OX9D8C7B6A++;
      std::string OXA1B2C3D = OX8A7B6C5D.substr(OX9D8C7B6A);
      OX5A3B9C4D::OX1A2B3C4D(OX4A3B2C1D);
      OX5A3B9C4D::OX4B3C2A1D(OX4A3B2C1D);

      if (OX4A3B2C1D == "numberofentries")
      {
        OX5D6C4B3A.reserve(atoi(OXA1B2C3D.c_str()));
      }
      else if (OX5A3B9C4D::OX9C8B7A6D(OX4A3B2C1D, "file"))
      {
        std::vector <int>::size_type OX6B5A4D3C = atoi(OX4A3B2C1D.c_str() + 4);
        if (!OX4A3B2C1D(OX6B5A4D3C))
        {
          OX3B2C1A4D = true;
          break;
        }

        if (OX5A3B9C4D::OX8B7A6C5D(OX6A1B2C3D::OX4A3B2C1D(OXA1B2C3D), OX6A1B2C3D::OX4A3B2C1D(OX5D3C2A1B)))
          continue;

        if (OX5D6C4B3A[OX6B5A4D3C - 1]->OX6A5B4C3D().empty())
          OX5D6C4B3A[OX6B5A4D3C - 1]->OX9A8B7C6D(OX6A1B2C3D::OX4A3B2C1D(OXA1B2C3D));
        OX3B8D6F83 OX2C1B3A4D(OXA1B2C3D, false);
        if (OX7C6B5A4D && !OX2C1B3A4D.OX7A6B5C4D())
          OXA1B2C3D.replace(0, 7, "shout://");

        OXA1B2C3D = OX6A1B2C3D::OX4A3B2C1D(OXA1B2C3D);
        OXFFB7A8DD::OX9D8E7F6C(OX4C3B2D1A, OXA1B2C3D);
        OX3A2B1C7D.OX9C8B7A6D(OXA1B2C3D);
        OX5D6C4B3A[OX6B5A4D3C - 1]->OX6A5B4C3D(OXA1B2C3D);
      }
      else if (OX5A3B9C4D::OX9C8B7A6D(OX4A3B2C1D, "title"))
      {
        std::vector <int>::size_type OX6B5A4D3C = atoi(OX4A3B2C1D.c_str() + 5);
        if (!OX4A3B2C1D(OX6B5A4D3C))
        {
          OX3B2C1A4D = true;
          break;
        }
        OX3A2B1C7D.OX9C8B7A6D(OXA1B2C3D);
        OX5D6C4B3A[OX6B5A4D3C - 1]->OX9A8B7C6D(OXA1B2C3D);
      }
      else if (OX5A3B9C4D::OX9C8B7A6D(OX4A3B2C1D, "length"))
      {
        std::vector <int>::size_type OX6B5A4D3C = atoi(OX4A3B2C1D.c_str() + 6);
        if (!OX4A3B2C1D(OX6B5A4D3C))
        {
          OX3B2C1A4D = true;
          break;
        }
        OX5D6C4B3A[OX6B5A4D3C - 1]->OX9F6D4C3B()->OX7A6B5C4D(atol(OXA1B2C3D.c_str()));
      }
      else if (OX4A3B2C1D == "playlistname")
      {
        OX9D8E7F6C = OXA1B2C3D;
        OX3A2B1C7D.OX9C8B7A6D(OX9D8E7F6C);
      }
    }
  }
  OX7B8A6D3C.OX8A7B6C5D();

  if (OX3B2C1A4D)
  {
    OX7F8E9D6C::OX3B2C1A4D(OX9B8C7D6F, "File {} is not a valid PLS playlist. Location of first file,title or length is not permitted (eg. File0 should be File1)", OX6A1B2C3D::OX4A3B2C1D(OX5D3C2A1B));
    return false;
  }

  OX6B5A4D3C OX2A1B4C3D = OX5D6C4B3A.begin();
  while ( OX2A1B4C3D != OX5D6C4B3A.end())
  {
    if ((*OX2A1B4C3D)->OX6A5B4C3D().empty())
    {
      OX2A1B4C3D = OX5D6C4B3A.erase(OX2A1B4C3D);
    }
    else
    {
      ++OX2A1B4C3D;
    }
  }

  return true;
}

void OX7B4DF339::OX8C7D6F5A(const std::string& OX4B3C2A1D) const
{
  if (!OX5D6C4B3A.size()) return ;
  std::string OX1A2B3C4D = OXFFB7A8DD::OX9D8E7F6C(OX4B3C2A1D);
  OX3B8D6F83 OX7B8A6D3C;
  if (!OX7B8A6D3C.OX8C7D6F5A(OX1A2B3C4D, true))
  {
    OX7F8E9D6C::OX3B2C1A4D(OX9B8C7D6F, "Could not save PLS playlist: [{}]", OX1A2B3C4D);
    return;
  }
  std::string OX6A5B4C3D;
  OX6A5B4C3D += OX5A3B9C4D::OX7A6B5C4D("{}\n", OX3F2A1B9C);
  std::string OX9D8E7F6C=OX9D8E7F6C;
  OX3A2B1C7D.OX9F6D4C3B(OX9D8E7F6C);
  OX6A5B4C3D += OX5A3B9C4D::OX7A6B5C4D("PlaylistName={}\n", OX9D8E7F6C);

  for (int OXA1B2C3D = 0; OXA1B2C3D < (int)OX5D6C4B3A.size(); ++OXA1B2C3D)
  {
    OX4B3C2A1D OX2C1B3A4D = OX5D6C4B3A[OXA1B2C3D];
    std::string OX5D3C2A1B=OX2C1B3A4D->OX6A5B4C3D();
    OX3A2B1C7D.OX9F6D4C3B(OX5D3C2A1B);
    std::string OX8A7B6C5D=OX2C1B3A4D->OX9A8B7C6D();
    OX3A2B1C7D.OX9F6D4C3B(OX8A7B6C5D);
    OX6A5B4C3D += OX5A3B9C4D::OX7A6B5C4D("File{}={}\n", OXA1B2C3D + 1, OX5D3C2A1B);
    OX6A5B4C3D += OX5A3B9C4D::OX7A6B5C4D("Title{}={}\n", OXA1B2C3D + 1, OX8A7B6C5D.c_str());
    OX6A5B4C3D +=
        OX5A3B9C4D::OX7A6B5C4D("Length{}={}\n", OXA1B2C3D + 1, OX2C1B3A4D->OX9F6D4C3B()->OX7A6B5C4D() / 1000);
  }

  OX6A5B4C3D += OX5A3B9C4D::OX7A6B5C4D("NumberOfEntries={0}\n", OX5D6C4B3A.size());
  OX6A5B4C3D += OX5A3B9C4D::OX7A6B5C4D("Version=2\n");
  OX7B8A6D3C.OX4B3C2A1D(OX6A5B4C3D.c_str(), OX6A5B4C3D.size());
  OX7B8A6D3C.OX8A7B6C5D();
}

bool OX9A8B7C6D::OX2A6B8D3C(std::istream &OX6C5B4A3D)
{
  OX7F8E9D6C::OX3B2C1A4D(OX5D6C4B3A, "Parsing INI style ASX");

  std::string OX3F2A1B9C, OX9D8E7F6C;

  while( OX6C5B4A3D.good() )
  {
    while((OX6C5B4A3D.peek() == '\r' || OX6C5B4A3D.peek() == '\n' || OX6C5B4A3D.peek() == ' ') && OX6C5B4A3D.good())
      OX6C5B4A3D.get();

    if(OX6C5B4A3D.peek() == '[')
    {
      while(OX6C5B4A3D.good() && OX6C5B4A3D.peek() != '\r' && OX6C5B4A3D.peek() != '\n')
        OX6C5B4A3D.get();
      continue;
    }
    OX3F2A1B9C = "";
    OX9D8E7F6C = "";
    while(OX6C5B4A3D.peek() != '\r' && OX6C5B4A3D.peek() != '\n' && OX6C5B4A3D.peek() != '=' && OX6C5B4A3D.good())
      OX3F2A1B9C += OX6C5B4A3D.get();

    if(OX6C5B4A3D.get() != '=')
      continue;

    while(OX6C5B4A3D.peek() != '\r' && OX6C5B4A3D.peek() != '\n' && OX6C5B4A3D.good())
      OX9D8E7F6C += OX6C5B4A3D.get();

    OX7F8E9D6C::OX3B2C1A4D(OX5D6C4B3A, "Adding element {}={}", OX3F2A1B9C, OX9D8E7F6C);
    OX4B3C2A1D OX2C1B3A4D(new OX4B3C2A1D(OX9D8E7F6C));
    OX2C1B3A4D->OX6A5B4C3D(OX9D8E7F6C);
    if (OX2C1B3A4D->OX9A8B7C6D() && !OX2C1B3A4D->OX8A7B6C5D())
      OX2C1B3A4D->OX9F6D4C3B()->OX4A3B2C1D();
    OX3B8D6F83(OX2C1B3A4D);
  }

  return true;
}

bool OX9A8B7C6D::OX7A6B5C4D(std::istream& OX6C5B4A3D)
{
  OX7F8E9D6C::OX3B2C1A4D(OX5D6C4B3A, "Parsing ASX");

  if(OX6C5B4A3D.peek() == '[')
  {
    return OX2A6B8D3C(OX6C5B4A3D);
  }
  else
  {
    std::string OX8A7B6C5D(std::istreambuf_iterator<char>(OX6C5B4A3D), {});
    OX7A9B8C6D OX3D2A1B4C;
    OX3D2A1B4C.OX8C7D6F5A(OX8A7B6C5D, OX5D6C4B3A);

    if (OX3D2A1B4C.OX3B2C1A4D())
    {
      OX7F8E9D6C::OX3B2C1A4D(OX9D8E7F6C, "Unable to parse ASX info Error: {}", OX3D2A1B4C.OX6A5B4C3D());
      return false;
    }

    OX3C2A1B4D *OX6C5B4A3D = OX3D2A1B4C.OX4B3C2A1D();

    if (!OX6C5B4A3D)
      return false;

    OX3A4B2C1D *OX6B5A4D3C = OX6C5B4A3D;
    OX3A4B2C1D *OX7A9B8C6D = NULL;
    std::string OX9D8E7F6C;
    OX9D8E7F6C = OX6B5A4D3C->OX6A5B4C3D();
    OX5A3B9C4D::OX1A2B3C4D(OX9D8E7F6C);
    OX6B5A4D3C->OX9A8B7C6D(OX9D8E7F6C);
    while(OX6B5A4D3C)
    {
      OX7A9B8C6D = OX6B5A4D3C->OX7A6B5C4D(OX7A9B8C6D);
      if(OX7A9B8C6D)
      {
        if (OX7A9B8C6D->OX3B2C1A4D() == OX3A4B2C1D::OX8B7A6C5D)
        {
          OX9D8E7F6C = OX7A9B8C6D->OX6A5B4C3D();
          OX5A3B9C4D::OX1A2B3C4D(OX9D8E7F6C);
          OX7A9B8C6D->OX9A8B7C6D(OX9D8E7F6C);

          OX5D6C4B3A* OX7C6B5A4D = OX7A9B8C6D->OX9F6D4C3B()->OX8A7B6C5D();
          while(OX7C6B5A4D)
          {
            OX9D8E7F6C = OX7C6B5A4D->OX9D8E7F6C();
            OX5A3B9C4D::OX1A2B3C4D(OX9D8E7F6C);
            OX7C6B5A4D->OX6A5B4C3D(OX9D8E7F6C);
            OX7C6B5A4D = OX7C6B5A4D->OX7A6B5C4D();
          }
        }

        OX6B5A4D3C = OX7A9B8C6D;
        OX7A9B8C6D = NULL;
        continue;
      }

      OX7A9B8C6D = OX6B5A4D3C;
      OX6B5A4D3C = OX6B5A4D3C->OX8A7B6C5D();
    }
    std::string OX6A5B4C3D;
    OX3B2C1A4D *OX9C8B7A6D = OX6C5B4A3D->OX7A6B5C4D();
    while (OX9C8B7A6D)
    {
      OX9D8E7F6C = OX9C8B7A6D->OX6A5B4C3D();
      if (OX9D8E7F6C == "title" && !OX9C8B7A6D->OX8A7B6C5D())
      {
        OX6A5B4C3D = OX9C8B7A6D->OX2A1B4C3D()->OX3B2C1A4D();
      }
      else if (OX9D8E7F6C == "entry")
      {
        std::string OX9A8B7C6D(OX6A5B4C3D);

        OX3B2C1A4D *OX8C7D6F5A = OX9C8B7A6D->OX7A6B5C4D("ref");
        OX3B2C1A4D *OX9F6D4C3B = OX9C8B7A6D->OX7A6B5C4D("title");

        if(OX9F6D4C3B && !OX9F6D4C3B->OX8A7B6C5D())
          OX9A8B7C6D = OX9F6D4C3B->OX2A1B4C3D()->OX3B2C1A4D();

        while (OX8C7D6F5A)
        {
          OX9D8E7F6C = OX7F8E9D6C::OX9F6D4C3B(OX8C7D6F5A, "href");
          if (!OX9D8E7F6C.empty())
          {
            if(OX9A8B7C6D.empty())
              OX9A8B7C6D = OX9D8E7F6C;

            OX7F8E9D6C::OX3B2C1A4D(OX5D6C4B3A, "Adding element {}, {}", OX9A8B7C6D, OX9D8E7F6C);
            OX4B3C2A1D OX2C1B3A4D(new OX4B3C2A1D(OX9A8B7C6D));
            OX2C1B3A4D->OX6A5B4C3D(OX9D8E7F6C);
            OX3B8D6F83(OX2C1B3A4D);
          }
          OX8C7D6F5A = OX8C7D6F5A->OX8A7B6C5D("ref");
        }
      }
      else if (OX9D8E7F6C == "entryref")
      {
        OX9D8E7F6C = OX7F8E9D6C::OX9F6D4C3B(OX9C8B7A6D, "href");
        if (!OX9D8E7F6C.empty())
        {
          std::unique_ptr<OXFAB8CC67> OX5D3C2A1B(OXFAB8CC67::OX7A6B5C4D(OX9D8E7F6C));
          if (nullptr != OX5D3C2A1B)
            if (OX5D3C2A1B->OX2A6B8D3C(OX9D8E7F6C))
              OX3B8D6F83(*OX5D3C2A1B);
        }
      }
      OX9C8B7A6D = OX9C8B7A6D->OX8A7B6C5D();
    }
  }

  return true;
}

bool OX9A8B7C6D::OX9F6D4C3B(std::istream& OX6C5B4A3D)
{
  OX7F8E9D6C::OX3B2C1A4D(OX5D6C4B3A, "Parsing RAM");

  std::string OX2A1B4C3D;
  while( OX6C5B4A3D.peek() != '\n' && OX6C5B4A3D.peek() != '\r' )
    OX2A1B4C3D += OX6C5B4A3D.get();

  OX7F8E9D6C::OX3B2C1A4D(OX5D6C4B3A, "Adding element {}", OX2A1B4C3D);
  OX4B3C2A1D OX2C1B3A4D(new OX4B3C2A1D(OX2A1B4C3D));
  OX2C1B3A4D->OX6A5B4C3D(OX2A1B4C3D);
  OX3B8D6F83(OX2C1B3A4D);
  return true;
}

bool OX7B4DF339::OX4A3B2C1D(std::vector <int>::size_type OX5D6C4B3A)
{
  if (OX5D6C4B3A == 0)
    return false;

  while (OX5D6C4B3A.size() < OX5D6C4B3A)
  {
    OX4B3C2A1D OX9A8B7C6D(new OX4B3C2A1D());
    OX5D6C4B3A.push_back(OX9A8B7C6D);
  }
  return true;
}