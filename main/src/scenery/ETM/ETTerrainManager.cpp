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

#include "ETTerrainManager.h"
#include "ETTerrainInfo.h"
#include "Impl/ETTerrainImpl.h"
#include "Impl/ETTile.h"

#include <OgreStringConverter.h>

using namespace Ogre;
using namespace std;
using Ogre::uint;
using Ogre::ushort;

namespace ET
{
  namespace Impl
  {
    uint checkTileSizeAndGetMaxLOD(size_t tileSize)
    {
      // check whether the tilesize suffices 2^n+1
      for (unsigned int i = 1; i <= 10; ++i)
      {
        if (tileSize == (uint)(1 << i) + 1)
        {
          return i+1;
        }
      }
      OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Given tilesize does not satisfy 2^n+1", "ET::Impl::checkTileSizeAndMaxLOD");
    }

    uint checkInfo(const TerrainInfo& info, size_t tileSize)
    {
      uint ret = checkTileSizeAndGetMaxLOD(tileSize);
      // check width and height
      if (info.getWidth() == 1 || info.getWidth() % (tileSize-1) != 1)
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Heightmap width must be a multiple of (tileSize-1) + 1", "ET::Impl::checkInfo");
      if (info.getHeight() == 1 || info.getHeight() % (tileSize-1) != 1)
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "Heightmap height must be a multiple of (tileSize-1) + 1", "ET::Impl::checkInfo");
      return ret;
    }




    TerrainImpl::TerrainImpl(SceneManager* sceneMgr, const string& name)
    : mSceneMgr(sceneMgr), mTerrainLoaded(false), mIndexHandler(0), mInstanceName(name),
      mViewportHeight(0), maxLODAtBorders(false)
    {
      mOpt.maxPixelError = 3;
      mOpt.useLODMorph = false;
      mOpt.factorC = 0.0f;
    }

    TerrainImpl::~TerrainImpl()
    {
      destroyTerrain();
    }


    void TerrainImpl::createTerrain(const TerrainInfo& info, size_t tileSize, uint maxLOD, bool vertexNormals, bool vertexTangents)
    {
      // ensure we have a viewport height set
      if (mOpt.factorC <= 0)
        OGRE_EXCEPT(Exception::ERR_INVALIDPARAMS, "You need to set a valid pixel error margin.", "TerrainImpl::createTerrain");

      // check given info for validity and determine the maximal possible LOD
      uint maxPossibleLOD = checkInfo(info, tileSize);

      // delete currently loaded terrain (if any)
      destroyTerrain();

      // load new terrain
      mInfo = info;
      mOpt.tileSize = tileSize;
      if (maxLOD < 1)
        maxLOD = 1;
      mOpt.maxMipMapLevel = min(maxLOD, maxPossibleLOD);
      mOpt.vertexNormals = vertexNormals;
      mOpt.vertexTangents = vertexTangents;
      mIndexHandler = new IndexHandler(mOpt.tileSize, mOpt.maxMipMapLevel);
      createTiles();

      mTerrainLoaded = true;
    }

    void TerrainImpl::createTiles()
    {
      // create a root node for the terrain tiles
      mTerrainNode = 0;
      string nodeName = mInstanceName + "/Terrain";
      if (mSceneMgr->hasSceneNode(nodeName))
        mTerrainNode = mSceneMgr->getSceneNode(nodeName);
      else
        mTerrainNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(nodeName);

      // create the tile grid
      size_t numTilesCol = (mInfo.getWidth()-1) / (mOpt.tileSize-1);
      size_t numTilesRow = (mInfo.getHeight()-1) / (mOpt.tileSize-1);
      mTiles.assign(numTilesCol, TileCol(numTilesRow, (Tile*)0));

      // for each tile, create a scene node to attach to and create the tile
      for (size_t i = 0; i < numTilesCol; ++i)
      {
        for (size_t j = 0; j < numTilesRow; ++j)
        {
          string name = nodeName+"/Tile[" + StringConverter::toString(i) + "][" + StringConverter::toString(j) + "]";
          SceneNode* node = 0;
          if (mSceneMgr->hasSceneNode(name))
          {
            node = mSceneMgr->getSceneNode(name);
            if (node->getParentSceneNode() != mTerrainNode)
              mTerrainNode->addChild(node);
          }
          else
            node = mTerrainNode->createChildSceneNode(name);

          mTiles[i][j] = new Tile(name, mSceneMgr, this, mIndexHandler, mInfo, mOpt, i*(mOpt.tileSize-1), j*(mOpt.tileSize-1));
          node->attachObject(mTiles[i][j]);
        }
      }
    }

