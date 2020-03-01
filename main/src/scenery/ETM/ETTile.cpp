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

#include "Impl/ETTerrainImpl.h"
#include "Impl/ETTile.h"
#include "Impl/ETIndexHandler.h"
#include "ETTerrainInfo.h"

#include <OgreColourValue.h>
#include <OgreHardwareBufferManager.h>

using namespace Ogre;

namespace ET
{
  namespace Impl
  {
    const unsigned short MAIN_BINDING = 0;
    const unsigned short DELTA_BINDING = 1;
    const unsigned int MORPH_CUSTOM_PARAM_ID = 77;

    Tile::Tile(const String& name, SceneManager* sm, TerrainImpl* tm,
      IndexHandler* indexHandler, TerrainInfo& info, Options opts, size_t startx, size_t startz)
    : MovableObject(name), mSceneMgr(sm), mTerrainMgr(tm),
      mIndexHandler(indexHandler), mInfo(info), mOpt(opts),
      mStartX(startx), mStartZ(startz),
      mLOD(0),
      mLODChangeMinDistSqr(opts.maxMipMapLevel),
      mLightListDirty(true),
      mTerrain(0),
      mLastNextLevel(0)
    {
      if (mOpt.maxMipMapLevel < 1)
        mOpt.maxMipMapLevel = 1;
      // disable LOD morphing if max LOD is 1
      if (mOpt.maxMipMapLevel == 1)
        mOpt.useLODMorph = false;

      mCastShadows = false;

      mTileX = startx / (mOpt.tileSize-1);
      mTileZ = startz / (mOpt.tileSize-1);

      createVertexData(startx, startz);
      calculateMinLevelDist2();
    }

    Tile::~Tile()
    {
      delete mTerrain;
    }

    const String& Tile::getMovableType() const
    {
      static const String type = "EditableTerrainTile";
      return type;
    }

    const MaterialPtr& Tile::getMaterial() const
    {
      return mTerrainMgr->getMaterial();
    }

    const AxisAlignedBox& Tile::getBoundingBox() const
    {
      return mBounds;
    }

    Real Tile::getBoundingRadius() const
    {
      return mBoundingRadius;
    }

    void Tile::getWorldTransforms(Matrix4* m) const
    {
      *m = mParentNode->_getFullTransform();
    }

    const Quaternion& Tile::getWorldOrientation() const
    {
      return mParentNode->_getDerivedOrientation();
    }

    const Vector3& Tile::getWorldPosition() const
    {
      return mParentNode->_getDerivedPosition();
    }


    Real Tile::getSquaredViewDepth(const Camera* cam) const
    {
      Vector3 diff = mCenter - cam->getDerivedPosition();
      return diff.squaredLength();
    }


    const LightList& Tile::getLights() const
    {
      if (mLightListDirty)
      {
        mSceneMgr->_populateLightList(mCenter, getBoundingRadius(), mLightList);
        mLightListDirty = false;
      }
      return mLightList;
    }


    uint32 Tile::getTypeFlags() const
    {
      return SceneManager::WORLD_GEOMETRY_TYPE_MASK;
    }


    void Tile::emptyBuffer(HardwareVertexBufferSharedPtr buf)
    {
      void* pVoid = buf->lock(HardwareBuffer::HBL_DISCARD);
      memset(pVoid, 0, mOpt.tileSize*mOpt.tileSize*sizeof(float));
      buf->unlock();
    }



