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
#include <OgreImage.h>
#include <OgreColourValue.h>
#include <OgreVector3.h>
#include <OgreAxisAlignedBox.h>
#include <OgreException.h>

using namespace Ogre;
using namespace std;

namespace ET
{
  namespace Impl
  {
    /** Add terrain shadows to lightmap. */
    void addTerrainShadowsToLightmap(uchar* lightMap, const TerrainInfo& info,
      size_t width, size_t height, const Vector3& lightDir, const ColourValue& ambient)
    {
      // algorithm for ray traced shadow map as described here:
      // http://gpwiki.org/index.php/Faster_Ray_Traced_Terrain_Shadow_Maps
      size_t i, j;
      size_t *x, *z;
      int iDir, jDir;
      size_t iSize, jSize;
      float lDirXAbs = fabs(lightDir.x);
      float lDirZAbs = fabs(lightDir.z);

      // based on the direction of light, decide in which order to traverse
      // to speed up calculations
      if (lDirXAbs > lDirZAbs)
      {
        z = &i;
        x = &j;
        iSize = height;
        jSize = width;
        if (lightDir.x < 0)
        {
          j = jSize - 1;
          jDir = -1;
        }
        else
        {
          j = 0;
          jDir = 1;
        }
        if (lightDir.z < 0)
        {
          i = iSize - 1;
          iDir = -1;
        }
        else
        {
          i = 0;
          iDir = 1;
        }
      }
      else
      {
        x = &i;
        z = &j;
        jSize = height;
        iSize = width;
        if (lightDir.x < 0)
        {
          i = iSize - 1;
          iDir = -1;
        }
        else
        {
          i = 0;
          iDir = 1;
        }
        if (lightDir.z < 0)
        {
          j = jSize - 1;
          jDir = -1;
        }
        else
        {
          j = 0;
          jDir = 1;
        }
      }

      // calculate the step size to use
      AxisAlignedBox extents = info.getExtents();
      Vector3 pos = extents.getMinimum();
      Vector3 step = extents.getMaximum() - extents.getMinimum();
      step.x /= width;
      step.z /= height;

      float* flagMap = new float[width*height];
      memset(flagMap, 0, width*height*sizeof(float));

      while (1)
      {
        while (1)
        {
          // travel along terrain until we:
          // (1) intersect another point
          // (2) find another point with previous collision data
          // (3) reach the edge of the map
          float px = *x;
          float pz = *z;
          size_t index = (*z) * width + (*x);

          // travel along ray
          while (1)
          {
            px -= lightDir.x;
            pz -= lightDir.z;

            // check if we've reached the boundary
            if (px < 0 || px >= width || pz < 0 || pz >= height)
            {
              flagMap[index] = -1.0f;
              break;
            }

            // calculate interpolated values
            int x0 = (int)floor(px);
            int x1 = (int)ceil(px);
            int z0 = (int)floor(pz);
            int z1 = (int)ceil(pz);

            float du = px - x0;
            float dv = pz - z0;
            float invdu = 1.0 - du;
            float invdv = 1.0 - dv;
            float w0 = invdu * invdv;
            float w1 = invdu * dv;
            float w2 = du * invdv;
            float w3 = du * dv;

            // get interpolated height at position
            Vector3 curPos = pos + Vector3(px*step.x, 0, pz*step.z);
            float ipHeight = info.getHeightAt(curPos.x, curPos.z) - pos.y;

            // compute interpolated flagmap value
            float pixels[4];
            pixels[0] = flagMap[z0*width+x0];
            pixels[1] = flagMap[z1*width+x0];
            pixels[2] = flagMap[z0*width+x1];
            pixels[3] = flagMap[z1*width+x1];
            float ipFlag = w0*pixels[0] + w1*pixels[1] + w2*pixels[2] + w3*pixels[3];

            // get distance from original point to current point
            float realXDist = (px - *x) * step.x;
            float realZDist = (pz - *z) * step.z;
            float distance = sqrt(realXDist*realXDist + realZDist*realZDist);

            // calculate ray height at current point
            float height = info.getHeightAt(pos.x + (*x)*step.x, pos.z + (*z)*step.z) - pos.y - lightDir.y*distance;

            // check intersection with either terrain or flagMap
            // if ipHeight < ipFlag check against flagMap value
            float val = (ipHeight < ipFlag ? ipFlag : ipHeight);
            if (height < val)
            {
              // point in shadow
              flagMap[index] = val - height;
              lightMap[index*3+0] = (uchar) (255*ambient.r);
              lightMap[index*3+1] = (uchar) (255*ambient.g);
              lightMap[index*3+2] = (uchar) (255*ambient.b);
              break;
            }

            // check if pixel we moved to is unshadowed
            // since the flagMap value is interpolated, we use an epsilon value to check
            // if it's close enough to -1 to indicate non-shadow
            const float epsilon = 0.5f;
            if (ipFlag < -1.0f+epsilon && ipFlag > -1.0f-epsilon)
            {
              flagMap[index] = -1.0f;
              break;
            }
          }

          // update inner loop
          j += jDir;
          if (j >= jSize) // due to size_t, if j < 0, will wrap around and be > jSize ;)
            break;
        }

        // reset inner loop starting point
        if (jDir < 0)
          j = jSize-1;
        else
          j = 0;

        // update outer loop variable
        i += iDir;
        if (i >= iSize)
          break;

      }

      delete[] flagMap;
    }