    void TerrainImpl::destroyTerrain()
    {
      for (size_t i = 0; i < mTiles.size(); ++i)
        for (size_t j = 0; j < mTiles[i].size(); ++j)
          delete mTiles[i][j];
      mTiles.clear();
      delete mIndexHandler;
      mIndexHandler = 0;
      mInfo.setHeightmap(0, 0, 0);
      mTerrainLoaded = false;
    }


    void TerrainImpl::setLODErrorMargin(unsigned int maxPixelError, unsigned int viewportHeight)
    {
      mOpt.maxPixelError = maxPixelError;

      // calculate the C factor used in LOD distance calculations
      float T = 2 * maxPixelError / float(viewportHeight);
      mOpt.factorC = 1 / T;
    }

    void TerrainImpl::setUseLODMorphing(bool lodMorph, float startMorphing, const std::string& morphParamName)
    {
      mOpt.useLODMorph = lodMorph;
      mOpt.lodMorphStart = startMorphing;
      mOpt.lodMorphParam = morphParamName;
    }



    unsigned int TerrainImpl::getNeighbourState(size_t x, size_t z) const
    {
      unsigned int north = 0, east = 0, south = 0, west = 0;

      // get LOD of the tile
      unsigned int lod = mTiles[x][z]->getLOD();

      // check for any neighbours and save their LOD if it's greater than ours
      if (z > 0 && mTiles[x][z-1]->getLOD() > lod)
        north = mTiles[x][z-1]->getLOD();
      if (x < mTiles.size()-1 && mTiles[x+1][z]->getLOD() > lod)
        east = mTiles[x+1][z]->getLOD();
      if (z < mTiles[x].size()-1 && mTiles[x][z+1]->getLOD() > lod)
        south = mTiles[x][z+1]->getLOD();
      if (x > 0 && mTiles[x-1][z]->getLOD() > lod)
        west = mTiles[x-1][z]->getLOD();

      // shift-unite the neighbours' LODs into an int
      return (north << 24) | (east << 16) | (south << 8) | west;
    }


    void TerrainImpl::deform(int x, int z, const Brush& brush, float intensity)
    {
      // positions given are supposed to be the mid of the brush
      // so adjust accordingly
      x -= (int)brush.getWidth()/2;
      z -= (int)brush.getHeight()/2;

      // iterate over all fields of the brush array and apply them to the
      // heightmap data if they lie within
      for (size_t i = 0; i < brush.getWidth(); ++i)
      {
        int posX = x + (int)i;
        if (posX < 0 || posX >= (int)mInfo.getWidth())
          continue;
        for (size_t j = 0; j < brush.getHeight(); ++j)
        {
          int posZ = z + (int)j;
          if (posZ < 0 || posZ >= (int)mInfo.getHeight())
            continue;

          float& height = mInfo.at(size_t(posX), size_t(posZ));
          height += intensity * brush.at(i, j);
          if (height > 1)
            height = 1;
          if (height < 0)
            height = 0;
        }
      }

      updateTiles(x, z, x+(int)brush.getWidth(), z+(int)brush.getHeight());
    }

    void TerrainImpl::setHeights(int x, int z, const Brush& brush)
    {
      // positions given are supposed to be the mid of the brush
      // so adjust accordingly
      x -= (int)brush.getWidth()/2;
      z -= (int)brush.getHeight()/2;

      // iterate over all fields of the brush array and apply them to
      // the heightmap data if they lie within
      for (size_t i = 0; i < brush.getWidth(); ++i)
      {
        int posX = x + (int)i;
        if (posX < 0 || posX >= (int)mInfo.getWidth())
          continue;
        for (size_t j = 0; j < brush.getHeight(); ++j)
        {
          int posZ = z + (int)j;
          if (posZ < 0 || posZ >= (int)mInfo.getHeight())
            continue;

          float& height = mInfo.at(size_t(posX), size_t(posZ));
          height = brush.at(i, j);
          if (height > 1)
            height = 1;
          if (height < 0)
            height = 0;
        }
      }

      updateTiles(x, z, x+(int)brush.getWidth(), z+(int)brush.getHeight());
    }