    void Tile::createVertexData(size_t startx, size_t startz)
    {
      mTerrain = new VertexData;
      mTerrain->vertexStart = 0;
      mTerrain->vertexCount = mOpt.tileSize * mOpt.tileSize;

      VertexDeclaration* decl = mTerrain->vertexDeclaration;
      VertexBufferBinding* bind = mTerrain->vertexBufferBinding;

      // first we need to declare the contents of our vertex buffer
      size_t offset = 0;
      decl->addElement(MAIN_BINDING, offset, VET_FLOAT3, VES_POSITION);
      offset += VertexElement::getTypeSize(VET_FLOAT3);
      if (mOpt.vertexNormals)
      {
        decl->addElement(MAIN_BINDING, offset, VET_FLOAT3, VES_NORMAL);
        offset += VertexElement::getTypeSize(VET_FLOAT3);
      }
      if (mOpt.vertexTangents)
      {
        decl->addElement(MAIN_BINDING, offset, VET_FLOAT3, VES_TANGENT);
        offset += VertexElement::getTypeSize(VET_FLOAT3);
      }
      decl->addElement(MAIN_BINDING, offset, VET_FLOAT2, VES_TEXTURE_COORDINATES, 0);
      offset += VertexElement::getTypeSize(VET_FLOAT2);

      mMainBuffer = HardwareBufferManager::getSingleton().createVertexBuffer(
        decl->getVertexSize(MAIN_BINDING), mTerrain->vertexCount, HardwareBuffer::HBU_STATIC_WRITE_ONLY);

      // bind the buffer
      bind->setBinding(MAIN_BINDING, mMainBuffer);

      // declare delta buffers, if requested
      if (mOpt.useLODMorph)
      {
        decl->addElement(DELTA_BINDING, 0, VET_FLOAT1, VES_BLEND_WEIGHTS);
      }


      // now construct the vertex buffer from the heightmap data
      size_t endx = startx + mOpt.tileSize;
      size_t endz = startz + mOpt.tileSize;
      Real minHeight = mInfo.getOffset().y + mInfo.getScaling().y, maxHeight = mInfo.getOffset().y;

      const VertexElement* posElem = decl->findElementBySemantic(VES_POSITION);
      const VertexElement* texElem0 = decl->findElementBySemantic(VES_TEXTURE_COORDINATES, 0);

      unsigned char* pBase = static_cast<unsigned char*>(mMainBuffer->lock(HardwareBuffer::HBL_DISCARD));

      for (size_t j = startz; j < endz; ++j)
      {
        for (size_t i = startx; i < endx; ++i)
        {
          float* pPos, * pTex0;//, * pTex1;
          posElem->baseVertexPointerToElement(pBase, &pPos);
          texElem0->baseVertexPointerToElement(pBase, &pTex0);

          Real height = mInfo.getOffset().y + mInfo.at(i, j) * mInfo.getScaling().y;
          *pPos++ = mInfo.getOffset().x + mInfo.getScaling().x * i;
          *pPos++ = height;
          *pPos++ = mInfo.getOffset().z + mInfo.getScaling().z * j;

          *pTex0++ = float(i) / (mInfo.getWidth() - 1);
          *pTex0++ = float(j) / (mInfo.getHeight() - 1);

          if (height < minHeight)
            minHeight = height;
          if (height > maxHeight)
            maxHeight = height;

          pBase += mMainBuffer->getVertexSize();
        }
      }
      mMainBuffer->unlock();

      // set the extents of this terrain tile
      mBounds.setExtents(mInfo.getOffset().x + mInfo.getScaling().x * startx, minHeight,
        mInfo.getOffset().z + mInfo.getScaling().z * startz,
        mInfo.getOffset().x + mInfo.getScaling().x * endx, maxHeight,
        mInfo.getOffset().z + mInfo.getScaling().z * (endz));

      mCenter = mBounds.getCenter();

      mBoundingRadius = (mBounds.getMaximum() - mCenter).length();

      // if using LOD morphing, create the delta buffers now (they'll be filled in _calculateMinLevelDist2)
      if (mOpt.useLODMorph)
      {
        for (unsigned int i = 0; i < mOpt.maxMipMapLevel-1; ++i)
        {
          HardwareVertexBufferSharedPtr buf = HardwareBufferManager::getSingleton().createVertexBuffer(
            VertexElement::getTypeSize(VET_FLOAT1), mOpt.tileSize*mOpt.tileSize,
            HardwareBuffer::HBU_STATIC_WRITE_ONLY);
          emptyBuffer(buf);
          mDeltaBuffers.push_back(buf);
        }
      }


      // calc vertex normals, if necessary
      if (mOpt.vertexNormals)
        calculateVertexNormals();
      // calc vertex tangents, if necessary
      if (mOpt.vertexTangents)
        calculateVertexTangents();
    }



