//
//  RenderablePolyVoxEntityItem.cpp
//  libraries/entities-renderer/src/
//
//  Created by Seth Alves on 5/19/15.
//  Copyright 2015 High Fidelity, Inc.
//  Copyright 2022-2023 Overte e.V.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//  SPDX-License-Identifier: Apache-2.0
//

#include "RenderablePolyVoxEntityItem.h"

#include <math.h>
#include <numeric>
#include <cstdint>
#include <optional>

#include <glm/gtx/transform.hpp>

#include <QObject>
#include <QByteArray>
#include <QtConcurrent/QtConcurrentRun>

#include <model-networking/SimpleMeshProxy.h>
#include "ModelScriptingInterface.h"
#include <EntityEditPacketSender.h>
#include <PhysicalEntitySimulation.h>
#include <StencilMaskPass.h>
#include <graphics/ShaderConstants.h>
#include <render/ShapePipeline.h>
#include <procedural/Procedural.h>

#include "entities-renderer/ShaderConstants.h"

#include <shaders/Shaders.h>

#include "EntityTreeRenderer.h"
#include "RenderPipelines.h"
#include "gpu/Forward.h"

#include <FadeEffect.h>

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdouble-promotion"
#endif

#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

#include <Model.h>
#include <PerfStat.h>
#include <render/Scene.h>

#include "graphics/Geometry.h"

#include "StencilMaskPass.h"

#include "EntityTreeRenderer.h"

#include "RenderablePolyVoxEntityItem.h"
#include "PhysicalEntitySimulation.h"

const float MARCHING_CUBE_COLLISION_HULL_OFFSET = 0.5;

/*
  A PolyVoxEntity has several interdependent parts:

  _voxelData -- compressed QByteArray representation of which voxels have which values
  _volData -- datastructure from the PolyVox library which holds which voxels have which values
  _mesh -- renderable representation of the voxels
  _shape -- used for bullet (physics) collisions

  Each one depends on the one before it, except that _voxelData is set from _volData if a script edits the voxels.

  There are booleans to indicate that something has been updated and the dependents now need to be updated.
  _meshReady       -- do we have something to give scripts that ask for the mesh?
  _voxelDataDirty  -- do we need to uncompress data and expand it into _volData?
  _volDataDirty    -- does recomputeMesh need to be called?
  _shapeReady      -- are we ready to tell bullet our shape?


  Here is a simplified diagram of the state machine implemented in RenderablePolyVoxEntityItem::update

  +-------------------+
  |                   |
  |                   |
  |  volDataDirty     |     voxelDataDirty
  |                +--v--+
  |         +------+Ready+--------+
  |         |      +-----+        |
  |         |                     |
  |   +-----v----+         +------v------+
  |   |BakingMesh|         |Uncompressing|
  |   +-----+----+         +------+------+
  |         |                     |
  |         |                     |
  |     +---v-------+     +-------v------------+
  |     |Compressing|     |BakingMeshNoCompress|
  |     +---------+-+     ++-------------------+
  |               |        |
  |               |        |
  |             +-v--------v+
  |             |BakingShape|
  |             +-----+-----+
  |                   |
  +-------------------+


  The work for each step is done on temporary worker threads.  The PolyVox entity will update _updateNeeded and
  enable or disable update calls on the entity, depending on if there is more work to do.

  From the 'Ready' state, if we receive an update from the network, _voxelDataDirty will be set true.  We
  uncompress the received data, bake the mesh (for the render-engine's benefit), and then compute the shape
  (for the physics-engine's benefit).  This is the right-hand side of the diagram.

  From the 'Ready' state, if a script changes a voxel, _volDataDirty will be set true.  We bake the mesh,
  compress the voxels into a new _voxelData, and transmit the new _voxelData to the entity-server.  We then
  bake the shape.  This is the left-hand side of the diagram.

  The actual state machine is more complicated than the diagram, because it's possible for _volDataDirty or
  _voxelDataDirty to be set true while worker threads are attempting to bake meshes or shapes.  If this happens,
  we jump back to a higher point in the diagram to avoid wasting effort.

  PolyVoxes are designed to seemlessly fit up against neighbors.  If voxels go right up to the edge of polyvox,
  the resulting mesh wont be closed -- the library assumes you'll have another polyvox next to it to continue the
  mesh.

  If a polyvox entity is "edged", the voxel space is wrapped in an extra layer of zero-valued voxels.  This avoids the
  previously mentioned gaps along the edges.

  Non-edged polyvox entities can be told about their neighbors in all 6 cardinal directions.  On the positive
  edges of the polyvox, the values are set from the (negative edge of the) relevant neighbor so that their meshes
  knit together.  This is handled by tellNeighborsToRecopyEdges and copyUpperEdgesFromNeighbors.  In these functions, variable
  names have XP for x-positive, XN x-negative, etc.

 */

// FIXME: move to GLM helpers
template <typename T, typename F>
void loop3(const T& start, const T& end, F f) {
    T current;
    for (current.z = start.z; current.z < end.z; ++current.z) {
        for (current.y = start.y; current.y < end.y; ++current.y) {
            for (current.x = start.x; current.x < end.x; ++current.x) {
                f(current);
            }
        }
    }
}

template <typename T, typename F>
void loop2(const T& start, const T& end, F f) {
    T current;
    for (current.y = start.y; current.y < end.y; ++current.y) {
        for (current.x = start.x; current.x < end.x; ++current.x) {
            f(current);
        }
    }
}
namespace {

typedef uint8_t VoxelType;
typedef VoxelType MaterialType;
}  // namespace

class VoxelVolume {
public:
    inline VoxelVolume(const glm::ivec3& size) :
        allocated_size(size), valid_size(size), raw_data(size.x * size.y * size.z, 0) {}

    bool isInside(const glm::ivec3& index) {
        return glm::all(glm::lessThanEqual(glm::ivec3(0), index)) && glm::all(glm::lessThan(index, valid_size));
    }
    inline bool isInside(const int32_t x, const int32_t y, const int32_t z) { return isInside(glm::ivec3(x, y, z)); }

    void setVoxelAt(const glm::ivec3& index, VoxelType data) {
        Q_ASSERT(isInside(index));
        ensureAllocation(index);
        raw_data[getRawIndexFor(index)] = data;
    }
    inline void setVoxelAt(int32_t x, int32_t y, int32_t z, uint8_t data) { return setVoxelAt(glm::ivec3(x, y, z), data); }

    VoxelType getVoxelAt(const glm::ivec3& index) {
        Q_ASSERT(isInside(index));
        if (glm::any(glm::greaterThanEqual(index, allocated_size)))
            return 0;
        return raw_data[getRawIndexFor(index)];
    }

    void resize(const glm::ivec3& new_size) { valid_size = new_size; }

    const glm::ivec3& getSize() { return valid_size; }

    std::optional<glm::ivec3> raycast(glm::vec4 originInVoxel, glm::vec4 farInVoxel) {
        float x1 = originInVoxel.x + 0.5f;
        float y1 = originInVoxel.y + 0.5f;
        float z1 = originInVoxel.z + 0.5f;
        float x2 = farInVoxel.x + 0.5f;
        float y2 = farInVoxel.y + 0.5f;
        float z2 = farInVoxel.z + 0.5f;

        int i = (int)floorf(x1);
        int j = (int)floorf(y1);
        int k = (int)floorf(z1);

        int iend = (int)floorf(x2);
        int jend = (int)floorf(y2);
        int kend = (int)floorf(z2);

        int di = ((x1 < x2) ? 1 : ((x1 > x2) ? -1 : 0));
        int dj = ((y1 < y2) ? 1 : ((y1 > y2) ? -1 : 0));
        int dk = ((z1 < z2) ? 1 : ((z1 > z2) ? -1 : 0));

        float deltatx = 1.0f / std::abs(x2 - x1);
        float deltaty = 1.0f / std::abs(y2 - y1);
        float deltatz = 1.0f / std::abs(z2 - z1);

        float minx = floorf(x1), maxx = minx + 1.0f;
        float tx = ((x1 > x2) ? (x1 - minx) : (maxx - x1)) * deltatx;
        float miny = floorf(y1), maxy = miny + 1.0f;
        float ty = ((y1 > y2) ? (y1 - miny) : (maxy - y1)) * deltaty;
        float minz = floorf(z1), maxz = minz + 1.0f;
        float tz = ((z1 > z2) ? (z1 - minz) : (maxz - z1)) * deltatz;

        glm::ivec3 index{ i, j, k };

        for (;;) {
            if (isInside(index) && getVoxelAt(index) != 0) {
                return index;
            }

            if (tx <= ty && tx <= tz) {
                if (i == iend)
                    break;
                tx += deltatx;
                i += di;

                if (di == 1)
                    index.x++;
                if (di == -1)
                    index.x--;
            } else if (ty <= tz) {
                if (j == jend)
                    break;
                ty += deltaty;
                j += dj;

                if (dj == 1)
                    index.y++;
                if (dj == -1)
                    index.y--;
            } else {
                if (k == kend)
                    break;
                tz += deltatz;
                k += dk;

                if (dk == 1)
                    index.z++;
                if (dk == -1)
                    index.z--;
            }
        }

        return std::nullopt;
    }

private:
    inline size_t getRawIndexFor(const glm::ivec3& index) {
        return allocated_size.x * allocated_size.y * index.z + allocated_size.x * index.y + index.x;
    }