    void TerrainImpl::getHeights(int x, int z, Brush& brush) const
    {
      // positions given are supposed to be the mid of the brush
      // so adjust accordingly
      x -= (int)brush.getWidth()/2;
      z -= (int)brush.getHeight()/2;

      // iterate over all fields of the brush array and fill them
      // if they lie within
      for (size_t i = 0; i < brush.getWidth(); ++i)
      {
        int posX = x + (int)i;
        for (size_t j = 0; j < brush.getHeight(); ++j)
        {
          int posZ = z + (int)j;
          if (posX < 0 || posX >= (int)mInfo.getWidth() || posZ < 0 || posZ >= (int)mInfo.getHeight())
          {
            brush.at(i, j) = -1;
            continue;
          }

          brush.at(i, j) = mInfo.at(size_t(posX), size_t(posZ));
        }
      }
    }


    void TerrainImpl::updateTiles(int fromX, int fromZ, int toX, int toZ)
    {
      // iterate over all terrain tiles and update them if they lie within
      // the update areal.
      for (size_t i = 0; i < mTiles.size(); ++i)
      {
        int startX = (int) ((mOpt.tileSize-1)*i);
        int endX = (int) ((mOpt.tileSize-1)*(i+1));
        for (size_t j = 0; j < mTiles[i].size(); ++j)
        {
          int startZ = (int) ((mOpt.tileSize-1)*j);
          int endZ = (int) ((mOpt.tileSize-1)*(j+1));
          size_t x1 = (size_t) max(fromX, startX);
          size_t x2 = (size_t) min(toX, endX);
          size_t z1 = (size_t) max(fromZ, startZ);
          size_t z2 = (size_t) min(toZ, endZ);
          if (x2 >= x1 && z2 >= z1)
          {
            // update this tile
            mTiles[i][j]->updateTerrain(x1, z1, x2, z2);
          }
        }
      }

      // update world bounds of the scene nodes holding the tiles (is this necessary?)
      for (unsigned short i = 0; i < mTerrainNode->numChildren(); ++i)
      {
        SceneNode* node = static_cast<SceneNode*>(mTerrainNode->getChild(i));
        node->_updateBounds();
      }
    }
  }


  TerrainManager::TerrainManager(SceneManager* sceneMgr, const string& name)
  : mImpl(new Impl::TerrainImpl(sceneMgr, name))
  {
  }

  TerrainManager::~TerrainManager()
  {
    delete mImpl;
  }

  void TerrainManager::createTerrain(const TerrainInfo& info, size_t tileSize, uint maxLOD, bool vertexNormals, bool vertexTangents)
  {
    mImpl->createTerrain(info, tileSize, maxLOD, vertexNormals, vertexTangents);
  }

  void TerrainManager::destroyTerrain()
  {
    mImpl->destroyTerrain();
  }

  const TerrainInfo& TerrainManager::getTerrainInfo() const
  {
    if (!mImpl->isTerrainLoaded())
      OGRE_EXCEPT(Exception::ERR_INVALID_STATE, "No terrain loaded.", "TerrainManager::getTerrainInfo");
    return mImpl->getTerrainInfo();
  }

  void TerrainManager::setLODErrorMargin(uint maxPixelError, uint viewportHeight)
  {
    mImpl->setLODErrorMargin(maxPixelError, viewportHeight);
  }


  void TerrainManager::setUseLODMorphing(bool lodMorph, float startMorphing, const string& morphParamName)
  {
    mImpl->setUseLODMorphing(lodMorph, startMorphing, morphParamName);
  }


  void TerrainManager::setMaterial(MaterialPtr material)
  {
    mImpl->setMaterial(material);
  }

  const MaterialPtr& TerrainManager::getMaterial() const
  {
    return mImpl->getMaterial();
  }


  void TerrainManager::deform(int x, int z, const Brush& brush, float intensity)
  {
    if (!mImpl->isTerrainLoaded())
      OGRE_EXCEPT(Exception::ERR_INVALID_STATE, "No terrain loaded.", "TerrainManager::deform");
    mImpl->deform(x, z, brush, intensity);
  }

  void TerrainManager::setHeights(int x, int z, const Brush& brush)
  {
    if (!mImpl->isTerrainLoaded())
      OGRE_EXCEPT(Exception::ERR_INVALID_STATE, "No terrain loaded.", "TerrainManager::setHeights");
    mImpl->setHeights(x, z, brush);
  }

  void TerrainManager::getHeights(int x, int z, Brush& brush) const
  {
    if (!mImpl->isTerrainLoaded())
      OGRE_EXCEPT(Exception::ERR_INVALID_STATE, "No terrain loaded.", "TerrainManager::getHeights");
    mImpl->getHeights(x, z, brush);
  }
}