    void Tile::calculateVertexNormals()
    {
      // set the vertex normals of the tile

      size_t startx = mStartX;
      size_t startz = mStartZ;
      size_t endx = startx + mOpt.tileSize;
      size_t endz = startz + mOpt.tileSize;
      Real minHeight = mInfo.getOffset().y + mInfo.getScaling().y, maxHeight = mInfo.getOffset().y;

      const VertexElement* normElem = mTerrain->vertexDeclaration->findElementBySemantic(VES_NORMAL);
      unsigned char* pBase = static_cast<unsigned char*>(mMainBuffer->lock(HardwareBuffer::HBL_NORMAL));

      for (size_t j = startz; j < endz; ++j)
      {
        for (size_t i = startx; i < endx; ++i)
        {
          float* pNorm;
          normElem->baseVertexPointerToElement(pBase, &pNorm);

          Vector3 normal = mInfo.getNormalAt(mInfo.vertexToPosX((int)i), mInfo.vertexToPosZ((int)j));
          *pNorm++ = normal.x;
          *pNorm++ = normal.y;
          *pNorm++ = normal.z;

          pBase += mMainBuffer->getVertexSize();
        }
      }
      mMainBuffer->unlock();
    }


    /** Addition by SongOfTheWeave */
    void Tile::calculateVertexTangents()
    {
      // set the vertex tangents of the tile
      size_t startx = mStartX;
      size_t startz = mStartZ;
      size_t endx = startx + mOpt.tileSize;
      size_t endz = startz + mOpt.tileSize;
      //Real minHeight = mInfo.getOffset().y + mInfo.getScaling().y, maxHeight = mInfo.getOffset().y;

      const VertexElement* normElem = mTerrain->vertexDeclaration->findElementBySemantic(VES_TANGENT);
      unsigned char* pBase = static_cast<unsigned char*>(mMainBuffer->lock(HardwareBuffer::HBL_NORMAL));

      for (size_t j = startz; j < endz; ++j)
      {
        for (size_t i = startx; i < endx; ++i)
        {
          float* pTan;
          normElem->baseVertexPointerToElement(pBase, &pTan);

          Vector3 tangent = mInfo.getTangentAt(mInfo.vertexToPosX((int)i), mInfo.vertexToPosZ((int)j));
          *pTan++ = tangent.x;
          *pTan++ = tangent.y;
          *pTan++ = tangent.z;

          pBase += mMainBuffer->getVertexSize();
        }
      }
      mMainBuffer->unlock();
    }


    IndexData* Tile::getIndexData()
    {
      IndexData* data = mIndexHandler->requestIndexData(mLOD,
        mTerrainMgr->getNeighbourState(mTileX, mTileZ));
      return data;
    }


    void Tile::getRenderOperation(RenderOperation& op)
    {
      op.useIndexes = true;
      op.operationType = RenderOperation::OT_TRIANGLE_LIST;
      op.vertexData = mTerrain;
      op.indexData = getIndexData();
    }

    void Tile::_updateRenderQueue(RenderQueue* queue)
    {
      mLightListDirty = true;
      queue->addRenderable(this, mRenderQueueID);
    }


    void Tile::_updateCustomGpuParameter(const GpuProgramParameters::AutoConstantEntry& constEntry,
      GpuProgramParameters* params) const
    {
      if (constEntry.data == MORPH_CUSTOM_PARAM_ID)
      {
        params->_writeRawConstant(constEntry.physicalIndex, mLODMorphFactor);
      }
      else
      {
        Renderable::_updateCustomGpuParameter(constEntry, params);
      }
    }


    Vector3 Tile::getVector(size_t x, size_t z) const
    {
      float posX = mInfo.getOffset().x + (mStartX + x)*mInfo.getScaling().x;
      float posZ = mInfo.getOffset().z + (mStartZ + z)*mInfo.getScaling().z;
      return Vector3(posX, mInfo.getHeightAt(posX, posZ), posZ);
    }

