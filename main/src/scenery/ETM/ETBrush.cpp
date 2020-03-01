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

#include "ETBrush.h"

#include <OgreImage.h>
#include <OgreException.h>

using namespace std;
using namespace Ogre;
using Ogre::uint;
using Ogre::ushort;

namespace ET
{
  Brush::Brush()
  : mWidth(0), mHeight(0), mBrushArray(0)
  {
  }

  Brush::Brush(const float* brush, size_t width, size_t height)
  : mWidth(width), mHeight(height)
  {
    mBrushArray = new float[width*height];
    memcpy(mBrushArray, brush, width*height*sizeof(float));
  }

  Brush::Brush(const std::vector<float>& brush, size_t width, size_t height)
  : mWidth(width), mHeight(height)
  {
    mBrushArray = new float[width*height];
    copy(brush.begin(), brush.end(), mBrushArray);
  }

  Brush::Brush(const Brush& other)
  : mWidth(other.mWidth), mHeight(other.mHeight)
  {
    mBrushArray = new float[mWidth*mHeight];
    memcpy(mBrushArray, other.mBrushArray, mWidth*mHeight*sizeof(float));
  }

  Brush::~Brush()
  {
    delete[] mBrushArray;
  }


  Brush& Brush::operator=(const Brush& other)
  {
    Brush tmp (other);
    tmp.swap(*this);
    return *this;
  }

  void Brush::swap(Brush& other)
  {
    std::swap(mWidth, other.mWidth);
    std::swap(mHeight, other.mHeight);
    std::swap(mBrushArray, other.mBrushArray);
  }

  Brush loadBrushFromImage(const Image& image)
  {
    size_t width = image.getWidth();
    size_t height = image.getHeight();
    std::vector<float> brush (width*height);

    // determine the bytes per pixel used in the image
    int bpp = int(image.getSize() / (width*height));
    /*switch (image.getFormat())
    {
    case PF_BYTE_A: bpp = 1; break;
    case PF_BYTE_LA: case PF_L16: bpp = 2; break;
    case PF_BYTE_RGB: case PF_BYTE_BGR: bpp = 3; break;
    case PF_BYTE_RGBA: case PF_BYTE_BGRA: bpp = 4; break;
    default: OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Don't know what to do with the given image format, sorry.", "loadBrushFromImage");
    }*/
    // from the bpp, calculate the relevant max value for one pixel
    uint maxValue = (1 << (bpp*8)) - 1;

    // now fill the brush array
    const uchar* imageData = image.getData();
    for (size_t i = 0; i < brush.size(); ++i)
    {
      uint val = 0;
      memcpy(&val, imageData, bpp);
      imageData += bpp;
      brush[i] = float(val) / maxValue;
    }

    return Brush(brush, width, height);
  }


  void saveBrushToImage(const Brush& brush, Image& image)
  {
    // save brush as a 16bit grayscale image
#if OGRE_VERSION_MINOR > 4
    ushort* data = (ushort*)OGRE_ALLOC_T(uchar, brush.getWidth()*brush.getHeight()*sizeof(ushort), MEMCATEGORY_GENERAL);
#else
    ushort* data = (ushort*)new uchar[brush.getWidth()*brush.getHeight()*sizeof(ushort)];
#endif
    for (size_t x = 0; x < brush.getWidth(); ++x)
      for (size_t y = 0; y < brush.getHeight(); ++y)
        data[y*brush.getWidth() + x] = ushort(brush.at(x, y) * 0xffff);

    // pass the data to the image, image takes over ownership
    image.loadDynamicImage((uchar*)data, brush.getWidth(), brush.getHeight(), 1, PF_L16, true);
  }
}