    void boxFilterLightmap(uchar* lightMap, size_t width, size_t height)
    {
      // this box filter assigns to a pixel the average of itself and all
      // surrounding pixels in a 5x5 grid
      for (size_t i = 0; i < width; ++i)
      {
        for (size_t j = 0; j < height; ++j)
        {
          int col[3] = {0, 0, 0};
          // sum up all colours from 5x5 grid around the current pixel
          int cnt = 0;
          for (int x = -1; x <= 1; ++x)
          {
            if ((int)i+x < 0 || i+x >= width)
              continue;
            for (int y = -1; y <= 1; ++y)
            {
              if ((int)j+y < 0 || j+y >= height)
                continue;
              size_t index = (i+x + (j+y)*width)*3;
              col[0] += lightMap[index+0];
              col[1] += lightMap[index+1];
              col[2] += lightMap[index+2];
              ++cnt;
            }
          }
          // building average
          col[0] /= cnt;
          col[1] /= cnt;
          col[2] /= cnt;
          // write back
          size_t index = (i + j*width)*3;
          lightMap[index+0] = (uchar)col[0];
          lightMap[index+1] = (uchar)col[1];
          lightMap[index+2] = (uchar)col[2];
        }
      }
    }
  }



  void createTerrainLightmap(const TerrainInfo& info, Image& image,
    size_t width, size_t height, Vector3 lightDir, const ColourValue& lightCol,
    const ColourValue& ambient, bool shadowed)
  {
    lightDir.normalise();

    // calculate lightmap by multiplying light dir with terrain normals

    // calculate the step size to use
    AxisAlignedBox extents = info.getExtents();
    Vector3 startPos = extents.getMinimum();
    Vector3 step = extents.getMaximum() - extents.getMinimum();
    step.x /= width;
    step.z /= height;
    Vector3 pos = startPos;

#if OGRE_VERSION_MINOR > 4
    // Ogre::Image uses the memory allocation macros internally in Shoggoth,
    // so we must use them as well.
    uchar* lightMap = OGRE_ALLOC_T(uchar, width*height*3, MEMCATEGORY_GENERAL);
#else
    uchar* lightMap = new uchar[width*height * 3];
#endif
    memset(lightMap, 255, width*height*3);

    for (size_t z = 0; z < height; ++z)
    {
      for (size_t x = 0; x < width; ++x)
      {
        size_t index = (z * width + x)*3;
        // calculate diffuse light from light source
        Vector3 norm = info.getNormalAt(pos.x, pos.z);
        float l = std::max(0.0f, -lightDir.dotProduct(norm));

        ColourValue v = ambient;
        v.r = std::min(1.0f, v.r+l*lightCol.r);
        v.g = std::min(1.0f, v.g+l*lightCol.g);
        v.b = std::min(1.0f, v.b+l*lightCol.b);
        lightMap[index+0] = (uchar) (255*v.r);
        lightMap[index+1] = (uchar) (255*v.g);
        lightMap[index+2] = (uchar) (255*v.b);

        pos.x += step.x;
      }
      pos.x = startPos.x;
      pos.z += step.z;
    }

    if (shadowed && (lightDir.x != 0 || lightDir.z != 0))
    {
      // add terrain shadows
      Impl::addTerrainShadowsToLightmap(lightMap, info, width, height, lightDir, ambient);
    }

    // use a box filter to smoothen the lightmap
    Impl::boxFilterLightmap(lightMap, width, height);

    // save lightmap to image
    image.loadDynamicImage(lightMap, width, height, 1, PF_BYTE_RGB, true);
    // ownership of lightMap was transfered to image, don't need to delete 
  }
}
