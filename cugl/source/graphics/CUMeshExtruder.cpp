//
//  CUMeshExtruder.cpp
//  Cornell University Game Library (CUGL)
//
//  This module is an extension of SimpleExtruder that can provide OpenGL
//  specific data. Originally, this functionality was part of SimpleExtruder.
//  However, when we separated graphics from the core package, we needed to
//  separate this part as well.
//
//  Since math objects are intended to be on the stack, we do not provide
//  any shared pointer support in this class.
//
//  CUGL MIT License:
//      This software is provided 'as-is', without any express or implied
//      warranty. In no event will the authors be held liable for any damages
//      arising from the use of this software.
//
//      Permission is granted to anyone to use this software for any purpose,
//      including commercial applications, and to alter it and redistribute it
//      freely, subject to the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not
//      be misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
//
//  Author: Walker White
//  Version: 7/3/24 (CUGL 3.0 reorganization)
//
#include <cugl/graphics/CUMeshExtruder.h>
#include <cugl/graphics/CUMesh.h>
#include <cugl/graphics/CUSpriteVertex.h>
#include <iterator>
#include <cmath>

using namespace cugl;
using namespace cugl::graphics;

#pragma mark -
#pragma mark Constructors
/**
 * Creates an extruder with the given path.
 *
 * The path data is copied. The extruder does not retain any references
 * to the original data.
 *
 * @param points    The path to extrude
 * @param closed    Whether the path is closed
 */
MeshExtruder::MeshExtruder(const std::vector<Vec2>& points, bool closed) :
    SimpleExtruder(points, closed) {}

/**
 * Creates an extruder with the given path.
 *
 * The path data is copied. The extruder does not retain any references
 * to the original data.
 *
 * @param path        The path to extrude
 */
MeshExtruder::MeshExtruder(const Path2& path) : SimpleExtruder(path) {}


#pragma mark -
#pragma mark Materialization
/**
 * Returns a mesh representing the path extrusion.
 *
 * This method creates a triangular mesh with the vertices of the extrusion,
 * coloring each vertex white. However, if fringe is set to true, then each
 * vertex will instead be colored clear (transparent), unless that vertex is
 * on a zero-width side. This effect can be used to produce border "fringes"
 * around a polygon for anti-aliasing.
 *
 * If the calculation is not yet performed, this method will return the
 * empty mesh.
 *
 * @param color     The default mesh color
 *
 * @return a mesh representing the path extrusion.
 */
Mesh<SpriteVertex> MeshExtruder::getMesh(Color4 color) const {
    Mesh<SpriteVertex> mesh;
    getMesh(&mesh,color);
    return mesh;
}

/**
 * Stores a mesh representing the path extrusion in the given buffer
 *
 * This method will add both the new vertices, and the corresponding indices
 * to the buffer. If the buffer is not empty, the indices will be adjusted
 * accordingly. You should clear the buffer first if you do not want to
 * preserve the original data.
 *
 * The vertices in this mesh will be colored white by default. However, if
 * fringe is set to true, then each vertex will instead be colored clear
 * (transparent), unless that vertex is on a zero-width side. This effect
 * can be used to produce border "fringes" around a polygon for anti-aliasing.
 *
 * If the calculation is not yet performed, this method will do nothing.
 *
 * @param mesh      The buffer to store the extruded path mesh
 * @param color     The default mesh color
 *
 * @return a reference to the buffer for chaining.
 */
Mesh<SpriteVertex>* MeshExtruder::getMesh(Mesh<SpriteVertex>* mesh, Color4 color) const {
    CUAssertLog(mesh, "Destination buffer is null");
    CUAssertLog(mesh->command == GL_TRIANGLES,
               "Buffer geometry is incompatible with this result.");
    if (_calculated) {
        Vec2* vts = reinterpret_cast<Vec2*>(_verts);
        Uint32 offset = (Uint32)mesh->vertices.size();
        mesh->vertices.reserve(_vsize+offset);
        GLuint clr = color.getPacked();
        for(size_t ii = 0; ii < _vsize; ii++) {
            mesh->vertices.emplace_back();
            SpriteVertex* vertex = &(mesh->vertices.back());
            vertex->position = vts[ii];
            vertex->color = clr;
        }
        mesh->indices.reserve( _isize+mesh->indices.size());
        for(size_t ii = 0; ii < _isize; ii++) {
            mesh->indices.push_back(_indxs[ii]+offset);
        }
    }
    return mesh;
}

/**
 * Returns a mesh representing the path extrusion.
 *
 * This method creates a triangular mesh with the vertices of the extrusion,
 * coloring each vertex white. However, if fringe is set to true, then each
 * vertex will instead be colored clear (transparent), unless that vertex is
 * on a zero-width side. This effect can be used to produce border "fringes"
 * around a polygon for anti-aliasing.
 *
 * If the calculation is not yet performed, this method will return the
 * empty mesh.
 *
 * @param inner     The interior mesh color
 * @param outer     The exterior mesh color
 *
 * @return a mesh representing the path extrusion.
 */
Mesh<SpriteVertex> MeshExtruder::getMesh(Color4 inner, Color4 outer) const {
    Mesh<SpriteVertex> mesh;
    getMesh(&mesh,inner,outer);
    return mesh;
}

/**
 * Stores a mesh representing the path extrusion in the given buffer
 *
 * This method will add both the new vertices, and the corresponding indices
 * to the buffer. If the buffer is not empty, the indices will be adjusted
 * accordingly. You should clear the buffer first if you do not want to
 * preserve the original data.
 *
 * The vertices in this mesh will be colored white by default. However, if
 * fringe is set to true, then each vertex will instead be colored clear
 * (transparent), unless that vertex is on a zero-width side. This effect
 * can be used to produce border "fringes" around a polygon for anti-aliasing.
 *
 * If the calculation is not yet performed, this method will do nothing.
 *
 * @param mesh      The buffer to store the extruded path mesh
 * @param inner     The interior mesh color
 * @param outer     The exterior mesh color
 *
 * @return a reference to the buffer for chaining.
 */
Mesh<SpriteVertex>* MeshExtruder::getMesh(Mesh<SpriteVertex>* mesh,
                                          Color4 inner, Color4 outer) const {
    CUAssertLog(mesh, "Destination buffer is null");
    CUAssertLog(mesh->command == GL_TRIANGLES,
               "Buffer geometry is incompatible with this result.");
    if (_calculated) {
        Vec2* vts = reinterpret_cast<Vec2*>(_verts);
        Uint32 offset = (Uint32)mesh->vertices.size();
        mesh->vertices.reserve(_vsize+offset);

        GLuint icolor = inner.getPacked();
        GLuint ocolor = outer.getPacked();
        for(size_t ii = 0; ii < _vsize; ii++) {
            mesh->vertices.emplace_back();
            SpriteVertex* vertex = &(mesh->vertices.back());
            vertex->position = vts[ii];
            if (!_sides[2*ii]) {
                vertex->color = icolor;
            } else {
                vertex->color = ocolor;
            }
        }
        mesh->indices.reserve( _isize+mesh->indices.size());
        for(size_t ii = 0; ii < _isize; ii++) {
            mesh->indices.push_back(_indxs[ii]+offset);
        }
    }
    return mesh;
}