    void Tile::calculateMinLevelDist2()
    {
      Real C = mOpt.factorC;

      // LOD 0 has no distance
      mLODChangeMinDistSqr[0] = 0.0f;


      for (unsigned int level = 1; level < mOpt.maxMipMapLevel; ++level)
      {
        mLODChangeMinDistSqr[level] = 0.0f;

        size_t step = 1 << level;
        size_t higherstep = step >> 1;

        float* pDeltas = 0;
        // for LOD morphing, lock the according delta buffer now
        if (mOpt.useLODMorph)
        {
          // indexed at LOD-1, because there are only maxLOD-1 transitions between LODs...
          emptyBuffer(mDeltaBuffers[level-1]);
          pDeltas = static_cast<float*>(mDeltaBuffers[level-1]->lock(HardwareBuffer::HBL_NORMAL));
        }

        // for every vertex that is not used in the current LOD we calculate its interpolated
        // height and compare against its actual height. the largest difference of a vertex
        // is then used to determine the minimal distance for this LOD.
        for (size_t j = 0; j < mOpt.tileSize - step; j += step)
        {
          for (size_t i = 0; i < mOpt.tileSize - step; i += step)
          {
            Vector3 v1 = getVector(i, j);
            Vector3 v2 = getVector(i+step, j);
            Vector3 v3 = getVector(i, j+step);
            Vector3 v4 = getVector(i+step, j+step);
            Plane t1 (v1, v3, v2);
            Plane t2 (v2, v3, v4);

            size_t zubound = (j == (mOpt.tileSize - step) ? step : step - 1);
            for (size_t z = 0; z <= zubound; ++z)
            {
              size_t xubound = (i == (mOpt.tileSize - step) ? step : step - 1);
              for (size_t x = 0; x < xubound; ++x)
              {
                size_t fulldetailx = i + x;
                size_t fulldetailz = j + z;
                if (fulldetailx % step == 0 && fulldetailz % step == 0)
                {
                  continue;
                }

                Real zpct = Real(z) / Real(step);
                Real xpct = Real(x) / Real(step);

                Vector3 actualPos = getVector(fulldetailx, fulldetailz);
                Real interp_h;
                if (xpct + zpct <= 1.0f)
                {
                  interp_h = (
                    -(t1.normal.x * actualPos.x)
                    - t1.normal.z * actualPos.z
                    - t1.d) / t1.normal.y;
                }
                else
                {
                  interp_h = (
                    -(t2.normal.x * actualPos.x)
                    - t2.normal.z * actualPos.z
                    - t2.d) / t2.normal.y;
                }

                Real actual_h = getVector(fulldetailx, fulldetailz).y;
                Real delta = fabs(interp_h - actual_h);

                Real D2 = delta * delta * C * C;

                if (mLODChangeMinDistSqr[level] < D2)
                  mLODChangeMinDistSqr[level] = D2;

                // for LOD morphing, store the difference in the delta buffer
                if (mOpt.useLODMorph &&
                  fulldetailx != 0 && fulldetailx != (mOpt.tileSize-1) &&
                  fulldetailz != 0 && fulldetailz != (mOpt.tileSize-1))
                {
                  pDeltas[fulldetailx + (fulldetailz * mOpt.tileSize)] = interp_h - actual_h;
                }
              }
            }
          }
        }

        // unlock delta buffers
        if (mOpt.useLODMorph)
          mDeltaBuffers[level-1]->unlock();
      }


      // post validate
      for (unsigned int i = 1; i < mOpt.maxMipMapLevel; ++i)
      {
        // ensure level distances are increasing
        if (mLODChangeMinDistSqr[i] < mLODChangeMinDistSqr[i-1])
          mLODChangeMinDistSqr[i] = mLODChangeMinDistSqr[i-1];

      }
    }


