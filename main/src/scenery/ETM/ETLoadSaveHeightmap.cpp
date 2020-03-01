/*
EDITABLE TERRAIN MANAGER for Ogre
Copyright (C) 2007  Holger Frydrych <h.frydrych@gmx.de>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

As a special exception, you may use this file as part of a free software
library without restriction.  Specifically, if other files instantiate
templates or use macros or inline functions from this file, or you compile
this file and link it with other files to produce an executable, this
file does not by itself cause the resulting executable to be covered by
the GNU General Public License.  This exception does not however
invalidate any other reasons why the executable file might be covered by
the GNU General Public License.
*/

#include "ETTerrainInfo.h"

#include <OgreDataStream.h>
#include <OgreImage.h>
#include <OgreException.h>

#include <iostream>

using namespace Ogre;
using namespace std;
using Ogre::uint;
using Ogre::ushort;

namespace ET
{
  void loadHeightmapFromImage(TerrainInfo& info, const Image& image)
  {
    uint bpp = 0;
    bool flip = false;

    switch (image.getFormat())
    {
    case PF_BYTE_A: case PF_BYTE_L:
      bpp = 1; break;
    case PF_BYTE_LA: case PF_L16:
      bpp = 2; break;
    case PF_BYTE_RGB:
      bpp = 3; break;
    case PF_BYTE_BGR:
      bpp = 3; flip = true; break;
    case PF_BYTE_RGBA:
      bpp = 4; break;
    case PF_BYTE_BGRA:
      bpp = 4; flip = true; break;
    default:
      OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Can't use the given image's format.", "loadHeightmapFromImage");
    }

    size_t size = image.getWidth() * image.getHeight();
    unsigned int maxVal = (1 << (bpp*8)) - 1;
    std::vector<float> data (size);
    const uchar* imageData = image.getData();

    for (size_t i = 0; i < size; ++i)
    {
      uchar read[4] = {0, 0, 0, 0};
      // TODO: Make this big endian aware/compatible
      memcpy(read, imageData, bpp);
      imageData += bpp;
      if (flip)
        swap(read[0], read[2]);
      unsigned int val = * ((unsigned int*)read);
      data[i] = float(val) / maxVal;
    }

    info.setHeightmap(image.getWidth(), image.getHeight(), data);
  }


  void saveHeightmapToImage(const TerrainInfo& info, Ogre::Image& image, unsigned int bpp)
  {
    PixelFormat pf;
    // decide on the image format to use
    switch (bpp)
    {
    case 1:
      pf = PF_BYTE_L; break;
    case 2:
      pf = PF_L16; break;
    case 3:
      pf = PF_BYTE_RGB; break;
    case 4:
      pf = PF_BYTE_RGBA; break;
    default:
      OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Bpp must be between 1 and 4.", "saveHeightmapToImage");
    }

    uint maxVal = (1 << (bpp*8)) - 1;

#if OGRE_VERSION_MINOR > 4
    uchar* data = OGRE_ALLOC_T(uchar, info.getWidth()*info.getHeight()*bpp, MEMCATEGORY_GENERAL);
#else
    uchar* data = new uchar[info.getWidth()*info.getHeight()*bpp];
#endif
    uchar* pos = data;

    // fill data array
    for (size_t j = 0; j < info.getHeight(); ++j)
    {
      for (size_t i = 0; i < info.getWidth(); ++i)
      {
        uint val = uint (maxVal * info.at(i, j));
        memcpy(pos, &val, bpp);
        pos += bpp;
      }
    }

    image.loadDynamicImage(data, info.getWidth(), info.getHeight(), 1, pf, true);
    // given ownership of data to image
  }




  void loadHeightmapFromRawData(TerrainInfo& info, DataStream& stream, size_t width, size_t height)
  {
    size_t size = width*height;
    size_t bpp = stream.size() / size;
    if (bpp < 1 || bpp > 4 || stream.size() % size != 0)
      OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Size of the given data stream does not match with specified dimensions.", "loadHeightmapFromRawData");

    unsigned int maxVal = (1 << (bpp*8)) - 1;
    std::vector<float> data (size);

    for (size_t i = 0; i < size; ++i)
    {
      unsigned int val = 0;
      // TODO: What about big endian compatibility?
      stream.read((void*)&val, bpp);
      data[i] = float(val) / maxVal;
    }

    info.setHeightmap(width, height, data);
  }


  void saveHeightmapToRawData(const TerrainInfo& info, ostream& stream, uint bpp)
  {
    if (bpp < 1 || bpp > 4)
      OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Bpp must be between 1 and 4.", "saveHeightmapToRawData");

    uint maxVal = (1 << (bpp*8)) - 1;
    for (size_t j = 0; j < info.getHeight(); ++j)
    {
      for (size_t i = 0; i < info.getWidth(); ++i)
      {
        uint val = (uint) (maxVal * info.at(i, j));
        stream.write(reinterpret_cast<char*>(&val), bpp);
      }
    }
  }

}
