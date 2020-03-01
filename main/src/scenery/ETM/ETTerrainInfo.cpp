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
#include <Ogre.h>
#include <OgreAxisAlignedBox.h>
#include <OgreException.h>
#include <OgreRay.h>

using namespace Ogre;
using namespace std;

namespace ET
{
  namespace Impl
  {
    struct InfoImpl
    {
      /** extents of the heightmap */
      size_t width, height;
      /** the heightmap */
      std::vector<float> heightmap;

      /** offset at which the (0, 0) point of the terrain is placed in Ogre */
      Ogre::Vector3 offset;
      /** scale of a terrain vertex */
      Ogre::Vector3 scale;
    };
  }

  TerrainInfo::TerrainInfo()
  : mWidth(0), mHeight(0), mOffset(Ogre::Vector3::ZERO), mScale(Ogre::Vector3::UNIT_SCALE)
  {
  }

  TerrainInfo::TerrainInfo(size_t width, size_t height, const float* heightmap)
  : mWidth(0), mHeight(0), mOffset(Ogre::Vector3::ZERO), mScale(Ogre::Vector3::UNIT_SCALE)
  {
    setHeightmap(width, height, heightmap);
  }

  TerrainInfo::TerrainInfo(size_t width, size_t height, const std::vector<float>& heightmap)
  : mWidth(0), mHeight(0), mOffset(Ogre::Vector3::ZERO), mScale(Ogre::Vector3::UNIT_SCALE)
  {
    setHeightmap(width, height, heightmap);
  }


  void TerrainInfo::setHeightmap(size_t width, size_t height, const float* heightmap)
  {
    mHeightmap.assign(heightmap, heightmap + width*height);
    mWidth = width;
    mHeight = height;
  }

  void TerrainInfo::setHeightmap(size_t width, size_t height, const std::vector<float>& heightmap)
  {
    if (width*height != heightmap.size())
      OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Given heightmap dimensions and size of the std::vector do not match.", "TerrainInfo::setHeightmap");

    mHeightmap = heightmap;
    mWidth = width;
    mHeight = height;
  }


  void TerrainInfo::setExtents(const AxisAlignedBox& extents)
  {
    if (mWidth == 0)
      OGRE_EXCEPT(Exception::ERR_INVALID_STATE, "You must set a heightmap first.", "TerrainInfo::setExtents");

    mOffset = extents.getMinimum();
    mScale = extents.getMaximum() - extents.getMinimum();
    mScale.x /= mWidth;
    mScale.z /= mHeight;
  }

  AxisAlignedBox TerrainInfo::getExtents() const
  {
    if (mWidth == 0)
      OGRE_EXCEPT(Exception::ERR_INVALID_STATE, "You must set a heightmap first.", "TerrainInfo::setExtents");

    Ogre::Vector3 maximum = mScale;
    maximum.x *= mWidth;
    maximum.z *= mHeight;
    maximum += mOffset;
    return AxisAlignedBox(mOffset, maximum);
  }


  float TerrainInfo::getHeightAt(float x, float z) const
  {
    // scale down
    x -= mOffset.x;
    z -= mOffset.z;
    x /= mScale.x;
    z /= mScale.z;
    if (x < 0.0 || x > mWidth-1 || z < 0.0 || z > mHeight-1)
    {
      // out of bounds
      return mOffset.y - 1;
    }

    // retrieve height from heightmap via bilinear interpolation
    size_t xi = (size_t) x, zi = (size_t) z;
    float xpct = x - xi, zpct = z - zi;
    if (xi == mWidth-1)
    {
      // one too far
      --xi;
      xpct = 1.0f;
    }
    if (zi == mHeight-1)
    {
      --zi;
      zpct = 1.0f;
    }

    // retrieve heights
    float heights[4];
    heights[0] = at(xi, zi);
    heights[1] = at(xi, zi+1);
    heights[2] = at(xi+1, zi);
    heights[3] = at(xi+1, zi+1);

    // interpolate
    float w[4];
    w[0] = (1.0 - xpct) * (1.0 - zpct);
    w[1] = (1.0 - xpct) * zpct;
    w[2] = xpct * (1.0 - zpct);
    w[3] = xpct * zpct;
    float ipHeight = w[0]*heights[0] + w[1]*heights[1] + w[2]*heights[2] + w[3]*heights[3];

    // scale to actual height
    ipHeight *= mScale.y;
    ipHeight += mOffset.y;

    return ipHeight;
  }


  Ogre::Vector3 TerrainInfo::getNormalAt(float x, float z) const
  {
    int flip = 1;
    Ogre::Vector3 here (x, getHeightAt(x, z), z);
    Ogre::Vector3 left (x-1, getHeightAt(x-1, z), z);
    Ogre::Vector3 down (x, getHeightAt(x, z+1), z+1);
    if (left.x < 0.0)
    {
      flip *= -1;
      left = Ogre::Vector3(x+1, getHeightAt(x+1, z), z);
    }
    if (down.z >= mOffset.z + mScale.z*(mHeight-1))
    {
      flip *= -1;
      down = Ogre::Vector3(x, getHeightAt(x, z-1), z-1);
    }
    left -= here;
    down -= here;

    Ogre::Vector3 norm = flip * left.crossProduct(down);
    norm.normalise();

    return norm;
  }


  /** Addition from SongOfTheWeave */
  Ogre::Vector3 TerrainInfo::getTangentAt(float x, float z) const
  {
    Ogre::Vector3 v3Return;
    int flip = 1;
    Ogre::Vector3 here (x, getHeightAt(x, z), z);
    Ogre::Vector3 left (x - 1, getHeightAt(x - 1, z), z);
    if (left.x < 0.0)
    {
      flip *= -1;
      left = Ogre::Vector3(x + 1, getHeightAt(x + 1, z), z);
    }
    left -= here;
    v3Return = flip * left;
    v3Return.normalise();
    return v3Return;
  }


  std::pair<bool, Ogre::Vector3> TerrainInfo::rayIntersects(const Ray& ray) const
  {
    AxisAlignedBox box = getExtents();
    Ogre::Vector3 point = ray.getOrigin();
    Ogre::Vector3 dir = ray.getDirection();

    // first, does the ray start from inside the terrain extents?
    if (!box.contains(point))
    {
      // not inside the box, so let's see if we are actually
      // colliding with it
      pair<bool, Real> res = ray.intersects(box);
      if (!res.first)
        return make_pair(false, Ogre::Vector3::ZERO);
      // update point to the collision position
      point = ray.getPoint(res.second);
    }

    // now move along the ray until we intersect or leave the bounding box
    while (true)
    {
      // have we arived at or under the terrain height?
      // note that this approach means that ray queries from below won't work
      // correctly, but then again, that shouldn't be a usual case...
      float height = getHeightAt(point.x, point.z);
      if (point.y <= height)
      {
        point.y = height;
        return make_pair(true, point);
      }

      // move further...
      point += dir;

      // check if we are still inside the boundaries
      if (point.x < box.getMinimum().x || point.z < box.getMinimum().z
        || point.x > box.getMaximum().x || point.z > box.getMaximum().z)
        return make_pair(false, Ogre::Vector3::ZERO);

    }
  }

}