    void ensureAllocation(const glm::ivec3& index) {
        if (glm::any(glm::greaterThanEqual(index, allocated_size))) {
            std::vector<VoxelType> new_(valid_size.x * valid_size.y * valid_size.z, 0);

            loop3(glm::ivec3{ 0 }, allocated_size, [&](const glm::ivec3& index) {
                new_[valid_size.x * valid_size.y * index.z + valid_size.x * index.y + index.x] =
                    raw_data[getRawIndexFor(index)];
            });

            raw_data = std::move(new_);
            allocated_size = valid_size;
        }
    }

private:
    glm::ivec3 allocated_size;
    glm::ivec3 valid_size;
    std::vector<VoxelType> raw_data;
};
namespace {

int edgeTable[256] = { 0x0,   0x109, 0x203, 0x30a, 0x406, 0x50f, 0x605, 0x70c, 0x80c, 0x905, 0xa0f, 0xb06, 0xc0a, 0xd03, 0xe09,
                       0xf00, 0x190, 0x99,  0x393, 0x29a, 0x596, 0x49f, 0x795, 0x69c, 0x99c, 0x895, 0xb9f, 0xa96, 0xd9a, 0xc93,
                       0xf99, 0xe90, 0x230, 0x339, 0x33,  0x13a, 0x636, 0x73f, 0x435, 0x53c, 0xa3c, 0xb35, 0x83f, 0x936, 0xe3a,
                       0xf33, 0xc39, 0xd30, 0x3a0, 0x2a9, 0x1a3, 0xaa,  0x7a6, 0x6af, 0x5a5, 0x4ac, 0xbac, 0xaa5, 0x9af, 0x8a6,
                       0xfaa, 0xea3, 0xda9, 0xca0, 0x460, 0x569, 0x663, 0x76a, 0x66,  0x16f, 0x265, 0x36c, 0xc6c, 0xd65, 0xe6f,
                       0xf66, 0x86a, 0x963, 0xa69, 0xb60, 0x5f0, 0x4f9, 0x7f3, 0x6fa, 0x1f6, 0xff,  0x3f5, 0x2fc, 0xdfc, 0xcf5,
                       0xfff, 0xef6, 0x9fa, 0x8f3, 0xbf9, 0xaf0, 0x650, 0x759, 0x453, 0x55a, 0x256, 0x35f, 0x55,  0x15c, 0xe5c,
                       0xf55, 0xc5f, 0xd56, 0xa5a, 0xb53, 0x859, 0x950, 0x7c0, 0x6c9, 0x5c3, 0x4ca, 0x3c6, 0x2cf, 0x1c5, 0xcc,
                       0xfcc, 0xec5, 0xdcf, 0xcc6, 0xbca, 0xac3, 0x9c9, 0x8c0, 0x8c0, 0x9c9, 0xac3, 0xbca, 0xcc6, 0xdcf, 0xec5,
                       0xfcc, 0xcc,  0x1c5, 0x2cf, 0x3c6, 0x4ca, 0x5c3, 0x6c9, 0x7c0, 0x950, 0x859, 0xb53, 0xa5a, 0xd56, 0xc5f,
                       0xf55, 0xe5c, 0x15c, 0x55,  0x35f, 0x256, 0x55a, 0x453, 0x759, 0x650, 0xaf0, 0xbf9, 0x8f3, 0x9fa, 0xef6,
                       0xfff, 0xcf5, 0xdfc, 0x2fc, 0x3f5, 0xff,  0x1f6, 0x6fa, 0x7f3, 0x4f9, 0x5f0, 0xb60, 0xa69, 0x963, 0x86a,
                       0xf66, 0xe6f, 0xd65, 0xc6c, 0x36c, 0x265, 0x16f, 0x66,  0x76a, 0x663, 0x569, 0x460, 0xca0, 0xda9, 0xea3,
                       0xfaa, 0x8a6, 0x9af, 0xaa5, 0xbac, 0x4ac, 0x5a5, 0x6af, 0x7a6, 0xaa,  0x1a3, 0x2a9, 0x3a0, 0xd30, 0xc39,
                       0xf33, 0xe3a, 0x936, 0x83f, 0xb35, 0xa3c, 0x53c, 0x435, 0x73f, 0x636, 0x13a, 0x33,  0x339, 0x230, 0xe90,
                       0xf99, 0xc93, 0xd9a, 0xa96, 0xb9f, 0x895, 0x99c, 0x69c, 0x795, 0x49f, 0x596, 0x29a, 0x393, 0x99,  0x190,
                       0xf00, 0xe09, 0xd03, 0xc0a, 0xb06, 0xa0f, 0x905, 0x80c, 0x70c, 0x605, 0x50f, 0x406, 0x30a, 0x203, 0x109,
                       0x0 };

int triTable[256][16] = { { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 0, 1, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 1, 8, 3, 9, 8, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 0, 8, 3, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 9, 2, 10, 0, 2, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 2, 8, 3, 2, 10, 8, 10, 9, 8, -1, -1, -1, -1, -1, -1, -1 },
                          { 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 0, 11, 2, 8, 11, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 1, 9, 0, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 1, 11, 2, 1, 9, 11, 9, 8, 11, -1, -1, -1, -1, -1, -1, -1 },
                          { 3, 10, 1, 11, 10, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 0, 10, 1, 0, 8, 10, 8, 11, 10, -1, -1, -1, -1, -1, -1, -1 },
                          { 3, 9, 0, 3, 11, 9, 11, 10, 9, -1, -1, -1, -1, -1, -1, -1 },
                          { 9, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 4, 3, 0, 7, 3, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 0, 1, 9, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 4, 1, 9, 4, 7, 1, 7, 3, 1, -1, -1, -1, -1, -1, -1, -1 },
                          { 1, 2, 10, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 3, 4, 7, 3, 0, 4, 1, 2, 10, -1, -1, -1, -1, -1, -1, -1 },
                          { 9, 2, 10, 9, 0, 2, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1 },
                          { 2, 10, 9, 2, 9, 7, 2, 7, 3, 7, 9, 4, -1, -1, -1, -1 },
                          { 8, 4, 7, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 11, 4, 7, 11, 2, 4, 2, 0, 4, -1, -1, -1, -1, -1, -1, -1 },
                          { 9, 0, 1, 8, 4, 7, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1 },
                          { 4, 7, 11, 9, 4, 11, 9, 11, 2, 9, 2, 1, -1, -1, -1, -1 },
                          { 3, 10, 1, 3, 11, 10, 7, 8, 4, -1, -1, -1, -1, -1, -1, -1 },
                          { 1, 11, 10, 1, 4, 11, 1, 0, 4, 7, 11, 4, -1, -1, -1, -1 },
                          { 4, 7, 8, 9, 0, 11, 9, 11, 10, 11, 0, 3, -1, -1, -1, -1 },
                          { 4, 7, 11, 4, 11, 9, 9, 11, 10, -1, -1, -1, -1, -1, -1, -1 },
                          { 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 9, 5, 4, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 0, 5, 4, 1, 5, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 8, 5, 4, 8, 3, 5, 3, 1, 5, -1, -1, -1, -1, -1, -1, -1 },
                          { 1, 2, 10, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 3, 0, 8, 1, 2, 10, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1 },
                          { 5, 2, 10, 5, 4, 2, 4, 0, 2, -1, -1, -1, -1, -1, -1, -1 },
                          { 2, 10, 5, 3, 2, 5, 3, 5, 4, 3, 4, 8, -1, -1, -1, -1 },
                          { 9, 5, 4, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 0, 11, 2, 0, 8, 11, 4, 9, 5, -1, -1, -1, -1, -1, -1, -1 },
                          { 0, 5, 4, 0, 1, 5, 2, 3, 11, -1, -1, -1, -1, -1, -1, -1 },
                          { 2, 1, 5, 2, 5, 8, 2, 8, 11, 4, 8, 5, -1, -1, -1, -1 },
                          { 10, 3, 11, 10, 1, 3, 9, 5, 4, -1, -1, -1, -1, -1, -1, -1 },
                          { 4, 9, 5, 0, 8, 1, 8, 10, 1, 8, 11, 10, -1, -1, -1, -1 },
                          { 5, 4, 0, 5, 0, 11, 5, 11, 10, 11, 0, 3, -1, -1, -1, -1 },
                          { 5, 4, 8, 5, 8, 10, 10, 8, 11, -1, -1, -1, -1, -1, -1, -1 },
                          { 9, 7, 8, 5, 7, 9, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 9, 3, 0, 9, 5, 3, 5, 7, 3, -1, -1, -1, -1, -1, -1, -1 },
                          { 0, 7, 8, 0, 1, 7, 1, 5, 7, -1, -1, -1, -1, -1, -1, -1 },
                          { 1, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 9, 7, 8, 9, 5, 7, 10, 1, 2, -1, -1, -1, -1, -1, -1, -1 },
                          { 10, 1, 2, 9, 5, 0, 5, 3, 0, 5, 7, 3, -1, -1, -1, -1 },
                          { 8, 0, 2, 8, 2, 5, 8, 5, 7, 10, 5, 2, -1, -1, -1, -1 },
                          { 2, 10, 5, 2, 5, 3, 3, 5, 7, -1, -1, -1, -1, -1, -1, -1 },
                          { 7, 9, 5, 7, 8, 9, 3, 11, 2, -1, -1, -1, -1, -1, -1, -1 },
                          { 9, 5, 7, 9, 7, 2, 9, 2, 0, 2, 7, 11, -1, -1, -1, -1 },
                          { 2, 3, 11, 0, 1, 8, 1, 7, 8, 1, 5, 7, -1, -1, -1, -1 },
                          { 11, 2, 1, 11, 1, 7, 7, 1, 5, -1, -1, -1, -1, -1, -1, -1 },
                          { 9, 5, 8, 8, 5, 7, 10, 1, 3, 10, 3, 11, -1, -1, -1, -1 },
                          { 5, 7, 0, 5, 0, 9, 7, 11, 0, 1, 0, 10, 11, 10, 0, -1 },
                          { 11, 10, 0, 11, 0, 3, 10, 5, 0, 8, 0, 7, 5, 7, 0, -1 },
                          { 11, 10, 5, 7, 11, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 0, 8, 3, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 9, 0, 1, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 1, 8, 3, 1, 9, 8, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1 },
                          { 1, 6, 5, 2, 6, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 1, 6, 5, 1, 2, 6, 3, 0, 8, -1, -1, -1, -1, -1, -1, -1 },
                          { 9, 6, 5, 9, 0, 6, 0, 2, 6, -1, -1, -1, -1, -1, -1, -1 },
                          { 5, 9, 8, 5, 8, 2, 5, 2, 6, 3, 2, 8, -1, -1, -1, -1 },
                          { 2, 3, 11, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 11, 0, 8, 11, 2, 0, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1 },
                          { 0, 1, 9, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1, -1, -1, -1 },
                          { 5, 10, 6, 1, 9, 2, 9, 11, 2, 9, 8, 11, -1, -1, -1, -1 },
                          { 6, 3, 11, 6, 5, 3, 5, 1, 3, -1, -1, -1, -1, -1, -1, -1 },
                          { 0, 8, 11, 0, 11, 5, 0, 5, 1, 5, 11, 6, -1, -1, -1, -1 },
                          { 3, 11, 6, 0, 3, 6, 0, 6, 5, 0, 5, 9, -1, -1, -1, -1 },
                          { 6, 5, 9, 6, 9, 11, 11, 9, 8, -1, -1, -1, -1, -1, -1, -1 },
                          { 5, 10, 6, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 4, 3, 0, 4, 7, 3, 6, 5, 10, -1, -1, -1, -1, -1, -1, -1 },
                          { 1, 9, 0, 5, 10, 6, 8, 4, 7, -1, -1, -1, -1, -1, -1, -1 },
                          { 10, 6, 5, 1, 9, 7, 1, 7, 3, 7, 9, 4, -1, -1, -1, -1 },
                          { 6, 1, 2, 6, 5, 1, 4, 7, 8, -1, -1, -1, -1, -1, -1, -1 },
                          { 1, 2, 5, 5, 2, 6, 3, 0, 4, 3, 4, 7, -1, -1, -1, -1 },
                          { 8, 4, 7, 9, 0, 5, 0, 6, 5, 0, 2, 6, -1, -1, -1, -1 },
                          { 7, 3, 9, 7, 9, 4, 3, 2, 9, 5, 9, 6, 2, 6, 9, -1 },
                          { 3, 11, 2, 7, 8, 4, 10, 6, 5, -1, -1, -1, -1, -1, -1, -1 },
                          { 5, 10, 6, 4, 7, 2, 4, 2, 0, 2, 7, 11, -1, -1, -1, -1 },
                          { 0, 1, 9, 4, 7, 8, 2, 3, 11, 5, 10, 6, -1, -1, -1, -1 },
                          { 9, 2, 1, 9, 11, 2, 9, 4, 11, 7, 11, 4, 5, 10, 6, -1 },
                          { 8, 4, 7, 3, 11, 5, 3, 5, 1, 5, 11, 6, -1, -1, -1, -1 },
                          { 5, 1, 11, 5, 11, 6, 1, 0, 11, 7, 11, 4, 0, 4, 11, -1 },
                          { 0, 5, 9, 0, 6, 5, 0, 3, 6, 11, 6, 3, 8, 4, 7, -1 },
                          { 6, 5, 9, 6, 9, 11, 4, 7, 9, 7, 11, 9, -1, -1, -1, -1 },
                          { 10, 4, 9, 6, 4, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 4, 10, 6, 4, 9, 10, 0, 8, 3, -1, -1, -1, -1, -1, -1, -1 },
                          { 10, 0, 1, 10, 6, 0, 6, 4, 0, -1, -1, -1, -1, -1, -1, -1 },
                          { 8, 3, 1, 8, 1, 6, 8, 6, 4, 6, 1, 10, -1, -1, -1, -1 },
                          { 1, 4, 9, 1, 2, 4, 2, 6, 4, -1, -1, -1, -1, -1, -1, -1 },
                          { 3, 0, 8, 1, 2, 9, 2, 4, 9, 2, 6, 4, -1, -1, -1, -1 },
                          { 0, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 8, 3, 2, 8, 2, 4, 4, 2, 6, -1, -1, -1, -1, -1, -1, -1 },
                          { 10, 4, 9, 10, 6, 4, 11, 2, 3, -1, -1, -1, -1, -1, -1, -1 },
                          { 0, 8, 2, 2, 8, 11, 4, 9, 10, 4, 10, 6, -1, -1, -1, -1 },
                          { 3, 11, 2, 0, 1, 6, 0, 6, 4, 6, 1, 10, -1, -1, -1, -1 },
                          { 6, 4, 1, 6, 1, 10, 4, 8, 1, 2, 1, 11, 8, 11, 1, -1 },
                          { 9, 6, 4, 9, 3, 6, 9, 1, 3, 11, 6, 3, -1, -1, -1, -1 },
                          { 8, 11, 1, 8, 1, 0, 11, 6, 1, 9, 1, 4, 6, 4, 1, -1 },
                          { 3, 11, 6, 3, 6, 0, 0, 6, 4, -1, -1, -1, -1, -1, -1, -1 },
                          { 6, 4, 8, 11, 6, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 7, 10, 6, 7, 8, 10, 8, 9, 10, -1, -1, -1, -1, -1, -1, -1 },
                          { 0, 7, 3, 0, 10, 7, 0, 9, 10, 6, 7, 10, -1, -1, -1, -1 },
                          { 10, 6, 7, 1, 10, 7, 1, 7, 8, 1, 8, 0, -1, -1, -1, -1 },
                          { 10, 6, 7, 10, 7, 1, 1, 7, 3, -1, -1, -1, -1, -1, -1, -1 },
                          { 1, 2, 6, 1, 6, 8, 1, 8, 9, 8, 6, 7, -1, -1, -1, -1 },
                          { 2, 6, 9, 2, 9, 1, 6, 7, 9, 0, 9, 3, 7, 3, 9, -1 },
                          { 7, 8, 0, 7, 0, 6, 6, 0, 2, -1, -1, -1, -1, -1, -1, -1 },
                          { 7, 3, 2, 6, 7, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 2, 3, 11, 10, 6, 8, 10, 8, 9, 8, 6, 7, -1, -1, -1, -1 },
                          { 2, 0, 7, 2, 7, 11, 0, 9, 7, 6, 7, 10, 9, 10, 7, -1 },
                          { 1, 8, 0, 1, 7, 8, 1, 10, 7, 6, 7, 10, 2, 3, 11, -1 },
                          { 11, 2, 1, 11, 1, 7, 10, 6, 1, 6, 7, 1, -1, -1, -1, -1 },
                          { 8, 9, 6, 8, 6, 7, 9, 1, 6, 11, 6, 3, 1, 3, 6, -1 },
                          { 0, 9, 1, 11, 6, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 7, 8, 0, 7, 0, 6, 3, 11, 0, 11, 6, 0, -1, -1, -1, -1 },
                          { 7, 11, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 3, 0, 8, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 0, 1, 9, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 8, 1, 9, 8, 3, 1, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1 },
                          { 10, 1, 2, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 1, 2, 10, 3, 0, 8, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1 },
                          { 2, 9, 0, 2, 10, 9, 6, 11, 7, -1, -1, -1, -1, -1, -1, -1 },
                          { 6, 11, 7, 2, 10, 3, 10, 8, 3, 10, 9, 8, -1, -1, -1, -1 },
                          { 7, 2, 3, 6, 2, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 7, 0, 8, 7, 6, 0, 6, 2, 0, -1, -1, -1, -1, -1, -1, -1 },
                          { 2, 7, 6, 2, 3, 7, 0, 1, 9, -1, -1, -1, -1, -1, -1, -1 },
                          { 1, 6, 2, 1, 8, 6, 1, 9, 8, 8, 7, 6, -1, -1, -1, -1 },
                          { 10, 7, 6, 10, 1, 7, 1, 3, 7, -1, -1, -1, -1, -1, -1, -1 },
                          { 10, 7, 6, 1, 7, 10, 1, 8, 7, 1, 0, 8, -1, -1, -1, -1 },
                          { 0, 3, 7, 0, 7, 10, 0, 10, 9, 6, 10, 7, -1, -1, -1, -1 },
                          { 7, 6, 10, 7, 10, 8, 8, 10, 9, -1, -1, -1, -1, -1, -1, -1 },
                          { 6, 8, 4, 11, 8, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 3, 6, 11, 3, 0, 6, 0, 4, 6, -1, -1, -1, -1, -1, -1, -1 },
                          { 8, 6, 11, 8, 4, 6, 9, 0, 1, -1, -1, -1, -1, -1, -1, -1 },
                          { 9, 4, 6, 9, 6, 3, 9, 3, 1, 11, 3, 6, -1, -1, -1, -1 },
                          { 6, 8, 4, 6, 11, 8, 2, 10, 1, -1, -1, -1, -1, -1, -1, -1 },
                          { 1, 2, 10, 3, 0, 11, 0, 6, 11, 0, 4, 6, -1, -1, -1, -1 },
                          { 4, 11, 8, 4, 6, 11, 0, 2, 9, 2, 10, 9, -1, -1, -1, -1 },
                          { 10, 9, 3, 10, 3, 2, 9, 4, 3, 11, 3, 6, 4, 6, 3, -1 },
                          { 8, 2, 3, 8, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1 },
                          { 0, 4, 2, 4, 6, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 1, 9, 0, 2, 3, 4, 2, 4, 6, 4, 3, 8, -1, -1, -1, -1 },
                          { 1, 9, 4, 1, 4, 2, 2, 4, 6, -1, -1, -1, -1, -1, -1, -1 },
                          { 8, 1, 3, 8, 6, 1, 8, 4, 6, 6, 10, 1, -1, -1, -1, -1 },
                          { 10, 1, 0, 10, 0, 6, 6, 0, 4, -1, -1, -1, -1, -1, -1, -1 },
                          { 4, 6, 3, 4, 3, 8, 6, 10, 3, 0, 3, 9, 10, 9, 3, -1 },
                          { 10, 9, 4, 6, 10, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 4, 9, 5, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 0, 8, 3, 4, 9, 5, 11, 7, 6, -1, -1, -1, -1, -1, -1, -1 },
                          { 5, 0, 1, 5, 4, 0, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1 },
                          { 11, 7, 6, 8, 3, 4, 3, 5, 4, 3, 1, 5, -1, -1, -1, -1 },
                          { 9, 5, 4, 10, 1, 2, 7, 6, 11, -1, -1, -1, -1, -1, -1, -1 },
                          { 6, 11, 7, 1, 2, 10, 0, 8, 3, 4, 9, 5, -1, -1, -1, -1 },
                          { 7, 6, 11, 5, 4, 10, 4, 2, 10, 4, 0, 2, -1, -1, -1, -1 },
                          { 3, 4, 8, 3, 5, 4, 3, 2, 5, 10, 5, 2, 11, 7, 6, -1 },
                          { 7, 2, 3, 7, 6, 2, 5, 4, 9, -1, -1, -1, -1, -1, -1, -1 },
                          { 9, 5, 4, 0, 8, 6, 0, 6, 2, 6, 8, 7, -1, -1, -1, -1 },
                          { 3, 6, 2, 3, 7, 6, 1, 5, 0, 5, 4, 0, -1, -1, -1, -1 },
                          { 6, 2, 8, 6, 8, 7, 2, 1, 8, 4, 8, 5, 1, 5, 8, -1 },
                          { 9, 5, 4, 10, 1, 6, 1, 7, 6, 1, 3, 7, -1, -1, -1, -1 },
                          { 1, 6, 10, 1, 7, 6, 1, 0, 7, 8, 7, 0, 9, 5, 4, -1 },
                          { 4, 0, 10, 4, 10, 5, 0, 3, 10, 6, 10, 7, 3, 7, 10, -1 },
                          { 7, 6, 10, 7, 10, 8, 5, 4, 10, 4, 8, 10, -1, -1, -1, -1 },
                          { 6, 9, 5, 6, 11, 9, 11, 8, 9, -1, -1, -1, -1, -1, -1, -1 },
                          { 3, 6, 11, 0, 6, 3, 0, 5, 6, 0, 9, 5, -1, -1, -1, -1 },
                          { 0, 11, 8, 0, 5, 11, 0, 1, 5, 5, 6, 11, -1, -1, -1, -1 },
                          { 6, 11, 3, 6, 3, 5, 5, 3, 1, -1, -1, -1, -1, -1, -1, -1 },
                          { 1, 2, 10, 9, 5, 11, 9, 11, 8, 11, 5, 6, -1, -1, -1, -1 },
                          { 0, 11, 3, 0, 6, 11, 0, 9, 6, 5, 6, 9, 1, 2, 10, -1 },
                          { 11, 8, 5, 11, 5, 6, 8, 0, 5, 10, 5, 2, 0, 2, 5, -1 },
                          { 6, 11, 3, 6, 3, 5, 2, 10, 3, 10, 5, 3, -1, -1, -1, -1 },
                          { 5, 8, 9, 5, 2, 8, 5, 6, 2, 3, 8, 2, -1, -1, -1, -1 },
                          { 9, 5, 6, 9, 6, 0, 0, 6, 2, -1, -1, -1, -1, -1, -1, -1 },
                          { 1, 5, 8, 1, 8, 0, 5, 6, 8, 3, 8, 2, 6, 2, 8, -1 },
                          { 1, 5, 6, 2, 1, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 1, 3, 6, 1, 6, 10, 3, 8, 6, 5, 6, 9, 8, 9, 6, -1 },
                          { 10, 1, 0, 10, 0, 6, 9, 5, 0, 5, 6, 0, -1, -1, -1, -1 },
                          { 0, 3, 8, 5, 6, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 10, 5, 6, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 11, 5, 10, 7, 5, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 11, 5, 10, 11, 7, 5, 8, 3, 0, -1, -1, -1, -1, -1, -1, -1 },
                          { 5, 11, 7, 5, 10, 11, 1, 9, 0, -1, -1, -1, -1, -1, -1, -1 },
                          { 10, 7, 5, 10, 11, 7, 9, 8, 1, 8, 3, 1, -1, -1, -1, -1 },
                          { 11, 1, 2, 11, 7, 1, 7, 5, 1, -1, -1, -1, -1, -1, -1, -1 },
                          { 0, 8, 3, 1, 2, 7, 1, 7, 5, 7, 2, 11, -1, -1, -1, -1 },
                          { 9, 7, 5, 9, 2, 7, 9, 0, 2, 2, 11, 7, -1, -1, -1, -1 },
                          { 7, 5, 2, 7, 2, 11, 5, 9, 2, 3, 2, 8, 9, 8, 2, -1 },
                          { 2, 5, 10, 2, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1 },
                          { 8, 2, 0, 8, 5, 2, 8, 7, 5, 10, 2, 5, -1, -1, -1, -1 },
                          { 9, 0, 1, 5, 10, 3, 5, 3, 7, 3, 10, 2, -1, -1, -1, -1 },
                          { 9, 8, 2, 9, 2, 1, 8, 7, 2, 10, 2, 5, 7, 5, 2, -1 },
                          { 1, 3, 5, 3, 7, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 0, 8, 7, 0, 7, 1, 1, 7, 5, -1, -1, -1, -1, -1, -1, -1 },
                          { 9, 0, 3, 9, 3, 5, 5, 3, 7, -1, -1, -1, -1, -1, -1, -1 },
                          { 9, 8, 7, 5, 9, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 5, 8, 4, 5, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1 },
                          { 5, 0, 4, 5, 11, 0, 5, 10, 11, 11, 3, 0, -1, -1, -1, -1 },
                          { 0, 1, 9, 8, 4, 10, 8, 10, 11, 10, 4, 5, -1, -1, -1, -1 },
                          { 10, 11, 4, 10, 4, 5, 11, 3, 4, 9, 4, 1, 3, 1, 4, -1 },
                          { 2, 5, 1, 2, 8, 5, 2, 11, 8, 4, 5, 8, -1, -1, -1, -1 },
                          { 0, 4, 11, 0, 11, 3, 4, 5, 11, 2, 11, 1, 5, 1, 11, -1 },
                          { 0, 2, 5, 0, 5, 9, 2, 11, 5, 4, 5, 8, 11, 8, 5, -1 },
                          { 9, 4, 5, 2, 11, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 2, 5, 10, 3, 5, 2, 3, 4, 5, 3, 8, 4, -1, -1, -1, -1 },
                          { 5, 10, 2, 5, 2, 4, 4, 2, 0, -1, -1, -1, -1, -1, -1, -1 },
                          { 3, 10, 2, 3, 5, 10, 3, 8, 5, 4, 5, 8, 0, 1, 9, -1 },
                          { 5, 10, 2, 5, 2, 4, 1, 9, 2, 9, 4, 2, -1, -1, -1, -1 },
                          { 8, 4, 5, 8, 5, 3, 3, 5, 1, -1, -1, -1, -1, -1, -1, -1 },
                          { 0, 4, 5, 1, 0, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 8, 4, 5, 8, 5, 3, 9, 0, 5, 0, 3, 5, -1, -1, -1, -1 },
                          { 9, 4, 5, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 4, 11, 7, 4, 9, 11, 9, 10, 11, -1, -1, -1, -1, -1, -1, -1 },
                          { 0, 8, 3, 4, 9, 7, 9, 11, 7, 9, 10, 11, -1, -1, -1, -1 },
                          { 1, 10, 11, 1, 11, 4, 1, 4, 0, 7, 4, 11, -1, -1, -1, -1 },
                          { 3, 1, 4, 3, 4, 8, 1, 10, 4, 7, 4, 11, 10, 11, 4, -1 },
                          { 4, 11, 7, 9, 11, 4, 9, 2, 11, 9, 1, 2, -1, -1, -1, -1 },
                          { 9, 7, 4, 9, 11, 7, 9, 1, 11, 2, 11, 1, 0, 8, 3, -1 },
                          { 11, 7, 4, 11, 4, 2, 2, 4, 0, -1, -1, -1, -1, -1, -1, -1 },
                          { 11, 7, 4, 11, 4, 2, 8, 3, 4, 3, 2, 4, -1, -1, -1, -1 },
                          { 2, 9, 10, 2, 7, 9, 2, 3, 7, 7, 4, 9, -1, -1, -1, -1 },
                          { 9, 10, 7, 9, 7, 4, 10, 2, 7, 8, 7, 0, 2, 0, 7, -1 },
                          { 3, 7, 10, 3, 10, 2, 7, 4, 10, 1, 10, 0, 4, 0, 10, -1 },
                          { 1, 10, 2, 8, 7, 4, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 4, 9, 1, 4, 1, 7, 7, 1, 3, -1, -1, -1, -1, -1, -1, -1 },
                          { 4, 9, 1, 4, 1, 7, 0, 8, 1, 8, 7, 1, -1, -1, -1, -1 },
                          { 4, 0, 3, 7, 4, 3, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 4, 8, 7, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 9, 10, 8, 10, 11, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 3, 0, 9, 3, 9, 11, 11, 9, 10, -1, -1, -1, -1, -1, -1, -1 },
                          { 0, 1, 10, 0, 10, 8, 8, 10, 11, -1, -1, -1, -1, -1, -1, -1 },
                          { 3, 1, 10, 11, 3, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 1, 2, 11, 1, 11, 9, 9, 11, 8, -1, -1, -1, -1, -1, -1, -1 },
                          { 3, 0, 9, 3, 9, 11, 1, 2, 9, 2, 11, 9, -1, -1, -1, -1 },
                          { 0, 2, 11, 8, 0, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 3, 2, 11, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 2, 3, 8, 2, 8, 10, 10, 8, 9, -1, -1, -1, -1, -1, -1, -1 },
                          { 9, 10, 2, 0, 9, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 2, 3, 8, 2, 8, 10, 0, 1, 8, 1, 10, 8, -1, -1, -1, -1 },
                          { 1, 10, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 1, 3, 8, 9, 1, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 0, 9, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { 0, 3, 8, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
                          { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 } };

// TODO: explore if having seperate buffers is better:tm:, as this was just copied from polyvox
struct PositionNormalMaterial {
    glm::vec3 position;
    glm::vec3 normal;
    float material;
};

class SurfaceExtractor {
public:
    virtual graphics::MeshPointer createMesh() = 0;

protected:
    SurfaceExtractor(std::shared_ptr<VoxelVolume> vol) : vol(vol) {}

protected:
    std::shared_ptr<VoxelVolume> vol;
};

class CubicSurfaceExtractorWithNormals : public SurfaceExtractor {
public:
    inline CubicSurfaceExtractorWithNormals(std::shared_ptr<VoxelVolume> vol) : SurfaceExtractor(vol) {}

    graphics::MeshPointer createMesh() override {
        vecVertices.clear();

        loop3(ivec3{ 0 }, vol->getSize() - glm::ivec3(-1), [&](const ivec3& index) {
            float regX = static_cast<float>(index.x);
            float regY = static_cast<float>(index.y);
            float regZ = static_cast<float>(index.z);

            VoxelType this_el = vol->getVoxelAt(index);

            MaterialType material = 0;
            uint32_t v0, v1, v2, v3;

            glm::ivec3 other_index = index + glm::ivec3(1, 0, 0);
            auto other_el = vol->getVoxelAt(other_index);
            if (vol->isInside(other_index) && isQuadNeeded(other_el, this_el, material)) {
                const glm::vec3 posX = glm::vec3(1.0f, 0.0f, 0.0f);
                v0 = addVertex(glm::vec3(regX + 0.5f, regY - 0.5f, regZ - 0.5f), posX, material);
                v1 = addVertex(glm::vec3(regX + 0.5f, regY - 0.5f, regZ + 0.5f), posX, material);
                v2 = addVertex(glm::vec3(regX + 0.5f, regY + 0.5f, regZ - 0.5f), posX, material);
                v3 = addVertex(glm::vec3(regX + 0.5f, regY + 0.5f, regZ + 0.5f), posX, material);

                addTriangleCubic(v0, v2, v1);
                addTriangleCubic(v1, v2, v3);
            }
            if (isQuadNeeded(this_el, other_el, material)) {
                const glm::ivec3 negX = glm::vec3(-1.0f, 0.0f, 0.0f);
                v0 = addVertex(glm::vec3(regX + 0.5f, regY - 0.5f, regZ - 0.5f), negX, material);
                v1 = addVertex(glm::vec3(regX + 0.5f, regY - 0.5f, regZ + 0.5f), negX, material);
                v2 = addVertex(glm::vec3(regX + 0.5f, regY + 0.5f, regZ - 0.5f), negX, material);
                v3 = addVertex(glm::vec3(regX + 0.5f, regY + 0.5f, regZ + 0.5f), negX, material);

                addTriangleCubic(v0, v1, v2);
                addTriangleCubic(v1, v3, v2);
            }

            other_index = index + glm::ivec3(0, 1, 0);
            other_el = vol->getVoxelAt(other_index);
            if (vol->isInside(other_index) && isQuadNeeded(vol->getVoxelAt(other_index), this_el, material)) {
                const glm::ivec3 posY = glm::vec3(0.0f, 1.0f, 0.0f);
                v0 = addVertex(glm::vec3(regX - 0.5f, regY + 0.5f, regZ - 0.5f), posY, material);
                v1 = addVertex(glm::vec3(regX - 0.5f, regY + 0.5f, regZ + 0.5f), posY, material);
                v2 = addVertex(glm::vec3(regX + 0.5f, regY + 0.5f, regZ - 0.5f), posY, material);
                v3 = addVertex(glm::vec3(regX + 0.5f, regY + 0.5f, regZ + 0.5f), posY, material);

                addTriangleCubic(v0, v1, v2);
                addTriangleCubic(v1, v3, v2);
            }
            if (isQuadNeeded(this_el, vol->getVoxelAt(other_index), material)) {
                const glm::ivec3 negY = glm::vec3(0.0f, -1.0f, 0.0f);
                v0 = addVertex(glm::vec3(regX - 0.5f, regY + 0.5f, regZ - 0.5f), negY, material);
                v1 = addVertex(glm::vec3(regX - 0.5f, regY + 0.5f, regZ + 0.5f), negY, material);
                v2 = addVertex(glm::vec3(regX + 0.5f, regY + 0.5f, regZ - 0.5f), negY, material);
                v3 = addVertex(glm::vec3(regX + 0.5f, regY + 0.5f, regZ + 0.5f), negY, material);

                addTriangleCubic(v0, v2, v1);
                addTriangleCubic(v1, v2, v3);
            }

            other_index = index + glm::ivec3(0, 0, 1);
            other_el = vol->getVoxelAt(other_index);
            if (vol->isInside(other_index) && isQuadNeeded(other_el, this_el, material)) {
                const glm::ivec3 posZ = glm::vec3(0.0f, 0.0f, 1.0f);
                v0 = addVertex(glm::vec3(regX - 0.5f, regY - 0.5f, regZ + 0.5f), posZ, material);
                v1 = addVertex(glm::vec3(regX - 0.5f, regY + 0.5f, regZ + 0.5f), posZ, material);
                v2 = addVertex(glm::vec3(regX + 0.5f, regY - 0.5f, regZ + 0.5f), posZ, material);
                v3 = addVertex(glm::vec3(regX + 0.5f, regY + 0.5f, regZ + 0.5f), posZ, material);

                addTriangleCubic(v0, v2, v1);
                addTriangleCubic(v1, v2, v3);
            }
            if (isQuadNeeded(this_el, other_el, material)) {
                const glm::ivec3 negZ = glm::vec3(0.0f, 0.0f, -1.0f);
                v0 = addVertex(glm::vec3(regX - 0.5f, regY - 0.5f, regZ + 0.5f), negZ, material);
                v1 = addVertex(glm::vec3(regX - 0.5f, regY + 0.5f, regZ + 0.5f), negZ, material);
                v2 = addVertex(glm::vec3(regX + 0.5f, regY - 0.5f, regZ + 0.5f), negZ, material);
                v3 = addVertex(glm::vec3(regX + 0.5f, regY + 0.5f, regZ + 0.5f), negZ, material);

                addTriangleCubic(v0, v1, v2);
                addTriangleCubic(v1, v3, v2);
            }
        });

        graphics::MeshPointer mesh(std::make_shared<graphics::Mesh>());

        // convert PolyVox mesh to a Sam mesh
        gpu::BufferPointer indexBuffer(gpu::Buffer::createBuffer(gpu::Buffer::IndexBuffer, vecIndices));
        auto indexBufferPtr = gpu::BufferPointer(indexBuffer);
        gpu::BufferView indexBufferView(indexBufferPtr, gpu::Element(gpu::SCALAR, gpu::UINT32, gpu::INDEX));
        mesh->setIndexBuffer(indexBufferView);

        gpu::BufferPointer vertexBuffer(gpu::Buffer::createBuffer(gpu::Buffer::VertexBuffer, vecVertices));
        auto vertexBufferPtr = gpu::BufferPointer(vertexBuffer);

        mesh->setVertexBuffer(gpu::BufferView(vertexBufferPtr, offsetof(PositionNormalMaterial, position),
                                              vertexBufferPtr->getSize(), sizeof(PositionNormalMaterial),
                                              gpu::Element(gpu::VEC3, gpu::FLOAT, gpu::XYZ)));

        // TODO -- use 3-byte normals rather than 3-float normals
        mesh->addAttribute(gpu::Stream::NORMAL, gpu::BufferView(vertexBufferPtr, offsetof(PositionNormalMaterial, normal),
                                                                vertexBufferPtr->getSize(), sizeof(PositionNormalMaterial),
                                                                gpu::Element(gpu::VEC3, gpu::FLOAT, gpu::XYZ)));

        std::vector<graphics::Mesh::Part> parts;
        parts.emplace_back(graphics::Mesh::Part((graphics::Index)0,                  // startIndex
                                                (graphics::Index)vecIndices.size(),  // numIndices
                                                (graphics::Index)0,                  // baseVertex
                                                graphics::Mesh::TRIANGLES));         // topology
        mesh->setPartBuffer(
            gpu::BufferView(new gpu::Buffer(gpu::Buffer::IndirectBuffer, parts.size() * sizeof(graphics::Mesh::Part),
                                            (gpu::Byte*)parts.data()),
                            gpu::Element::PART_DRAWCALL));

        return mesh;
    }

private:
    bool isQuadNeeded(VoxelType front, VoxelType back, MaterialType& materialToUse) {
        if ((back > 0) && (front == 0)) {
            materialToUse = back;
            return true;
        } else {
            return false;
        }
    }

    uint32_t addVertex(glm::vec3&& pos, glm::vec3 norm, MaterialType mat) {
        vecVertices.emplace_back(pos, norm, static_cast<float>(mat));

        return vecVertices.size() - 1;
    }
    void addTriangleCubic(uint32_t v1, uint32_t v2, uint32_t v3) {
        vecIndices.push_back(v1);
        vecIndices.push_back(v2);
        vecIndices.push_back(v3);
    }

    std::vector<PositionNormalMaterial> vecVertices;
    std::vector<uint32_t> vecIndices;
};

class MarchingCubesSurfaceExtractor : public SurfaceExtractor {
public:
    inline MarchingCubesSurfaceExtractor(std::shared_ptr<VoxelVolume> vol) : SurfaceExtractor(vol) {}

    graphics::MeshPointer createMesh() override {
        vecIndices.clear();
        vecVertices.clear();

        auto valid_size = vol->getSize();
        loop3(glm::ivec3(0), valid_size - glm::ivec3(-1), [this, valid_size](glm::ivec3 index) {
            const auto i0 = index;
            const auto i1 = index + glm::ivec3(1, 0, 0);
            const auto i2 = index + glm::ivec3(1, 0, 1);
            const auto i3 = index + glm::ivec3(0, 0, 1);
            const auto i4 = index + glm::ivec3(0, 1, 0);
            const auto i5 = index + glm::ivec3(1, 1, 0);
            const auto i6 = index + glm::ivec3(1, 1, 1);
            const auto i7 = index + glm::ivec3(0, 1, 1);

            const auto v0 = vol->getVoxelAt(i0);
            const auto v1 = vol->getVoxelAt(i1);
            const auto v2 = vol->getVoxelAt(i2);
            const auto v3 = vol->getVoxelAt(i3);
            const auto v4 = vol->getVoxelAt(i4);
            const auto v5 = vol->getVoxelAt(i5);
            const auto v6 = vol->getVoxelAt(i6);
            const auto v7 = vol->getVoxelAt(i7);

            uint8_t cubeindex = 0;
            if (v0 != 0)
                cubeindex |= 0b00000001;
            if (v1 != 0)
                cubeindex |= 0b00000010;
            if (v2 != 0)
                cubeindex |= 0b00000100;
            if (v3 != 0)
                cubeindex |= 0b00001000;
            if (v4 != 0)
                cubeindex |= 0b00010000;
            if (v5 != 0)
                cubeindex |= 0b00100000;
            if (v6 != 0)
                cubeindex |= 0b01000000;
            if (v7 != 0)
                cubeindex |= 0b10000000;

            auto edgemask = edgeTable[cubeindex];
            glm::vec3 vertlist[12];

            if (edgemask & 1)
                vertlist[0] = glm::vec3(i0 + i1) / glm::vec3(2.f);
            if (edgemask & 2)
                vertlist[1] = glm::vec3(i1 + i2) / glm::vec3(2.f);
            if (edgemask & 4)
                vertlist[2] = glm::vec3(i2 + i3) / glm::vec3(2.f);
            if (edgemask & 8)
                vertlist[3] = glm::vec3(i3 + i0) / glm::vec3(2.f);
            if (edgemask & 16)
                vertlist[4] = glm::vec3(i4 + i5) / glm::vec3(2.f);
            if (edgemask & 32)
                vertlist[5] = glm::vec3(i5 + i6) / glm::vec3(2.f);
            if (edgemask & 64)
                vertlist[6] = glm::vec3(i6 + i7) / glm::vec3(2.f);
            if (edgemask & 128)
                vertlist[7] = glm::vec3(i7 + i4) / glm::vec3(2.f);
            if (edgemask & 256)
                vertlist[8] = glm::vec3(i0 + i4) / glm::vec3(2.f);
            if (edgemask & 512)
                vertlist[9] = glm::vec3(i1 + i5) / glm::vec3(2.f);
            if (edgemask & 1024)
                vertlist[10] = glm::vec3(i2 + i6) / glm::vec3(2.f);
            if (edgemask & 2048)
                vertlist[11] = glm::vec3(i3 + i7) / glm::vec3(2.f);

            auto& tris = triTable[cubeindex];
            for (size_t i = 0; tris[i] != -1; i += 3) {
                auto v1 = vertlist[tris[i]];
                auto v2 = vertlist[tris[i + 1]];
                auto v3 = vertlist[tris[i + 2]];
                glm::vec3 norm = glm::cross(v3 - v2, v1 - v2);

                auto id1 = addVertex(v1, norm, 1);
                auto id2 = addVertex(v2, norm, 1);
                auto id3 = addVertex(v3, norm, 1);
                addTriangleCubic(id1, id2, id3);
            }
        });
        graphics::MeshPointer mesh(std::make_shared<graphics::Mesh>());

        // convert PolyVox mesh to a Sam mesh
        gpu::BufferPointer indexBuffer(gpu::Buffer::createBuffer(gpu::Buffer::IndexBuffer, vecIndices));
        auto indexBufferPtr = gpu::BufferPointer(indexBuffer);
        gpu::BufferView indexBufferView(indexBufferPtr, gpu::Element(gpu::SCALAR, gpu::UINT32, gpu::INDEX));
        mesh->setIndexBuffer(indexBufferView);

        gpu::BufferPointer vertexBuffer(gpu::Buffer::createBuffer(gpu::Buffer::VertexBuffer, vecVertices));
        auto vertexBufferPtr = gpu::BufferPointer(vertexBuffer);

        mesh->setVertexBuffer(gpu::BufferView(vertexBufferPtr, offsetof(PositionNormalMaterial, position),
                                              vertexBufferPtr->getSize(), sizeof(PositionNormalMaterial),
                                              gpu::Element(gpu::VEC3, gpu::FLOAT, gpu::XYZ)));

        // TODO -- use 3-byte normals rather than 3-float normals
        mesh->addAttribute(gpu::Stream::NORMAL, gpu::BufferView(vertexBufferPtr, offsetof(PositionNormalMaterial, normal),
                                                                vertexBufferPtr->getSize(), sizeof(PositionNormalMaterial),
                                                                gpu::Element(gpu::VEC3, gpu::FLOAT, gpu::XYZ)));

        std::vector<graphics::Mesh::Part> parts;
        parts.emplace_back(graphics::Mesh::Part((graphics::Index)0,                  // startIndex
                                                (graphics::Index)vecIndices.size(),  // numIndices
                                                (graphics::Index)0,                  // baseVertex
                                                graphics::Mesh::TRIANGLES));         // topology
        mesh->setPartBuffer(
            gpu::BufferView(new gpu::Buffer(gpu::Buffer::IndirectBuffer, parts.size() * sizeof(graphics::Mesh::Part),
                                            (gpu::Byte*)parts.data()),
                            gpu::Element::PART_DRAWCALL));

        return mesh;
    }

private:
    uint32_t addVertex(const glm::vec3& pos, glm::vec3& norm, uint32_t mat) {
        vecVertices.emplace_back(pos, norm, static_cast<float>(mat));

        return vecVertices.size() - 1;
    }
    void addTriangleCubic(uint32_t v1, uint32_t v2, uint32_t v3) {
        vecIndices.push_back(v1);
        vecIndices.push_back(v2);
        vecIndices.push_back(v3);
    }

    std::vector<PositionNormalMaterial> vecVertices;
    std::vector<uint32_t> vecIndices;
};

}  // namespace

EntityItemPointer RenderablePolyVoxEntityItem::factory(const EntityItemID& entityID, const EntityItemProperties& properties) {
    std::shared_ptr<RenderablePolyVoxEntityItem> entity(new RenderablePolyVoxEntityItem(entityID),
                                                        [](RenderablePolyVoxEntityItem* ptr) { ptr->deleteLater(); });
    entity->setProperties(properties);
    entity->initializePolyVox();
    return entity;
}

RenderablePolyVoxEntityItem::RenderablePolyVoxEntityItem(const EntityItemID& entityItemID) : PolyVoxEntityItem(entityItemID) {
}

RenderablePolyVoxEntityItem::~RenderablePolyVoxEntityItem() {
    withWriteLock([&] { _volData.reset(); });
}

void RenderablePolyVoxEntityItem::initializePolyVox() {
    setVoxelVolumeSize(_voxelVolumeSize);
}

void RenderablePolyVoxEntityItem::setVoxelData(const QByteArray& voxelData) {
    // accept compressed voxel information from the entity-server
    bool changed = false;
    withWriteLock([&] {
        if (_voxelData != voxelData) {
            _voxelData = voxelData;
            _voxelDataDirty = true;
            changed = true;
        }
    });
    if (changed) {
        startUpdates();
    }
}

void RenderablePolyVoxEntityItem::setVoxelSurfaceStyle(uint16_t voxelSurfaceStyle) {
    // this controls whether the polyvox surface extractor does marching-cubes or makes a cubic mesh.  It
    // also determines if the extra "edged" layer is used.
    bool volSizeChanged = false;

    withWriteLock([&] {
        if (_voxelSurfaceStyle == voxelSurfaceStyle) {
            return;
        }

        // if we are switching to or from "edged" we need to force a resize of _volData.
        bool wasEdged = isEdged();
        bool willBeEdged = isEdged((PolyVoxSurfaceStyle)voxelSurfaceStyle);

        if (wasEdged != willBeEdged) {
            _volData.reset();
            _voxelDataDirty = true;
            volSizeChanged = true;
        }
        _voxelSurfaceStyle = voxelSurfaceStyle;
        startUpdates();
    });

    if (volSizeChanged) {
        // setVoxelVolumeSize will re-alloc _volData with the right size
        setVoxelVolumeSize(_voxelVolumeSize);
    }

    _updateFromNeighborXEdge = _updateFromNeighborYEdge = _updateFromNeighborZEdge = true;
    tellNeighborsToRecopyEdges(true);
    startUpdates();
}

bool RenderablePolyVoxEntityItem::setVoxel(const ivec3& v, uint8_t toValue) {
    if (_locked) {
        return false;
    }

    bool result = false;
    withWriteLock([&] { result = setVoxelInternal(v, toValue); });
    if (result) {
        startUpdates();
    }

    return result;
}

void RenderablePolyVoxEntityItem::forEachVoxelValue(const ivec3& voxelSize,
                                                    std::function<void(const ivec3& v, uint8_t)> thunk) {
    // a thread-safe way for code outside this class to iterate over a range of voxels
    withReadLock([&] {
        loop3(ivec3(0), voxelSize, [&](const ivec3& v) {
            uint8_t uVoxelValue = getVoxelInternal(v);
            thunk(v, uVoxelValue);
        });
    });
}

QByteArray RenderablePolyVoxEntityItem::volDataToArray(quint16 voxelXSize, quint16 voxelYSize, quint16 voxelZSize) const {
    int totalSize = voxelXSize * voxelYSize * voxelZSize;
    ivec3 voxelSize{ voxelXSize, voxelYSize, voxelZSize };
    ivec3 low{ 0 };
    int index = 0;

    QByteArray result = QByteArray(totalSize, '\0');
    withReadLock([&] {
        if (isEdged()) {
            low += 1;
        }

        loop3(ivec3(0), voxelSize, [&](const ivec3& v) { result[index++] = _volData->getVoxelAt(v + low); });
    });

    return result;
}

bool RenderablePolyVoxEntityItem::setAll(uint8_t toValue) {
    bool result = false;
    if (_locked) {
        return result;
    }

    withWriteLock(
        [&] { loop3(ivec3(0), ivec3(_voxelVolumeSize), [&](const ivec3& v) { result |= setVoxelInternal(v, toValue); }); });
    if (result) {
        startUpdates();
    }
    return result;
}

bool RenderablePolyVoxEntityItem::setCuboid(const glm::vec3& lowPosition, const glm::vec3& cuboidSize, int toValue) {
    bool result = false;
    if (_locked) {
        return result;
    }

    ivec3 iLowPosition = ivec3{ glm::round(lowPosition) };
    ivec3 iCuboidSize = ivec3{ glm::round(cuboidSize) };
    ivec3 iVoxelVolumeSize = ivec3{ glm::round(_voxelVolumeSize) };

    ivec3 low = glm::max(glm::min(iLowPosition, iVoxelVolumeSize - 1), ivec3(0));
    ivec3 high = glm::max(glm::min(low + iCuboidSize, iVoxelVolumeSize), low);

    withWriteLock([&] { loop3(low, high, [&](const ivec3& v) { result |= setVoxelInternal(v, toValue); }); });
    if (result) {
        startUpdates();
    }
    return result;
}

bool RenderablePolyVoxEntityItem::setVoxelInVolume(const vec3& position, uint8_t toValue) {
    if (_locked) {
        return false;
    }

    // same as setVoxel but takes a vector rather than 3 floats.
    return setVoxel(glm::round(position), toValue);
}

bool RenderablePolyVoxEntityItem::setSphereInVolume(const vec3& center, float radius, uint8_t toValue) {
    bool result = false;
    if (_locked) {
        return result;
    }

    float radiusSquared = radius * radius;
    // This three-level for loop iterates over every voxel in the volume
    withWriteLock([&] {
        loop3(ivec3(0), ivec3(_voxelVolumeSize), [&](const ivec3& v) {
            // Store our current position as a vector...
            glm::vec3 pos = vec3(v) + 0.5f;  // consider voxels cenetered on their coordinates
            // And compute how far the current position is from the center of the volume
            float fDistToCenterSquared = glm::distance2(pos, center);
            // If the current voxel is less than 'radius' units from the center then we set its value
            if (fDistToCenterSquared <= radiusSquared) {
                result |= setVoxelInternal(v, toValue);
            }
        });
    });
    if (result) {
        startUpdates();
    }

    return result;
}

bool RenderablePolyVoxEntityItem::setSphere(const vec3& centerWorldCoords, float radiusWorldCoords, uint8_t toValue) {
    bool result = false;
    if (_locked) {
        return result;
    }

    glm::mat4 vtwMatrix = voxelToWorldMatrix();
    glm::mat4 wtvMatrix = glm::inverse(vtwMatrix);

    glm::vec3 dimensions = getScaledDimensions();
    glm::vec3 voxelSize = dimensions / _voxelVolumeSize;
    float smallestDimensionSize = voxelSize.x;
    smallestDimensionSize = glm::min(smallestDimensionSize, voxelSize.y);
    smallestDimensionSize = glm::min(smallestDimensionSize, voxelSize.z);
    if (smallestDimensionSize <= 0.0f) {
        return false;
    }

    glm::vec3 maxRadiusInVoxelCoords = glm::vec3(radiusWorldCoords / smallestDimensionSize);
    glm::vec3 centerInVoxelCoords = wtvMatrix * glm::vec4(centerWorldCoords, 1.0f);

    glm::vec3 low = glm::floor(centerInVoxelCoords - maxRadiusInVoxelCoords);
    glm::vec3 high = glm::ceil(centerInVoxelCoords + maxRadiusInVoxelCoords);

    glm::ivec3 lowI = glm::clamp(low, glm::vec3(0.0f), _voxelVolumeSize);
    glm::ivec3 highI = glm::clamp(high, glm::vec3(0.0f), _voxelVolumeSize);

    glm::vec3 radials(radiusWorldCoords / voxelSize.x, radiusWorldCoords / voxelSize.y, radiusWorldCoords / voxelSize.z);

    // This three-level for loop iterates over every voxel in the volume that might be in the sphere
    withWriteLock([&] {
        loop3(lowI, highI, [&](const ivec3& v) {
            // set voxels whose bounding-box touches the sphere
            AABox voxelBox(glm::vec3(v) - 0.5f, glm::vec3(1.0f, 1.0f, 1.0f));
            if (voxelBox.touchesAAEllipsoid(centerInVoxelCoords, radials)) {
                result |= setVoxelInternal(v, toValue);
            }

            // TODO -- this version only sets voxels which have centers inside the sphere.  which is best?
            // // Store our current position as a vector...
            // glm::vec4 pos(x + 0.5f, y + 0.5f, z + 0.5f, 1.0); // consider voxels cenetered on their coordinates
            // // convert to world coordinates
            // glm::vec3 worldPos = glm::vec3(vtwMatrix * pos);
            // // compute how far the current position is from the center of the volume
            // float fDistToCenter = glm::distance(worldPos, centerWorldCoords);
            // // If the current voxel is less than 'radius' units from the center then we set its value
            // if (fDistToCenter <= radiusWorldCoords) {
            //     result |= setVoxelInternal(x, y, z, toValue);
            // }
        });
    });
    if (result) {
        startUpdates();
    }

    return result;
}

bool RenderablePolyVoxEntityItem::setCapsule(const vec3& startWorldCoords,
                                             const vec3& endWorldCoords,
                                             float radiusWorldCoords,
                                             uint8_t toValue) {
    bool result = false;
    if (_locked) {
        return result;
    }

    glm::mat4 vtwMatrix = voxelToWorldMatrix();
    glm::mat4 wtvMatrix = glm::inverse(vtwMatrix);

    glm::vec3 dimensions = getScaledDimensions();
    glm::vec3 voxelSize = dimensions / _voxelVolumeSize;
    float smallestDimensionSize = voxelSize.x;
    smallestDimensionSize = glm::min(smallestDimensionSize, voxelSize.y);
    smallestDimensionSize = glm::min(smallestDimensionSize, voxelSize.z);

    glm::vec3 maxRadiusInVoxelCoords = glm::vec3(radiusWorldCoords / smallestDimensionSize);

    glm::vec3 startInVoxelCoords = wtvMatrix * glm::vec4(startWorldCoords, 1.0f);
    glm::vec3 endInVoxelCoords = wtvMatrix * glm::vec4(endWorldCoords, 1.0f);

    glm::vec3 low = glm::min(glm::floor(startInVoxelCoords - maxRadiusInVoxelCoords),
                             glm::floor(endInVoxelCoords - maxRadiusInVoxelCoords));
    glm::vec3 high =
        glm::max(glm::ceil(startInVoxelCoords + maxRadiusInVoxelCoords), glm::ceil(endInVoxelCoords + maxRadiusInVoxelCoords));

    glm::ivec3 lowI = glm::clamp(low, glm::vec3(0.0f), _voxelVolumeSize);
    glm::ivec3 highI = glm::clamp(high, glm::vec3(0.0f), _voxelVolumeSize);

    // This three-level for loop iterates over every voxel in the volume that might be in the capsule
    withWriteLock([&] {
        loop3(lowI, highI, [&](const ivec3& v) {
            // Store our current position as a vector...
            glm::vec4 pos{ vec3(v) + 0.5f, 1.0 };  // consider voxels cenetered on their coordinates
            // convert to world coordinates
            glm::vec3 worldPos = glm::vec3(vtwMatrix * pos);
            if (pointInCapsule(worldPos, startWorldCoords, endWorldCoords, radiusWorldCoords)) {
                result |= setVoxelInternal(v, toValue);
            }
        });
    });
    if (result) {
        startUpdates();
    }

    return result;
}

bool RenderablePolyVoxEntityItem::findDetailedRayIntersection(const glm::vec3& origin,
                                                              const glm::vec3& direction,
                                                              const glm::vec3& viewFrustumPos,
                                                              OctreeElementPointer& element,
                                                              float& distance,
                                                              BoxFace& face,
                                                              glm::vec3& surfaceNormal,
                                                              QVariantMap& extraInfo,
                                                              bool precisionPicking) const {
    // TODO -- correctly pick against marching-cube generated meshes
    if (!precisionPicking) {
        // just intersect with bounding box
        return true;
    }

    glm::mat4 wtvMatrix = worldToVoxelMatrix(true);
    glm::vec3 normDirection = glm::normalize(direction);

    // the PolyVox ray intersection code requires a near and far point.
    // set ray cast length to long enough to cover all of the voxel space
    float distanceToEntity = glm::distance(origin, getWorldPosition());
    glm::vec3 dimensions = getScaledDimensions();
    float largestDimension = glm::compMax(dimensions) * 2.0f;
    glm::vec3 farPoint = origin + normDirection * (distanceToEntity + largestDimension);

    glm::vec4 originInVoxel = wtvMatrix * glm::vec4(origin, 1.0f);
    glm::vec4 farInVoxel = wtvMatrix * glm::vec4(farPoint, 1.0f);

    std::optional<glm::ivec3> resultor;
    withReadLock([&] { resultor = _volData->raycast(originInVoxel, farInVoxel); });
    if (!resultor.has_value()) {
        // the ray completed its path -- nothing was hit.
        return false;
    }

    glm::vec3 result3 = glm::vec3(std::move(resultor).value());

    AABox voxelBox;
    voxelBox += result3 - Vectors::HALF;
    voxelBox += result3 + Vectors::HALF;

    glm::vec3 directionInVoxel = vec3(wtvMatrix * glm::vec4(direction, 0.0f));
    return voxelBox.findRayIntersection(glm::vec3(originInVoxel), directionInVoxel, 1.0f / directionInVoxel, distance, face,
                                        surfaceNormal);
}

bool RenderablePolyVoxEntityItem::findDetailedParabolaIntersection(const glm::vec3& origin,
                                                                   const glm::vec3& velocity,
                                                                   const glm::vec3& acceleration,
                                                                   const glm::vec3& viewFrustumPos,
                                                                   OctreeElementPointer& element,
                                                                   float& parabolicDistance,
                                                                   BoxFace& face,
                                                                   glm::vec3& surfaceNormal,
                                                                   QVariantMap& extraInfo,
                                                                   bool precisionPicking) const {
    // TODO -- correctly pick against marching-cube generated meshes
    if (!precisionPicking) {
        // just intersect with bounding box
        return true;
    }

    glm::mat4 wtvMatrix = worldToVoxelMatrix(true);
    glm::vec4 originInVoxel = wtvMatrix * glm::vec4(origin, 1.0f);
    glm::vec4 velocityInVoxel = wtvMatrix * glm::vec4(velocity, 0.0f);
    glm::vec4 accelerationInVoxel = wtvMatrix * glm::vec4(acceleration, 0.0f);

    // find the first intersection with the voxel bounding box (slightly enlarged so we can catch voxels that touch the sides)
    bool success;
    glm::vec3 center = getCenterPosition(success);
    glm::vec3 dimensions = getScaledDimensions();
    const float FIRST_BOX_HALF_SCALE = 0.51f;
    AABox voxelBox1(wtvMatrix * vec4(center - FIRST_BOX_HALF_SCALE * dimensions, 1.0f),
                    wtvMatrix * vec4(2.0f * FIRST_BOX_HALF_SCALE * dimensions, 0.0f));
    bool hit1;
    float parabolicDistance1;
    // If we're starting inside the box, our first point is originInVoxel
    if (voxelBox1.contains(originInVoxel)) {
        parabolicDistance1 = 0.0f;
        hit1 = true;
    } else {
        BoxFace face1;
        glm::vec3 surfaceNormal1;
        hit1 = voxelBox1.findParabolaIntersection(glm::vec3(originInVoxel), glm::vec3(velocityInVoxel),
                                                  glm::vec3(accelerationInVoxel), parabolicDistance1, face1, surfaceNormal1);
    }

    if (hit1) {
        // find the second intersection, which should be with the inside of the box (use a slightly large box again)
        const float SECOND_BOX_HALF_SCALE = 0.52f;
        AABox voxelBox2(wtvMatrix * vec4(center - SECOND_BOX_HALF_SCALE * dimensions, 1.0f),
                        wtvMatrix * vec4(2.0f * SECOND_BOX_HALF_SCALE * dimensions, 0.0f));
        glm::vec4 originInVoxel2 = originInVoxel + velocityInVoxel * parabolicDistance1 +
                                   0.5f * accelerationInVoxel * parabolicDistance1 * parabolicDistance1;
        glm::vec4 velocityInVoxel2 = velocityInVoxel + accelerationInVoxel * parabolicDistance1;
        glm::vec4 accelerationInVoxel2 = accelerationInVoxel;
        float parabolicDistance2;
        BoxFace face2;
        glm::vec3 surfaceNormal2;
        // this should always be true
        if (voxelBox2.findParabolaIntersection(glm::vec3(originInVoxel2), glm::vec3(velocityInVoxel2),
                                               glm::vec3(accelerationInVoxel2), parabolicDistance2, face2, surfaceNormal2)) {
            const int MAX_SECTIONS = 15;
            std::optional<glm::ivec3> resultor;
            glm::vec4 segmentStartVoxel = originInVoxel2;
            for (int i = 0; i < MAX_SECTIONS; i++) {
                float t = parabolicDistance2 * ((float)(i + 1)) / ((float)MAX_SECTIONS);
                glm::vec4 segmentEndVoxel = originInVoxel2 + velocityInVoxel2 * t + 0.5f * accelerationInVoxel2 * t * t;
                withReadLock([&] { resultor = _volData->raycast(segmentStartVoxel, segmentEndVoxel); });
                if (!resultor.has_value()) {
                    // We hit something!
                    break;
                }
                segmentStartVoxel = segmentEndVoxel;
            }

            if (!resultor.has_value()) {
                // the parabola completed its path -- nothing was hit.
                return false;
            }

            glm::vec3 result3 = glm::vec3(std::move(resultor).value());

            AABox voxelBox;
            voxelBox += result3 - Vectors::HALF;
            voxelBox += result3 + Vectors::HALF;

            return voxelBox.findParabolaIntersection(glm::vec3(originInVoxel), glm::vec3(velocityInVoxel),
                                                     glm::vec3(accelerationInVoxel), parabolicDistance, face, surfaceNormal);
        }
    }
    return false;
}

// virtual
ShapeType RenderablePolyVoxEntityItem::getShapeType() const {
    if (_collisionless) {
        return SHAPE_TYPE_NONE;
    }
    return SHAPE_TYPE_COMPOUND;
}

void RenderablePolyVoxEntityItem::setRegistrationPoint(const glm::vec3& value) {
    if (value != getRegistrationPoint()) {
        withWriteLock([&] { _shapeReady = false; });
        EntityItem::setRegistrationPoint(value);
        startUpdates();
    }
}

bool RenderablePolyVoxEntityItem::isReadyToComputeShape() const {
    ShapeType shapeType = getShapeType();
    if (shapeType == SHAPE_TYPE_NONE) {
        return true;
    }

    bool result;
    withReadLock([&] { result = _shapeReady; });
    return result;
}

void RenderablePolyVoxEntityItem::computeShapeInfo(ShapeInfo& info) {
    ShapeType shapeType = getShapeType();
    if (shapeType == SHAPE_TYPE_NONE) {
        info.setParams(getShapeType(), 0.5f * getScaledDimensions());
        return;
    }

    withReadLock([&] { info = _shapeInfo; });
}

void RenderablePolyVoxEntityItem::changeUpdates(bool value) {
    if (_updateNeeded != value) {
        EntityTreePointer entityTree = getTree();
        if (entityTree) {
            EntitySimulationPointer simulation = entityTree->getSimulation();
            if (simulation) {
                _updateNeeded = value;
                markDirtyFlags(Simulation::DIRTY_UPDATEABLE);
                simulation->changeEntity(getThisPointer());
            }
        }
    }
}

void RenderablePolyVoxEntityItem::startUpdates() {
    changeUpdates(true);
}

void RenderablePolyVoxEntityItem::stopUpdates() {
    changeUpdates(false);
}

void RenderablePolyVoxEntityItem::update(const quint64& now) {
    bool doRecomputeMesh{ false };
    bool doUncompress{ false };
    bool doCompress{ false };
    bool doRecomputeShape{ false };

    withWriteLock([&] {
        tellNeighborsToRecopyEdges(false);

        switch (_state) {
            case PolyVoxState::Ready: {
                if (_volDataDirty) {
                    _volDataDirty = _voxelDataDirty = false;
                    _state = PolyVoxState::BakingMesh;
                    doRecomputeMesh = true;
                } else if (_voxelDataDirty) {
                    _voxelDataDirty = false;
                    _state = PolyVoxState::Uncompressing;
                    doUncompress = true;
                } else {
                    copyUpperEdgesFromNeighbors();
                    if (!_volDataDirty && !_voxelDataDirty) {
                        // nothing to do
                        stopUpdates();
                    }
                }
                break;
            }

            case PolyVoxState::Uncompressing: {
                break;  // wait
            }
            case PolyVoxState::UncompressingFinished: {
                if (_volDataDirty) {
                    _volDataDirty = _voxelDataDirty = false;
                    _state = PolyVoxState::BakingMeshNoCompress;
                    doRecomputeMesh = true;
                } else if (_voxelDataDirty) {
                    _voxelDataDirty = false;
                    // _voxelData changed while we were uncompressing the previous version, uncompress again
                    _state = PolyVoxState::Uncompressing;
                    doUncompress = true;
                } else {
                    _state = PolyVoxState::Ready;
                }
                break;
            }

            case PolyVoxState::BakingMesh: {
                break;  // wait
            }
            case PolyVoxState::BakingMeshFinished: {
                if (_volDataDirty) {
                    _volDataDirty = _voxelDataDirty = false;
                    _state = PolyVoxState::BakingMesh;
                    // a local edit happened while we were baking the mesh.  rebake mesh...
                    doRecomputeMesh = true;
                } else if (_voxelDataDirty) {
                    _voxelDataDirty = false;
                    // we received a change from the wire while baking the mesh.
                    _state = PolyVoxState::Uncompressing;
                    doUncompress = true;
                } else {
                    _state = PolyVoxState::Compressing;
                    doCompress = true;
                }
                break;
            }

            case PolyVoxState::BakingMeshNoCompress: {
                break;  // wait
            }
            case PolyVoxState::BakingMeshNoCompressFinished: {
                if (_volDataDirty) {
                    _volDataDirty = _voxelDataDirty = false;
                    _state = PolyVoxState::BakingMesh;
                    // a local edit happened while we were baking the mesh.  rebake mesh...
                    doRecomputeMesh = true;
                } else if (_voxelDataDirty) {
                    _voxelDataDirty = false;
                    // we received a change from the wire while baking the mesh.
                    _state = PolyVoxState::Uncompressing;
                    doUncompress = true;
                } else {
                    _state = PolyVoxState::BakingShape;
                    doRecomputeShape = true;
                }
                break;
            }

            case PolyVoxState::Compressing: {
                break;  // wait
            }
            case PolyVoxState::CompressingFinished: {
                _state = PolyVoxState::BakingShape;
                doRecomputeShape = true;
                break;
            }

            case PolyVoxState::BakingShape: {
                break;  // wait
            }
            case PolyVoxState::BakingShapeFinished: {
                _state = PolyVoxState::Ready;
                break;
            }
        }
    });

    if (doRecomputeMesh) {
        recomputeMesh();
    }
    if (doUncompress) {
        uncompressVolumeData();
    }
    if (doCompress) {
        compressVolumeDataAndSendEditPacket();
    }
    if (doRecomputeShape) {
        computeShapeInfoWorker();
    }
}

void RenderablePolyVoxEntityItem::setVoxelVolumeSize(const glm::vec3& voxelVolumeSize) {
    // This controls how many individual voxels are in the entity.  This is unrelated to
    // the dimentions of the entity -- it defines the sizes of the arrays that hold voxel values.
    // In addition to setting the number of voxels, this is used in a few places for its
    // side-effect of allocating _volData to be the correct size.
    withWriteLock([&] {
        if (_volData && _voxelVolumeSize == voxelVolumeSize) {
            return;
        }

        _voxelDataDirty = true;
        _voxelVolumeSize = voxelVolumeSize;
        _volData.reset();
        _onCount = 0;
        _updateFromNeighborXEdge = _updateFromNeighborYEdge = _updateFromNeighborZEdge = true;
        startUpdates();

        glm::ivec3 size;

        if (isEdged()) {
            // with _EDGED_ we maintain an extra box of voxels around those that the user asked for.  This
            // changes how the surface extractor acts -- it becomes impossible to have holes in the
            // generated mesh.  The non _EDGED_ modes will leave holes in the mesh at the edges of the
            // voxel space.
            size = glm::ivec3(_voxelVolumeSize) + glm::ivec3(1);  // corners are inclusive
        } else {
            // these should each have -1 after them, but if we leave layers on the upper-axis faces,
            // they act more like I expect.
            size = glm::ivec3(_voxelVolumeSize);
        }

        _volData.reset(new VoxelVolume(size));
        // having the "outside of voxel-space" value be 255 has helped me notice some problems.
        //_volData->setBorderValue(255);
    });

    tellNeighborsToRecopyEdges(true);
}

bool inUserBounds(const std::shared_ptr<VoxelVolume> vol, PolyVoxEntityItem::PolyVoxSurfaceStyle surfaceStyle, const ivec3& v) {
    if (glm::any(glm::lessThan(v, ivec3(0)))) {
        return false;
    }

    glm::ivec3 volume = vol->getSize();
    // x, y, z are in user voxel-coords, not adjusted-for-edge voxel-coords.
    if (PolyVoxEntityItem::isEdged(surfaceStyle)) {
        volume -= 2;
    }

    if (glm::any(glm::greaterThanEqual(v, volume))) {
        return false;
    }

    return true;
}

uint8_t RenderablePolyVoxEntityItem::getVoxel(const ivec3& v) const {
    uint8_t result;
    withReadLock([&] { result = getVoxelInternal(v); });
    return result;
}

uint8_t RenderablePolyVoxEntityItem::getVoxelInternal(const ivec3& v) const {
    if (!inUserBounds(_volData, (PolyVoxSurfaceStyle)_voxelSurfaceStyle, v)) {
        return 0;
    }

    // if _voxelSurfaceStyle is *_EDGED_*, we maintain an extra layer of
    // voxels all around the requested voxel space.  Having the empty voxels around
    // the edges changes how the surface extractor behaves.
    if (isEdged()) {
        return _volData->getVoxelAt(v + glm::ivec3(1));
    }
    return _volData->getVoxelAt(v);
}

void RenderablePolyVoxEntityItem::setVoxelMarkNeighbors(int x, int y, int z, uint8_t toValue) {
    _volData->setVoxelAt(x, y, z, toValue);
    if (x == 0) {
        _neighborXNeedsUpdate = true;
        startUpdates();
    }
    if (y == 0) {
        _neighborYNeedsUpdate = true;
        startUpdates();
    }
    if (z == 0) {
        _neighborZNeedsUpdate = true;
        startUpdates();
    }
}

bool RenderablePolyVoxEntityItem::setVoxelInternal(const ivec3& v, uint8_t toValue) {
    // set a voxel without recompressing the voxel data.  This assumes that the caller has write-locked the entity.
    bool result = updateOnCount(v, toValue);
    if (result) {
        if (isEdged()) {
            setVoxelMarkNeighbors(v.x + 1, v.y + 1, v.z + 1, toValue);
        } else {
            setVoxelMarkNeighbors(v.x, v.y, v.z, toValue);
        }
        _volDataDirty = true;
    }
    return result;
}

bool RenderablePolyVoxEntityItem::updateOnCount(const ivec3& v, uint8_t toValue) {
    // keep _onCount up to date
    if (!inUserBounds(_volData, (PolyVoxSurfaceStyle)_voxelSurfaceStyle, v)) {
        return false;
    }

    uint8_t uVoxelValue = getVoxelInternal(v);
    if (toValue != 0) {
        if (uVoxelValue == 0) {
            _onCount++;
            return true;
        }
    } else {
        // toValue == 0
        if (uVoxelValue != 0) {
            _onCount--;
            assert(_onCount >= 0);
            return true;
        }
    }
    return false;
}

void RenderablePolyVoxEntityItem::uncompressVolumeData() {
    // take compressed data and expand it into _volData.
    QByteArray voxelData;
    auto entity = std::static_pointer_cast<RenderablePolyVoxEntityItem>(getThisPointer());

    withReadLock([&] { voxelData = _voxelData; });

    QtConcurrent::run([=, this] {
        QDataStream reader(voxelData);
        quint16 voxelXSize, voxelYSize, voxelZSize;
        reader >> voxelXSize;
        reader >> voxelYSize;
        reader >> voxelZSize;

        if (voxelXSize == 0 || voxelXSize > PolyVoxEntityItem::MAX_VOXEL_DIMENSION || voxelYSize == 0 ||
            voxelYSize > PolyVoxEntityItem::MAX_VOXEL_DIMENSION || voxelZSize == 0 ||
            voxelZSize > PolyVoxEntityItem::MAX_VOXEL_DIMENSION) {
            qCDebug(entitiesrenderer) << "voxelSize is not reasonable, skipping uncompressions." << voxelXSize << voxelYSize
                                      << voxelZSize << getName() << getID();
            entity->setVoxelsFromData(QByteArray(1, 0), 1, 1, 1);
            return;
        }

        int rawSize = voxelXSize * voxelYSize * voxelZSize;

        QByteArray compressedData;
        reader >> compressedData;

        QByteArray uncompressedData = qUncompress(compressedData);

        if (uncompressedData.size() != rawSize) {
            qCDebug(entitiesrenderer) << "PolyVox uncompress -- size is (" << voxelXSize << voxelYSize << voxelZSize << ")"
                                      << "so expected uncompressed length of" << rawSize << "but length is"
                                      << uncompressedData.size() << getName() << getID();
            entity->setVoxelsFromData(QByteArray(1, 0), 1, 1, 1);
            return;
        }

        entity->setVoxelsFromData(uncompressedData, voxelXSize, voxelYSize, voxelZSize);
    });
}

void RenderablePolyVoxEntityItem::setVoxelsFromData(QByteArray uncompressedData,
                                                    quint16 voxelXSize,
                                                    quint16 voxelYSize,
                                                    quint16 voxelZSize) {
    // this accepts the payload from uncompressVolumeData
    ivec3 low{ 0 };
    bool result = false;

    withWriteLock([&] {
        if (isEdged()) {
            low += 1;
        }
        loop3(ivec3(0), ivec3(voxelXSize, voxelYSize, voxelZSize), [&](const ivec3& v) {
            int uncompressedIndex = (v.z * (voxelYSize) * (voxelXSize)) + (v.y * (voxelZSize)) + v.x;
            result |= setVoxelInternal(v, uncompressedData[uncompressedIndex]);
        });

        _state = PolyVoxState::UncompressingFinished;
    });
    if (result) {
        startUpdates();
    }
}

void RenderablePolyVoxEntityItem::compressVolumeDataAndSendEditPacket() {
    // compress the data in _volData and save the results.  The compressed form is used during
    // saves to disk and for transmission over the wire to the entity-server

    EntityItemPointer entity = getThisPointer();

    quint16 voxelXSize;
    quint16 voxelYSize;
    quint16 voxelZSize;
    withReadLock([&] {
        voxelXSize = _voxelVolumeSize.x;
        voxelYSize = _voxelVolumeSize.y;
        voxelZSize = _voxelVolumeSize.z;
    });

#ifdef WANT_DEBUG
    qDebug() << "Compressing voxel and sending data packet";
#endif

    QtConcurrent::run([voxelXSize, voxelYSize, voxelZSize, entity] {
        auto polyVoxEntity = std::static_pointer_cast<RenderablePolyVoxEntityItem>(entity);
        QByteArray uncompressedData = polyVoxEntity->volDataToArray(voxelXSize, voxelYSize, voxelZSize);

        QByteArray newVoxelData;
        QDataStream writer(&newVoxelData, QIODevice::WriteOnly | QIODevice::Truncate);

        writer << voxelXSize << voxelYSize << voxelZSize;

        QByteArray compressedData = qCompress(uncompressedData, 9);
        writer << compressedData;

        // make sure the compressed data can be sent over the wire-protocol
        if (newVoxelData.size() > 1150) {
            // HACK -- until we have a way to allow for properties larger than MTU, don't update.
            // revert the active voxel-space to the last version that fit.
            qCDebug(entitiesrenderer) << "compressed voxel data is too large" << entity->getName() << entity->getID();

            const auto polyVoxEntity = std::static_pointer_cast<RenderablePolyVoxEntityItem>(entity);
            polyVoxEntity->compressVolumeDataFinished(QByteArray());
            return;
        }

        std::static_pointer_cast<RenderablePolyVoxEntityItem>(entity)->compressVolumeDataFinished(newVoxelData);
    });
}

void RenderablePolyVoxEntityItem::compressVolumeDataFinished(const QByteArray& voxelData) {
    // compressed voxel information from the entity-server
    withWriteLock([&] {
        if (voxelData.size() > 0 && _voxelData != voxelData) {
            _voxelData = voxelData;
        }
        _state = PolyVoxState::CompressingFinished;
    });

    auto now = usecTimestampNow();
    setLastEdited(now);
    setLastBroadcast(now);

    EntityTreeElementPointer element = getElement();
    EntityTreePointer tree = element ? element->getTree() : nullptr;

    if (tree) {
        tree->withReadLock([&] {
            EntityPropertyFlags desiredProperties;
            desiredProperties.setHasProperty(PROP_VOXEL_DATA);
            EntityItemProperties properties = getProperties(desiredProperties, false);
            properties.setVoxelDataDirty();
            properties.setLastEdited(now);

            EntitySimulationPointer simulation = tree ? tree->getSimulation() : nullptr;
            PhysicalEntitySimulationPointer peSimulation = std::static_pointer_cast<PhysicalEntitySimulation>(simulation);
            EntityEditPacketSender* packetSender = peSimulation ? peSimulation->getPacketSender() : nullptr;
            if (packetSender) {
                packetSender->queueEditEntityMessage(PacketType::EntityEdit, tree, getID(), properties);
            }
        });
    }
}

EntityItemPointer lookUpNeighbor(EntityTreePointer tree, EntityItemID neighborID, EntityItemWeakPointer& currentWP) {
    EntityItemPointer current = currentWP.lock();

    if (!current && neighborID == UNKNOWN_ENTITY_ID) {
        // no neighbor
        return nullptr;
    }

    if (current && current->getID() == neighborID) {
        // same neighbor
        return current;
    }

    if (neighborID == UNKNOWN_ENTITY_ID) {
        currentWP.reset();
        return nullptr;
    }

    current = tree->findEntityByID(neighborID);
    if (!current) {
        return nullptr;
    }

    currentWP = current;
    return current;
}

void RenderablePolyVoxEntityItem::cacheNeighbors() {
    // this attempts to turn neighbor entityIDs into neighbor weak-pointers
    EntityTreeElementPointer element = getElement();
    EntityTreePointer tree = element ? element->getTree() : nullptr;
    if (!tree) {
        return;
    }
    lookUpNeighbor(tree, _xNNeighborID, _xNNeighbor);
    lookUpNeighbor(tree, _yNNeighborID, _yNNeighbor);
    lookUpNeighbor(tree, _zNNeighborID, _zNNeighbor);
    lookUpNeighbor(tree, _xPNeighborID, _xPNeighbor);
    lookUpNeighbor(tree, _yPNeighborID, _yPNeighbor);
    lookUpNeighbor(tree, _zPNeighborID, _zPNeighbor);
}

void RenderablePolyVoxEntityItem::copyUpperEdgesFromNeighbors() {
    // fill in our upper edges with a copy of our neighbors lower edges so that the meshes knit together
    if ((PolyVoxSurfaceStyle)_voxelSurfaceStyle != PolyVoxEntityItem::SURFACE_MARCHING_CUBES) {
        return;
    }

    if (!_updateFromNeighborXEdge && !_updateFromNeighborYEdge && !_updateFromNeighborZEdge) {
        return;
    }

    cacheNeighbors();

    auto size = _volData->getSize();

    if (_updateFromNeighborXEdge) {
        _updateFromNeighborXEdge = false;
        auto currentXPNeighbor = getXPNeighbor();
        if (currentXPNeighbor && currentXPNeighbor->getVoxelVolumeSize() == _voxelVolumeSize) {
            withWriteLock([&] {
                int x = size.x - 1;
                loop2(glm::ivec2(0), glm::ivec2(size.y, size.z), [&](glm::ivec2 index_) {
                    const auto index = glm::ivec3(x, index_.x, index_.y);
                    uint8_t neighborValue = currentXPNeighbor->getVoxel({ 0, index_.x, index_.y });
                    uint8_t prevValue = _volData->getVoxelAt(index);
                    if (prevValue != neighborValue) {
                        _volData->setVoxelAt(index, neighborValue);
                        _volDataDirty = true;
                    }
                });
            });
        }
    }

    if (_updateFromNeighborYEdge) {
        _updateFromNeighborYEdge = false;
        auto currentYPNeighbor = getYPNeighbor();
        if (currentYPNeighbor && currentYPNeighbor->getVoxelVolumeSize() == _voxelVolumeSize) {
            withWriteLock([&] {
                int y = size.y - 1;
                loop2(glm::ivec2(0), glm::ivec2(size.x, size.z), [&](glm::ivec2 index_) {
                    const auto index = glm::ivec3(index_.x, y, index_.y);
                    uint8_t neighborValue = currentYPNeighbor->getVoxel({ index_.x, 0, index_.y });
                    uint8_t prevValue = _volData->getVoxelAt(index);
                    if (prevValue != neighborValue) {
                        _volData->setVoxelAt(index, neighborValue);
                        _volDataDirty = true;
                    }
                });
            });
        }
    }

    if (_updateFromNeighborZEdge) {
        _updateFromNeighborZEdge = false;
        auto currentZPNeighbor = getZPNeighbor();
        if (currentZPNeighbor && currentZPNeighbor->getVoxelVolumeSize() == _voxelVolumeSize) {
            withWriteLock([&] {
                int z = size.z - 1;
                loop2(glm::ivec2(0), glm::ivec2(size.x, size.y), [&](glm::ivec2 index_) {
                    const auto index = glm::ivec3(index_.x, index_.y, z);
                    uint8_t neighborValue = currentZPNeighbor->getVoxel({ index_.x, index_.y, 0 });
                    uint8_t prevValue = _volData->getVoxelAt(index);
                    if (prevValue != neighborValue) {
                        _volData->setVoxelAt(index, neighborValue);
                        _volDataDirty = true;
                    }
                });
            });
        }
    }
}

void RenderablePolyVoxEntityItem::tellNeighborsToRecopyEdges(bool force) {
    // if this polyvox has changed any of its voxels with a zero coord (in x, y, or z) notify neighbors, if there are any
    if (force || _neighborXNeedsUpdate || _neighborYNeedsUpdate || _neighborZNeedsUpdate) {
        cacheNeighbors();

        if (force || _neighborXNeedsUpdate) {
            _neighborXNeedsUpdate = false;
            auto currentXNNeighbor = getXNNeighbor();
            if (currentXNNeighbor) {
                currentXNNeighbor->neighborXEdgeChanged();
            }
        }
        if (force || _neighborYNeedsUpdate) {
            _neighborYNeedsUpdate = false;
            auto currentYNNeighbor = getYNNeighbor();
            if (currentYNNeighbor) {
                currentYNNeighbor->neighborYEdgeChanged();
            }
        }
        if (force || _neighborZNeedsUpdate) {
            _neighborZNeedsUpdate = false;
            auto currentZNNeighbor = getZNNeighbor();
            if (currentZNNeighbor) {
                currentZNNeighbor->neighborZEdgeChanged();
            }
        }
    }
}

void RenderablePolyVoxEntityItem::recomputeMesh() {
    // use _volData to make a renderable mesh
    PolyVoxSurfaceStyle voxelSurfaceStyle;
    withReadLock([&] { voxelSurfaceStyle = (PolyVoxSurfaceStyle)_voxelSurfaceStyle; });

    auto entity = std::static_pointer_cast<RenderablePolyVoxEntityItem>(getThisPointer());

    QtConcurrent::run([entity, voxelSurfaceStyle] {
        graphics::MeshPointer mesh;

        entity->withReadLock([&] {
            std::shared_ptr<VoxelVolume> volData = entity->_volData;
            std::unique_ptr<SurfaceExtractor> extractor;
            switch (voxelSurfaceStyle) {
                case PolyVoxEntityItem::SURFACE_EDGED_MARCHING_CUBES:
                case PolyVoxEntityItem::SURFACE_MARCHING_CUBES: {
                    extractor.reset(new MarchingCubesSurfaceExtractor(volData));
                    break;
                }
                case PolyVoxEntityItem::SURFACE_EDGED_CUBIC:
                case PolyVoxEntityItem::SURFACE_CUBIC: {
                    extractor.reset(new CubicSurfaceExtractorWithNormals(volData));
                    break;
                }
            }
            mesh = extractor->createMesh();
        });

        entity->setMesh(mesh);
    });
}

void RenderablePolyVoxEntityItem::setMesh(graphics::MeshPointer mesh) {
    // this catches the payload from recomputeMesh
    withWriteLock([&] {
        if (!_collisionless) {
            _flags |= Simulation::DIRTY_SHAPE | Simulation::DIRTY_MASS;
        }
        _shapeReady = false;
        _mesh = mesh;
        if (_state == PolyVoxState::BakingMeshNoCompress) {
            _state = PolyVoxState::BakingMeshNoCompressFinished;
        } else {
            _state = PolyVoxState::BakingMeshFinished;
        }
        _meshReady = true;
        startUpdates();
    });

    somethingChangedNotification();
}

void RenderablePolyVoxEntityItem::computeShapeInfoWorker() {
    // this creates a collision-shape for the physics engine.  The shape comes from
    // _volData for cubic extractors and from _mesh for marching-cube extractors

    EntityItemPointer entity = getThisPointer();

    PolyVoxSurfaceStyle voxelSurfaceStyle;
    glm::vec3 voxelVolumeSize;
    graphics::MeshPointer mesh;

    withReadLock([&] {
        voxelSurfaceStyle = (PolyVoxSurfaceStyle)_voxelSurfaceStyle;
        voxelVolumeSize = _voxelVolumeSize;
        mesh = _mesh;
    });

    QtConcurrent::run([entity, voxelSurfaceStyle, voxelVolumeSize, mesh] {
        auto polyVoxEntity = std::static_pointer_cast<RenderablePolyVoxEntityItem>(entity);
        QVector<QVector<glm::vec3>> pointCollection;
        AABox box;
        glm::mat4 vtoM = std::static_pointer_cast<RenderablePolyVoxEntityItem>(entity)->voxelToLocalMatrix();

        if (voxelSurfaceStyle == PolyVoxEntityItem::SURFACE_MARCHING_CUBES ||
            voxelSurfaceStyle == PolyVoxEntityItem::SURFACE_EDGED_MARCHING_CUBES) {
            // pull each triangle in the mesh into a polyhedron which can be collided with
            unsigned int i = 0;

            const gpu::BufferView& vertexBufferView = mesh->getVertexBuffer();
            const gpu::BufferView& indexBufferView = mesh->getIndexBuffer();

            gpu::BufferView::Iterator<const uint32_t> it = indexBufferView.cbegin<uint32_t>();
            while (it != indexBufferView.cend<uint32_t>()) {
                uint32_t p0Index = *(it++);
                uint32_t p1Index = *(it++);
                uint32_t p2Index = *(it++);

                const glm::vec3& p0 = vertexBufferView.get<const glm::vec3>(p0Index);
                const glm::vec3& p1 = vertexBufferView.get<const glm::vec3>(p1Index);
                const glm::vec3& p2 = vertexBufferView.get<const glm::vec3>(p2Index);

                glm::vec3 av = (p0 + p1 + p2) / 3.0f;  // center of the triangular face
                glm::vec3 normal = glm::normalize(glm::cross(p1 - p0, p2 - p0));
                glm::vec3 p3 = av - normal * MARCHING_CUBE_COLLISION_HULL_OFFSET;

                glm::vec3 p0Model = glm::vec3(vtoM * glm::vec4(p0, 1.0f));
                glm::vec3 p1Model = glm::vec3(vtoM * glm::vec4(p1, 1.0f));
                glm::vec3 p2Model = glm::vec3(vtoM * glm::vec4(p2, 1.0f));
                glm::vec3 p3Model = glm::vec3(vtoM * glm::vec4(p3, 1.0f));

                box += p0Model;
                box += p1Model;
                box += p2Model;
                box += p3Model;

                QVector<glm::vec3> pointsInPart;
                pointsInPart << p0Model;
                pointsInPart << p1Model;
                pointsInPart << p2Model;
                pointsInPart << p3Model;
                // add next convex hull
                QVector<glm::vec3> newMeshPoints;
                pointCollection << newMeshPoints;
                // add points to the new convex hull
                pointCollection[i++] << pointsInPart;
            }
        } else {
            unsigned int i = 0;
            polyVoxEntity->forEachVoxelValue(voxelVolumeSize, [&](const ivec3& v, uint8_t value) {
                if (value > 0) {
                    const auto& x = v.x;
                    const auto& y = v.y;
                    const auto& z = v.z;
                    if (glm::all(glm::greaterThan(v, ivec3(0))) && glm::all(glm::lessThan(v, ivec3(voxelVolumeSize) - 1)) &&
                        (polyVoxEntity->getVoxelInternal({ x - 1, y, z }) > 0) &&
                        (polyVoxEntity->getVoxelInternal({ x, y - 1, z }) > 0) &&
                        (polyVoxEntity->getVoxelInternal({ x, y, z - 1 }) > 0) &&
                        (polyVoxEntity->getVoxelInternal({ x + 1, y, z }) > 0) &&
                        (polyVoxEntity->getVoxelInternal({ x, y + 1, z }) > 0) &&
                        (polyVoxEntity->getVoxelInternal({ x, y, z + 1 }) > 0)) {
                        // this voxel has neighbors in every cardinal direction, so there's no need
                        // to include it in the collision hull.
                        return;
                    }

                    QVector<glm::vec3> pointsInPart;

                    float offL = -0.5f;
                    float offH = 0.5f;
                    if (voxelSurfaceStyle == PolyVoxEntityItem::SURFACE_EDGED_CUBIC) {
                        offL += 1.0f;
                        offH += 1.0f;
                    }

                    glm::vec3 p000 = glm::vec3(vtoM * glm::vec4(x + offL, y + offL, z + offL, 1.0f));
                    glm::vec3 p001 = glm::vec3(vtoM * glm::vec4(x + offL, y + offL, z + offH, 1.0f));
                    glm::vec3 p010 = glm::vec3(vtoM * glm::vec4(x + offL, y + offH, z + offL, 1.0f));
                    glm::vec3 p011 = glm::vec3(vtoM * glm::vec4(x + offL, y + offH, z + offH, 1.0f));
                    glm::vec3 p100 = glm::vec3(vtoM * glm::vec4(x + offH, y + offL, z + offL, 1.0f));
                    glm::vec3 p101 = glm::vec3(vtoM * glm::vec4(x + offH, y + offL, z + offH, 1.0f));
                    glm::vec3 p110 = glm::vec3(vtoM * glm::vec4(x + offH, y + offH, z + offL, 1.0f));
                    glm::vec3 p111 = glm::vec3(vtoM * glm::vec4(x + offH, y + offH, z + offH, 1.0f));

                    box += p000;
                    box += p001;
                    box += p010;
                    box += p011;
                    box += p100;
                    box += p101;
                    box += p110;
                    box += p111;

                    pointsInPart << p000;
                    pointsInPart << p001;
                    pointsInPart << p010;
                    pointsInPart << p011;
                    pointsInPart << p100;
                    pointsInPart << p101;
                    pointsInPart << p110;
                    pointsInPart << p111;

                    // add next convex hull
                    QVector<glm::vec3> newMeshPoints;
                    pointCollection << newMeshPoints;
                    // add points to the new convex hull
                    pointCollection[i++] << pointsInPart;
                }
            });
        }
        polyVoxEntity->setCollisionPoints(pointCollection, box);
    });
}

void RenderablePolyVoxEntityItem::setCollisionPoints(ShapeInfo::PointCollection pointCollection, AABox box) {
    // this catches the payload from computeShapeInfoWorker
    if (pointCollection.isEmpty()) {
        EntityItem::computeShapeInfo(_shapeInfo);
        withWriteLock([&] {
            _shapeReady = true;
            _state = PolyVoxState::BakingShapeFinished;
        });
        return;
    }

    glm::vec3 collisionModelDimensions = box.getDimensions();
    // include the registrationPoint in the shape key, because the offset is already
    // included in the points and the shapeManager wont know that the shape has changed.
    withWriteLock([&] {
        QString shapeKey = QString(_voxelData.toBase64()) + "," + QString::number(_registrationPoint.x) + "," +
                           QString::number(_registrationPoint.y) + "," + QString::number(_registrationPoint.z);
        _shapeInfo.setParams(SHAPE_TYPE_COMPOUND, collisionModelDimensions, shapeKey);
        _shapeInfo.setPointCollection(pointCollection);
        _shapeReady = true;
        _state = PolyVoxState::BakingShapeFinished;
    });
}

void RenderablePolyVoxEntityItem::setXNNeighborID(const EntityItemID& xNNeighborID) {
    if (xNNeighborID == _id) {  // TODO loops are still possible
        return;
    }

    if (xNNeighborID != _xNNeighborID) {
        PolyVoxEntityItem::setXNNeighborID(xNNeighborID);
        _neighborXNeedsUpdate = true;
        startUpdates();
    }
}

void RenderablePolyVoxEntityItem::setYNNeighborID(const EntityItemID& yNNeighborID) {
    if (yNNeighborID == _id) {  // TODO loops are still possible
        return;
    }

    if (yNNeighborID != _yNNeighborID) {
        PolyVoxEntityItem::setYNNeighborID(yNNeighborID);
        _neighborYNeedsUpdate = true;
        startUpdates();
    }
}

void RenderablePolyVoxEntityItem::setZNNeighborID(const EntityItemID& zNNeighborID) {
    if (zNNeighborID == _id) {  // TODO loops are still possible
        return;
    }

    if (zNNeighborID != _zNNeighborID) {
        PolyVoxEntityItem::setZNNeighborID(zNNeighborID);
        _neighborZNeedsUpdate = true;
        startUpdates();
    }
}

void RenderablePolyVoxEntityItem::setXPNeighborID(const EntityItemID& xPNeighborID) {
    if (xPNeighborID == _id) {  // TODO loops are still possible
        return;
    }
    if (xPNeighborID != _xPNeighborID) {
        PolyVoxEntityItem::setXPNeighborID(xPNeighborID);
        _updateFromNeighborXEdge = true;
        startUpdates();
    }
}

void RenderablePolyVoxEntityItem::setYPNeighborID(const EntityItemID& yPNeighborID) {
    if (yPNeighborID == _id) {  // TODO loops are still possible
        return;
    }
    if (yPNeighborID != _yPNeighborID) {
        PolyVoxEntityItem::setYPNeighborID(yPNeighborID);
        _updateFromNeighborYEdge = true;
        startUpdates();
    }
}

void RenderablePolyVoxEntityItem::setZPNeighborID(const EntityItemID& zPNeighborID) {
    if (zPNeighborID == _id) {  // TODO loops are still possible
        return;
    }
    if (zPNeighborID != _zPNeighborID) {
        PolyVoxEntityItem::setZPNeighborID(zPNeighborID);
        _updateFromNeighborZEdge = true;
        startUpdates();
    }
}

std::shared_ptr<RenderablePolyVoxEntityItem> RenderablePolyVoxEntityItem::getXNNeighbor() {
    return std::dynamic_pointer_cast<RenderablePolyVoxEntityItem>(_xNNeighbor.lock());
}

std::shared_ptr<RenderablePolyVoxEntityItem> RenderablePolyVoxEntityItem::getYNNeighbor() {
    return std::dynamic_pointer_cast<RenderablePolyVoxEntityItem>(_yNNeighbor.lock());
}

std::shared_ptr<RenderablePolyVoxEntityItem> RenderablePolyVoxEntityItem::getZNNeighbor() {
    return std::dynamic_pointer_cast<RenderablePolyVoxEntityItem>(_zNNeighbor.lock());
}

std::shared_ptr<RenderablePolyVoxEntityItem> RenderablePolyVoxEntityItem::getXPNeighbor() {
    return std::dynamic_pointer_cast<RenderablePolyVoxEntityItem>(_xPNeighbor.lock());
}

std::shared_ptr<RenderablePolyVoxEntityItem> RenderablePolyVoxEntityItem::getYPNeighbor() {
    return std::dynamic_pointer_cast<RenderablePolyVoxEntityItem>(_yPNeighbor.lock());
}

std::shared_ptr<RenderablePolyVoxEntityItem> RenderablePolyVoxEntityItem::getZPNeighbor() {
    return std::dynamic_pointer_cast<RenderablePolyVoxEntityItem>(_zPNeighbor.lock());
}

// deprecated
bool RenderablePolyVoxEntityItem::getMeshes(MeshProxyList& result) {
    bool success = false;
    if (_mesh) {
        MeshProxy* meshProxy = nullptr;
        glm::mat4 transform = voxelToLocalMatrix();
        withReadLock([&] {
            gpu::BufferView::Index numVertices = (gpu::BufferView::Index)_mesh->getNumVertices();
            if (!_meshReady) {
                // we aren't ready to return a mesh.  the caller will have to try again later.
                success = false;
            } else if (numVertices == 0) {
                // we are ready, but there are no triangles in the mesh.
                success = true;
            } else {
                success = true;
                // the mesh will be in voxel-space.  transform it into object-space
                meshProxy = new SimpleMeshProxy(
                    _mesh->map([=](glm::vec3 position) { return glm::vec3(transform * glm::vec4(position, 1.0f)); },
                               [=](glm::vec3 color) { return color; },
                               [=](glm::vec3 normal) { return glm::normalize(glm::vec3(transform * glm::vec4(normal, 0.0f))); },
                               [&](uint32_t index) { return index; }));
                result << meshProxy;
            }
        });
    }
    return success;
}

scriptable::ScriptableModelBase RenderablePolyVoxEntityItem::getScriptableModel() {
    if (!_mesh) {
        return scriptable::ScriptableModelBase();
    }

    bool success = false;
    glm::mat4 transform = voxelToLocalMatrix();
    scriptable::ScriptableModelBase result;
    result.objectID = getThisPointer()->getID();
    withReadLock([&] {
        gpu::BufferView::Index numVertices = (gpu::BufferView::Index)_mesh->getNumVertices();
        if (!_meshReady) {
            // we aren't ready to return a mesh.  the caller will have to try again later.
            success = false;
        } else if (numVertices == 0) {
            // we are ready, but there are no triangles in the mesh.
            success = true;
        } else {
            success = true;
            // the mesh will be in voxel-space.  transform it into object-space
            result.append(
                _mesh->map([=](glm::vec3 position) { return glm::vec3(transform * glm::vec4(position, 1.0f)); },
                           [=](glm::vec3 color) { return color; },
                           [=](glm::vec3 normal) { return glm::normalize(glm::vec3(transform * glm::vec4(normal, 0.0f))); },
                           [&](uint32_t index) { return index; }));
        }
    });
    return result;
}

using namespace render;
using namespace render::entities;

static uint8_t CUSTOM_PIPELINE_NUMBER;
static const gpu::Element COLOR_ELEMENT{ gpu::VEC4, gpu::NUINT8, gpu::RGBA };
// forward, shadow, fade, wireframe
static std::map<std::tuple<bool, bool, bool, bool>, ShapePipelinePointer> _pipelines;
static gpu::Stream::FormatPointer _vertexFormat;
static gpu::Stream::FormatPointer _vertexColorFormat;

static ShapePipelinePointer polyvoxPipelineFactory(const ShapePlumber& plumber, const ShapeKey& key, RenderArgs* args) {
    if (_pipelines.empty()) {
        using namespace shader::entities_renderer::program;

        // forward, shadow, fade
        static const std::vector<std::tuple<bool, bool, bool, uint32_t>> keys = {
            std::make_tuple(false, false, false, polyvox),
            std::make_tuple(true, false, false, polyvox_forward),
            std::make_tuple(false, true, false, polyvox_shadow),
            // no such thing as forward + shadow
            std::make_tuple(false, false, true, polyvox_fade),
            std::make_tuple(false, true, true, polyvox_shadow_fade),
            // no such thing as forward + fade/shadow
        };
        for (auto& key : keys) {
            for (int i = 0; i < 2; ++i) {
                bool wireframe = i != 0;

                auto state = std::make_shared<gpu::State>();
                state->setCullMode(gpu::State::CULL_BACK);
                state->setDepthTest(true, true, ComparisonFunction::LESS_EQUAL);
                PrepareStencil::testMaskDrawShape(*state);

                if (wireframe) {
                    state->setFillMode(gpu::State::FILL_LINE);
                }

                auto pipeline = gpu::Pipeline::create(gpu::Shader::createProgram(std::get<3>(key)), state);
                if (!std::get<2>(key)) {
                    _pipelines[std::make_tuple(std::get<0>(key), std::get<1>(key), std::get<2>(key), wireframe)] =
                        std::make_shared<render::ShapePipeline>(pipeline, nullptr, nullptr, nullptr);
                } else {
                    _pipelines[std::make_tuple(std::get<0>(key), std::get<1>(key), std::get<2>(key), wireframe)] =
                        std::make_shared<render::ShapePipeline>(pipeline, nullptr, FadeEffect::getBatchSetter(),
                                                                FadeEffect::getItemUniformSetter());
                }
            }
        }
    }

    bool isFaded = key.isFaded() && args->_renderMethod != Args::RenderMethod::FORWARD;
    return _pipelines[std::make_tuple(args->_renderMethod == Args::RenderMethod::FORWARD,
                                      args->_renderMode == Args::RenderMode::SHADOW_RENDER_MODE, isFaded, key.isWireframe())];
}

PolyVoxEntityRenderer::PolyVoxEntityRenderer(const EntityItemPointer& entity) : Parent(entity) {
    static std::once_flag once;
    std::call_once(once, [&] {
        CUSTOM_PIPELINE_NUMBER = render::ShapePipeline::registerCustomShapePipelineFactory(polyvoxPipelineFactory);
        _vertexFormat = std::make_shared<gpu::Stream::Format>();
        _vertexFormat->setAttribute(gpu::Stream::POSITION, 0, gpu::Element(gpu::VEC3, gpu::FLOAT, gpu::XYZ), 0);
        _vertexFormat->setAttribute(gpu::Stream::NORMAL, 0, gpu::Element(gpu::VEC3, gpu::FLOAT, gpu::XYZ), 12);

        _vertexColorFormat = std::make_shared<gpu::Stream::Format>();
        _vertexColorFormat->setAttribute(gpu::Stream::POSITION, 0, gpu::Element(gpu::VEC3, gpu::FLOAT, gpu::XYZ), 0);
        _vertexColorFormat->setAttribute(gpu::Stream::NORMAL, 0, gpu::Element(gpu::VEC3, gpu::FLOAT, gpu::XYZ), 12);
        _vertexColorFormat->setAttribute(gpu::Stream::COLOR, gpu::Stream::COLOR, COLOR_ELEMENT, 0, gpu::Stream::PER_INSTANCE);
    });
    _params = std::make_shared<gpu::Buffer>(gpu::Buffer::UniformBuffer, sizeof(glm::vec4), nullptr);
}

ShapeKey PolyVoxEntityRenderer::getShapeKey() {
    bool hasMaterials = false;
    graphics::MultiMaterial materials;
    {
        std::lock_guard<std::mutex> lock(_materialsLock);
        auto materialsItr = _materials.find("0");
        hasMaterials = (materialsItr != _materials.end());
        if (hasMaterials) {
            materials = materialsItr->second;
        }
    }
    ShapeKey::Builder builder;
    if (!hasMaterials) {
        builder = ShapeKey::Builder().withCustom(CUSTOM_PIPELINE_NUMBER);
        if (_primitiveMode == PrimitiveMode::LINES) {
            builder.withWireframe();
        }
    } else {
        updateShapeKeyBuilderFromMaterials(builder);
        Pipeline pipelineType = getPipelineType(materials);
        if (pipelineType == Pipeline::MATERIAL) {
            builder.withTriplanar();
        }
    }
    return builder.build();
}

bool PolyVoxEntityRenderer::needsRenderUpdate() const {
    return needsRenderUpdateFromMaterials() || Parent::needsRenderUpdate();
}

bool PolyVoxEntityRenderer::needsRenderUpdateFromTypedEntity(const TypedEntityPointer& entity) const {
    if (resultWithReadLock<bool>([&] {
            if (entity->voxelToLocalMatrix() != _lastVoxelToLocalMatrix) {
                return true;
            }

            if (entity->_mesh != _mesh) {
                return true;
            }

            return false;
        })) {
        return true;
    }

    return Parent::needsRenderUpdateFromTypedEntity(entity);
}

void PolyVoxEntityRenderer::doRenderUpdateAsynchronousTyped(const TypedEntityPointer& entity) {
    _lastVoxelToLocalMatrix = entity->voxelToLocalMatrix();
    bool success;
    _position = entity->getCenterPosition(success);
    _orientation =
        entity->getBillboardMode() == BillboardMode::NONE ? entity->getWorldOrientation() : entity->getLocalOrientation();
    _lastVoxelVolumeSize = entity->getVoxelVolumeSize();
    _params->setSubData(0, vec4(_lastVoxelVolumeSize, 0.0));
    graphics::MeshPointer newMesh;
    entity->withReadLock([&] { newMesh = entity->_mesh; });

    if (newMesh && newMesh->getIndexBuffer()._buffer) {
        _mesh = newMesh;
    }

    std::array<QString, 3> xyzTextureURLs{ { entity->getXTextureURL(), entity->getYTextureURL(), entity->getZTextureURL() } };
    for (size_t i = 0; i < xyzTextureURLs.size(); ++i) {
        auto& texture = _xyzTextures[i];
        const auto& textureURL = xyzTextureURLs[i];
        if (textureURL.isEmpty()) {
            texture.reset();
        } else if (!texture || texture->getURL() != QUrl(textureURL)) {
            texture = DependencyManager::get<TextureCache>()->getTexture(textureURL);
        }
    }

    updateMaterials();
}

bool PolyVoxEntityRenderer::isTransparent() const {
    // TODO: We don't currently support transparent PolyVox (unless they are using the material path)
    return /*Parent::isTransparent() || */ materialsTransparent();
}

Item::Bound PolyVoxEntityRenderer::getBound(RenderArgs* args) {
    return Parent::getMaterialBound(args);
}

void PolyVoxEntityRenderer::doRender(RenderArgs* args) {
    if (!_mesh || !_mesh->getIndexBuffer()._buffer) {
        return;
    }

    PerformanceTimer perfTimer("RenderablePolyVoxEntityItem::render");
    gpu::Batch& batch = *args->_batch;

    bool usePrimaryFrustum = args->_renderMode == RenderArgs::RenderMode::SHADOW_RENDER_MODE || args->_mirrorDepth > 0;
    glm::mat4 rotation = glm::mat4_cast(
        BillboardModeHelpers::getBillboardRotation(_position, _orientation, _billboardMode,
                                                   usePrimaryFrustum ? BillboardModeHelpers::getPrimaryViewFrustumPosition()
                                                                     : args->getViewFrustum().getPosition()));
    Transform transform(glm::translate(_position) * rotation * _lastVoxelToLocalMatrix);
    batch.setModelTransform(transform, _prevRenderTransform);
    if (args->_renderMode == Args::RenderMode::DEFAULT_RENDER_MODE ||
        args->_renderMode == Args::RenderMode::MIRROR_RENDER_MODE) {
        _prevRenderTransform = transform;
    }

    batch.setInputBuffer(gpu::Stream::POSITION, _mesh->getVertexBuffer()._buffer, 0, sizeof(PositionNormalMaterial));
    batch.setIndexBuffer(gpu::UINT32, _mesh->getIndexBuffer()._buffer, 0);

    bool hasMaterials = false;
    graphics::MultiMaterial materials;
    {
        std::lock_guard<std::mutex> lock(_materialsLock);
        auto materialsItr = _materials.find("0");
        hasMaterials = (materialsItr != _materials.end());
        if (hasMaterials) {
            materials = materialsItr->second;
        }
    }
    if (!hasMaterials) {
        for (size_t i = 0; i < _xyzTextures.size(); ++i) {
            const auto& texture = _xyzTextures[i];
            if (texture) {
                batch.setResourceTexture((uint32_t)i, texture->getGPUTexture());
            } else {
                batch.setResourceTexture((uint32_t)i, DependencyManager::get<TextureCache>()->getWhiteTexture());
            }
        }
        batch.setInputFormat(_vertexFormat);
        batch.setUniformBuffer(0, _params);
    } else {
        if (materials.isInvisible()) {
            return;
        }

        glm::vec4 outColor = glm::vec4(1.0f);  // albedo comes from the material instead of vertex colors

        Pipeline pipelineType = getPipelineType(materials);
        if (pipelineType == Pipeline::PROCEDURAL) {
            auto procedural = std::static_pointer_cast<graphics::ProceduralMaterial>(materials.top().material);
            outColor = procedural->getColor(outColor);
            withReadLock([&] {
                procedural->prepare(batch, transform.getTranslation(), transform.getScale(), transform.getRotation(), _created,
                                    ProceduralProgramKey(outColor.a < 1.0f));
            });
        } else if (pipelineType == Pipeline::MATERIAL) {
            if (RenderPipelines::bindMaterials(materials, batch, args->_renderMode, args->_enableTexturing)) {
                args->_details._materialSwitches++;
            }
        }

        const uint32_t compactColor = GeometryCache::toCompactColor(glm::vec4(outColor));
        _colorBuffer->setData(sizeof(compactColor), (const gpu::Byte*)&compactColor);
        gpu::BufferView colorView(_colorBuffer, COLOR_ELEMENT);
        batch.setInputBuffer(gpu::Stream::COLOR, colorView);
        batch.setInputFormat(_vertexColorFormat);
        batch.setUniformBuffer(graphics::slot::buffer::TriplanarScale, _params);
    }

    batch.drawIndexed(gpu::TRIANGLES, (gpu::uint32)_mesh->getNumIndices(), 0);
}

QDebug operator<<(QDebug debug, PolyVoxState state) {
    switch (state) {
        case PolyVoxState::Ready:
            debug << "Ready";
            break;
        case PolyVoxState::Uncompressing:
            debug << "Uncompressing";
            break;
        case PolyVoxState::UncompressingFinished:
            debug << "UncompressingFinished";
            break;
        case PolyVoxState::BakingMesh:
            debug << "BakingMesh";
            break;
        case PolyVoxState::BakingMeshFinished:
            debug << "BakingMeshFinished";
            break;
        case PolyVoxState::BakingMeshNoCompress:
            debug << "BakingMeshNoCompress";
            break;
        case PolyVoxState::BakingMeshNoCompressFinished:
            debug << "BakingMeshNoCompressFinished";
            break;
        case PolyVoxState::Compressing:
            debug << "Compressing";
            break;
        case PolyVoxState::CompressingFinished:
            debug << "CompressingFinished";
            break;
        case PolyVoxState::BakingShape:
            debug << "BakingShape";
            break;
        case PolyVoxState::BakingShapeFinished:
            debug << "BakingShapeFinished";
            break;
    }
    return debug;
}

scriptable::ScriptableModelBase PolyVoxEntityRenderer::getScriptableModel() {
    scriptable::ScriptableModelBase result = asTypedEntity<RenderablePolyVoxEntityItem>()->getScriptableModel();

    {
        std::lock_guard<std::mutex> lock(_materialsLock);
        result.appendMaterials(_materials);
    }

    return result;
}
