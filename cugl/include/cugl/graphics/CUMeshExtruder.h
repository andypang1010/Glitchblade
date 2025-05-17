//
//  CUMeshExtruder.h
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
#ifndef __CU_MESH_EXTRUDER_H__
#define __CU_MESH_EXTRUDER_H__

#include <cugl/core/math/polygon/CUSimpleExtruder.h>

namespace cugl {

    /**
     * The classes and functions needed to construct a graphics pipeline.
     *
     * Initially these were part of the core CUGL module (everyone wants graphics,
     * right). However, after student demand for a headless option that did not
     * have so many OpenGL dependencies, this was factored out.
     */
    namespace graphics {

/** Forward class declaration */
class SpriteVertex;

// It is generally a bad idea to include a template in our header
template<typename T>
class Mesh;

/**
 * This class is a factory for extruding paths into a sprite mesh.
 *
 * The class {@link SimpleExtruder} actually stores enough information to aid
 * with texturing. However, texture information is not part of {@link Poly2}.
 * If we want texture information, we need instead a mesh of {@link SpriteVertex}
 * values. That is the purpose of this extruder.
 *
 * This functionality was originally part of the base class {@link SimpleExtruder}.
 * However, upon our decision to factor out graphics as its own package, we
 * needed to separate the behavior.
 */
class MeshExtruder : public SimpleExtruder {
    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates an extruder with no vertex data.
     */
    MeshExtruder() : SimpleExtruder() {}
    
    /**
     * Creates an extruder with the given path.
     *
     * The path data is copied. The extruder does not retain any references
     * to the original data.
     *
     * @param points    The path to extrude
     * @param closed    Whether the path is closed
     */
    MeshExtruder(const std::vector<Vec2>& points, bool closed);
    
    /**
     * Creates an extruder with the given path.
     *
     * The path data is copied. The extruder does not retain any references
     * to the original data.
     *
     * @param path        The path to extrude
     */
    MeshExtruder(const Path2& path);
    
    /**
     * Deletes this extruder, releasing all resources.
     */
    ~MeshExtruder() {}
    
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
    Mesh<SpriteVertex> getMesh(Color4 color) const;
    
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
    Mesh<SpriteVertex>* getMesh(Mesh<SpriteVertex>* mesh, Color4 color) const;
    
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
    Mesh<SpriteVertex> getMesh(Color4 inner, Color4 outer) const;
    
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
    Mesh<SpriteVertex>* getMesh(Mesh<SpriteVertex>* mesh,
                                Color4 inner, Color4 outer) const;
};

    }
}

#endif /* __CU_SIMPLE_EXTRUDER_H__ */