    void Tile::_notifyCurrentCamera(Camera* cam)
    {
      MovableObject::_notifyCurrentCamera(cam);

      Vector3 cpos = cam->getDerivedPosition();
      const AxisAlignedBox& aabb = getWorldBoundingBox(true);
      Vector3 diff(0, 0, 0);
      diff.makeFloor(cpos - aabb.getMinimum());
      diff.makeCeil(cpos - aabb.getMaximum());

      // find the LOD to use for this tile
      Real L = diff.squaredLength();
      mLOD = mOpt.maxMipMapLevel-1;
      for (unsigned int i = 1; i < mOpt.maxMipMapLevel; ++i)
      {
        if (mLODChangeMinDistSqr[i] > L)
        {
          mLOD = i - 1;
          break;
        }
      }

      if (mOpt.useLODMorph)
      {
        // find the next LOD after the current one
        unsigned int nextLevel = mLOD + 1;
        for (unsigned int i = nextLevel; i < mOpt.maxMipMapLevel; ++i)
        {
          if (mLODChangeMinDistSqr[i] > mLODChangeMinDistSqr[mLOD])
          {
            nextLevel = i;
            break;
          }
        }

        // determine the LOD morph factor between the two LODs
        if (nextLevel == mOpt.maxMipMapLevel)
        {
          mLODMorphFactor = 0;
        }
        else
        {
          Real range = mLODChangeMinDistSqr[nextLevel] - mLODChangeMinDistSqr[mLOD];
          Real percent = (L - mLODChangeMinDistSqr[mLOD]) / range;
          Real rescale = 1.0f / (1.0f - mOpt.lodMorphStart);
          mLODMorphFactor = std::max((percent - mOpt.lodMorphStart) * rescale, Real(0));
        }

        if (mLastNextLevel != nextLevel)
        {
          if (nextLevel != mOpt.maxMipMapLevel)
          {
            mTerrain->vertexBufferBinding->setBinding(DELTA_BINDING, mDeltaBuffers[nextLevel-1]);
          }
          else
          {
            // bind dummy
            mTerrain->vertexBufferBinding->setBinding(DELTA_BINDING, mDeltaBuffers[0]);
          }
        }
        mLastNextLevel = nextLevel;
      }
    }


    unsigned int Tile::getLOD() const
    {
      return mLOD;
    }


    void Tile::updateTerrain(size_t startx, size_t startz, size_t endx, size_t endz)
    {
      // determine the area of this tile that needs to be updated
      size_t fromX = std::max(mStartX, startx);
      size_t fromZ = std::max(mStartZ, startz);
      size_t toX = std::min(endx, mStartX+mOpt.tileSize-1);
      size_t toZ = std::min(endz, mStartZ+mOpt.tileSize-1);

      const VertexElement* posElem = mTerrain->vertexDeclaration->findElementBySemantic(VES_POSITION);
      unsigned char* pBase = static_cast<unsigned char*>(mMainBuffer->lock(HardwareBuffer::HBL_NORMAL));

      // update all necessary vertices
      for (size_t j = fromZ; j <= toZ; ++j)
      {
        for (size_t i = fromX; i <= toX; ++i)
        {
          size_t tX = i - mStartX;
          size_t tZ = j - mStartZ;
          unsigned char* pBasePos = pBase + (tZ*mOpt.tileSize + tX) * mMainBuffer->getVertexSize();

          Real height = mInfo.getOffset().y + mInfo.at(i, j) * mInfo.getScaling().y;
          float* pPos;
          posElem->baseVertexPointerToElement(pBasePos, &pPos);
          pPos[1] = height;
        }
      }

      mMainBuffer->unlock();

      // update the extents of this terrain tile
      size_t ex = mStartX + mOpt.tileSize;
      size_t ez = mStartZ + mOpt.tileSize;
      // find min and max heights
      Real minHeight = mInfo.getOffset().y + mInfo.getScaling().y, maxHeight = mInfo.getOffset().y;
      for (size_t j = mStartZ; j < ez; ++j)
      {
        for (size_t i = mStartX; i < ex; ++i)
        {
          Real height = mInfo.getOffset().y + mInfo.at(i, j)*mInfo.getScaling().y;
          if (height < minHeight)
            minHeight = height;
          if (height > maxHeight)
            maxHeight = height;
        }
      }
      mBounds.setMinimumY(minHeight);
      mBounds.setMaximumY(maxHeight);

      mCenter = mBounds.getCenter();

      mBoundingRadius = (mBounds.getMaximum() - mCenter).length();

      // recalculate the distances at which to switch LOD
      calculateMinLevelDist2();

      // recalculate vertex normals, if necessary
      if (mOpt.vertexNormals)
        calculateVertexNormals();
    }


#if OGRE_VERSION_MINOR > 4
    /** Shoggoth compatibility function */
    void Tile::visitRenderables(Renderable::Visitor* visitor, bool debugRenderables)
    {
      visitor->visit(this, 0, false, 0);
    }
#endif


  }
}
