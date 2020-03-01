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

#include "Impl/ETIndexHandler.h"

#include <OgreHardwareBufferManager.h>

using namespace Ogre;
using Ogre::uint;
using Ogre::ushort;

namespace ET
{
  namespace Impl
  {
    IndexHandler::IndexHandler(size_t tileSize, unsigned int maxLOD)
    : mTileSize(tileSize), mIndexes(maxLOD)
    {
    }

    IndexHandler::~IndexHandler()
    {
      for (size_t i = 0; i < mIndexes.size(); ++i)
      {
        for (IndexMap::iterator it = mIndexes[i].begin(); it != mIndexes[i].end(); ++it)
          delete it->second;
      }
    }

    IndexData* IndexHandler::requestIndexData(unsigned int lod, unsigned int neighbourState)
    {
      assert(lod < mIndexes.size() && "Requested unexpected LOD level");
      IndexData* data;
      IndexMap::iterator it = mIndexes[lod].find(neighbourState);
      if (it != mIndexes[lod].end())
        data = it->second;
      else
      {
        data = createIndexes(lod, neighbourState);
        mIndexes[lod].insert(IndexMap::value_type(neighbourState, data));
      }
      return data;
    }


    unsigned short IndexHandler::index(size_t x, size_t z) const
    {
      return ushort(x + z * mTileSize);
    }


    IndexData* IndexHandler::createIndexes(unsigned int lod, unsigned int neighbourState)
    {
      unsigned int numIndexes = 0;
      unsigned int step = 1 << lod;

      unsigned int northLOD = neighbourState >> 24;
      unsigned int eastLOD = (neighbourState >> 16) & 0xFF;
      unsigned int southLOD = (neighbourState >> 8) & 0xFF;
      unsigned int westLOD = neighbourState & 0xFF;
      unsigned int north = northLOD ? step : 0;
      unsigned int east = eastLOD ? step : 0;
      unsigned int south = southLOD ? step : 0;
      unsigned int west = westLOD ? step : 0;

      size_t newLength = (mTileSize/step) * (mTileSize/step) * 2 * 2 * 2;
      IndexData* indexData = new IndexData;
      indexData->indexBuffer = HardwareBufferManager::getSingleton().createIndexBuffer(
        HardwareIndexBuffer::IT_16BIT, newLength, HardwareBuffer::HBU_STATIC_WRITE_ONLY);
      unsigned short* pIdx = static_cast<unsigned short*>(indexData->indexBuffer->lock(
        0, indexData->indexBuffer->getSizeInBytes(), HardwareBuffer::HBL_DISCARD));

      // go over all vertices and combine them to triangles in trilist format.
      // leave out the edges if we need to stitch those in case over lower LOD at the
      // neighbour.
      for (unsigned int j = north; j < mTileSize-1 - south; j += step)
      {
        for (unsigned int i = west; i < mTileSize-1 - east; i += step)
        {
          // triangles
          *pIdx++ = index(i, j);
          *pIdx++ = index(i, j+step);
          *pIdx++ = index(i+step, j);

          *pIdx++ = index(i, j+step);
          *pIdx++ = index(i+step, j+step);
          *pIdx++ = index(i+step, j);

          numIndexes += 6;
        }
      }

      // stitching edges to neighbours where needed
      if (northLOD)
        numIndexes += stitchEdge(NORTH, lod, northLOD, westLOD > 0, eastLOD > 0, &pIdx);
      if (eastLOD)
        numIndexes += stitchEdge(EAST, lod, eastLOD, northLOD > 0, southLOD > 0, &pIdx);
      if (southLOD)
        numIndexes += stitchEdge(SOUTH, lod, southLOD, eastLOD > 0, westLOD > 0, &pIdx);
      if (westLOD)
        numIndexes += stitchEdge(WEST, lod, westLOD, southLOD > 0, northLOD > 0, &pIdx);

      indexData->indexBuffer->unlock();
      indexData->indexCount = numIndexes;
      indexData->indexStart = 0;

      return indexData;
    }


    int IndexHandler::stitchEdge(int direction, unsigned int hiLOD, unsigned int loLOD, bool omitFirstTri,
        bool omitLastTri, unsigned short** ppIdx)
    {
      assert(loLOD > hiLOD);

      // code taken from Ogre's TSM

      unsigned short* pIdx = *ppIdx;

      int step = 1 << hiLOD;
      int superstep = 1 << loLOD;
      int halfsuperstep = superstep >> 1;
      int rowstep = 0;
      size_t startx = 0, starty = 0, endx = 0;
      bool horizontal = false;
      switch (direction)
      {
      case NORTH:
        startx = starty = 0;
        endx = mTileSize - 1;
        rowstep = step;
        horizontal = true;
        break;

      case SOUTH:
        startx = starty = mTileSize-1;
        endx = 0;
        rowstep = -step;
        step = -step;
        superstep = -superstep;
        halfsuperstep = -halfsuperstep;
        horizontal = true;
        break;

      case EAST:
        startx = 0;
        endx = mTileSize-1;
        starty = mTileSize-1;
        rowstep = -step;
        horizontal = false;
        break;

      case WEST:
        startx = mTileSize-1;
        endx = 0;
        starty = 0;
        rowstep = step;
        step = -step;
        superstep = -superstep;
        halfsuperstep = -halfsuperstep;
        horizontal = false;
        break;
      }

      unsigned int numIndexes = 0;

      for (size_t j = startx; j != endx; j += superstep)
      {
        int k;
        for (k = 0; k != halfsuperstep; k += step)
        {
          size_t jk = j + k;
          if (j != startx || k != 0 || !omitFirstTri)
          {
            if (horizontal)
            {
              *pIdx++ = index(j, starty);
              *pIdx++ = index(jk, starty + rowstep);
              *pIdx++ = index(jk + step, starty + rowstep);
            }
            else
            {
              *pIdx++ = index(starty, j);
              *pIdx++ = index(starty+rowstep, jk);
              *pIdx++ = index(starty+rowstep, jk+step);
            }
            numIndexes += 3;
          }
        }

        if (horizontal)
        {
          *pIdx++ = index(j, starty);
          *pIdx++ = index(j+halfsuperstep, starty+rowstep);
          *pIdx++ = index(j+superstep, starty);
        }
        else
        {
          *pIdx++ = index(starty, j);
          *pIdx++ = index(starty+rowstep, j+halfsuperstep);
          *pIdx++ = index(starty, j+superstep);
        }
        numIndexes += 3;

        for (k = halfsuperstep; k != superstep; k += step)
        {
          size_t jk = j + k;
          if (j != endx - superstep || k != superstep - step || !omitLastTri)
          {
            if (horizontal)
            {
              *pIdx++ = index(j+superstep, starty);
              *pIdx++ = index(jk, starty+rowstep);
              *pIdx++ = index(jk+step, starty+rowstep);
            }
            else
            {
              *pIdx++ = index(starty, j+superstep);
              *pIdx++ = index(starty+rowstep, jk);
              *pIdx++ = index(starty+rowstep, jk+step);
            }
            numIndexes += 3;
          }
        }
      }

      *ppIdx = pIdx;

      return numIndexes;
    }
  }
}
