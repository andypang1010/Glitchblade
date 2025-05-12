//
//  CUParticleSystem.h
//  Cornell University Game Library (CUGL)
//
//  This module an implementation of a simple particle system. It is adapted
//  from the OpenGL tutorial series:
//
//     http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/particles-instancing/
//
//  This module provides a simplified interface that supports sprite animation
//  as well as particle coloring. While the API for a particle system is 3d,
//  this can easily be integrated into 2d scene graphs simply by setting the
//  z-value to 0.
//
//  The particle system is designed to allow some flexibility with user-provided
//  simulations. Ideally, we would have templated the particle system to allow
//  a user-defined particle class. However, templating has issues with
//  proliferation (e.g. forcing any general purpose class that uses a particle
//  system to be templated as well). The alternative is subclassing to limit the
//  template to the method and not the class. However this causes issues with
//  allocation and heap fragmentation.
//
//  The solution that we use in this module is delegating. There is a base
//  particle class which takes a void pointer that can store additional user
//  data. In addition, the user provides function pointers to the particle
//  system to define initialization and simulation of individual particles.
//
//  This module is a mixture of structs (public attribute classes designed to
//  be used on the stack) and classes that use our standard shared-pointer
//  architecture.
//
//  1. The constructor does not perform any initialization; it just sets all
//     attributes to their defaults.
//
//  2. All initialization takes place via init methods, which can fail if an
//     object is initialized more than once.
//
//  3. All allocation takes place via static constructors which return a shared
//     pointer.
//
//
//  CUGL MIT License:
//      This software is provided 'as-is', without any express or implied
//      warranty.  In no event will the authors be held liable for any damages
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
#ifndef __CU_PARTICLE_SYSTEM_H__
#define __CU_PARTICLE_SYSTEM_H__
#include <cugl/core/math/cu_math.h>
#include <cugl/core/assets/CUJsonValue.h>
#include <cugl/core/util/CURandom.h>
#include <cugl/graphics/CUMesh.h>

namespace cugl {

    /**
     * The classes and functions needed to construct a graphics pipeline.
     *
     * Initially these were part of the core CUGL module (everyone wants graphics,
     * right). However, after student demand for a headless option that did not
     * have so many OpenGL dependencies, this was factored out.
     */
    namespace graphics {

/** Forward class reference */
class InstanceBuffer;
class Shader;

#pragma mark Particle Buffer
/**
 * This class is a struct representing a particle vertex.
 *
 * Particles are typically a quad of four vertices, but this is not required.
 * In fact, a tighter fit to the sprite can aid with transparency issues.
 *
 * The texture coordinates may change per instance (in the case of animation).
 * That is achieved by adding a texture offset to the base texture coordinate.
 *
 * Particle Vertices are used by {@link InstanceBuffer} to render the particle
 * template. So we do not refer to them with shared pointers, or have fancy
 * constructor for them.
 */
class ParticleVertex {
public:
    /** The position of this vertex */
    Vec2 position;
    /** The texture coordinate of this vertex */
    Vec2 texCoord;
    
    /**
     * Creates a default particle vertex
     */
    ParticleVertex() {}
    
    /**
     * Creates a particle vertex with the given position and texture coord
     *
     * @param pos   The vertex position
     * @param coord The vertex texture coordinate
     */
    ParticleVertex(const Vec2 pos, const Vec2 coord) {
        position = pos;
        texCoord = coord;
    }
    
    /**
     * Creates a particle vertex with the given position and texture coord
     *
     * @param x     The x-coordinate of the vertex position
     * @param y     The y-coordinate of the vertex position
     * @param u     The u-coordinate of the texture coordinate
     * @param v     The v-coordinate of the texture coordinate
     */
    ParticleVertex(float x, float y, float u, float v) {
        position.set(x,y);
        texCoord.set(u,v);
    }
    
    /**
     * Creates a new ParticleVertex from the given JSON value.
     *
     * A particle vertex can be described as an array of floats or a JSON object.
     * If it is a JSON object, then it supports the following attributes.
     *
     *     "position":  An array of float arrays of length two
     *     "texcoord":  An array of float arrays of length two
     *
     * Again, all attributes are optional, as all values resolve to the origin.
     *
     * If the particle vertex is represented as an array, then it should be an
     * array of length no more than 4. These float are assigned to the
     * attributes position (2) and texCoord (2) in that order. Missing values
     * are replaced with a 0.
     *
     * @param json  The JSON object specifying the vertex
     */
    ParticleVertex(const std::shared_ptr<JsonValue>& json);
    
    /**
     * Sets this ParticleVertex to have the data in the given JSON value.
     *
     * A particle vertex can be described as an array of floats or a JSON object.
     * If it is a JSON object, then it supports the following attributes.
     *
     *     "position":  An array of float arrays of length two
     *     "texcoord":  An array of float arrays of length two
     *
     * Again, all attributes are optional, as all values resolve to the origin.
     *
     * If the particle vertex is represented as an array, then it should be an
     * array of length no more than 4. These float are assigned to the
     * attributes position (2) and texCoord (2) in that order. Missing values
     * are replaced with a 0.
     *
     * @param json  The JSON object specifying the vertex
     *
     * @return a reference to this particle vertex for chaining
     */
    ParticleVertex& set(const std::shared_ptr<JsonValue>& json);
};

/**
 * This class is a struct representing the data for a particle instance
 *
 * A particle system takes the template and multiplies using instancing. This
 * struct represents the data unique to each instance.
 *
 * Sprite animation is handled via the attribute texOffset. This value is added
 * to the texture coordinates of the template to produce the new texture
 * coordinates. This requires each texture component to have the same size. Set
 * this value to (0,0) to disable animation.
 *
 * Particle Instances are used by {@link InstanceBuffer} to render the particle
 * instances. So we do not refer to them with shared pointers, or have fancy
 * constructor for them.
 */
class ParticleInstance {
public:
    /** The position of the particle in 3d space; w is the radius */
    Vec4 position;
    /** The color of the particle */
    Color4 color;
    /** The texture offset of this particle (for animation) */
    Vec2 texOffset;
    /** The *squared* distance to the camera. If dead, this value is -1.0f */
    float distance;
    
    /**
     * Creates an empty particle instance
     */
    ParticleInstance() : distance(0) {}
};

#pragma mark -
#pragma mark Particle Emitter
/**
 * This class defines a single particle emitter.
 *
 * A particle system is collection of one or more emitters. Emitters define the
 * location, velocity, and time interval used to create particles. They also
 * all have their own random number generator to keep behavior reproducible.
 *
 * ParticleTemplates are internal to a {@link ParticleSystem}, so we do not
 * refer to them with shared pointers. We treat them like a math class that goes
 * on the stack.
 */
class ParticleEmitter {
public:
    /** The random number generator for this emitter */
    std::shared_ptr<Random> generator;
    /** The location of this emitter */
    Vec3 position;
    /** The initial velocity of particles emitted */
    Vec3 velocity;
    /** The lifespan of emitted particles */
    float lifespan;
    /** The amount of time (in seconds) between emissions */
    double interval;
    /** The amount of time (in seconds) that has passed for this emitter */
    double duration;
    /** The amount of time remaining until the next emission */
    double remainder;
    
    /**
     * Creates a degenerate particle emitter
     *
     * No particles will be emitted until the velocity, lifespan, and time
     * interval are set.
     */
    ParticleEmitter() :
    lifespan(0), interval(0), duration(0), remainder(0) {}
    
    /**
     * Creates a particle emitter with the given random generator seed.
     *
     * All other attributes will be zeroed out.
     *
     * @param seed  The random generator seed.
     */
    ParticleEmitter(Uint64 seed) :
    lifespan(0), interval(0), duration(0), remainder(0) {
        generator = Random::allocWithSeed(seed);
    }
    
    /**
     * Creates a particle emitter with the given JSON.
     *
     * This initializer is designed to create a particle emitter during asset
     * loading. This JSON format supports the following attribute values:
     *
     *     "random":    A long an or array of longs defining the random seed
     *     "position":  A float array of length 3 representing the position
     *     "velocity":  A float array of length 3 representing emission velocity
     *     "lifespan":  A float representing particle lifespan in seconds
     *     "internal":  A float representing the emission interval in seconds
     *
     * All of these values are optional. If "random" is missing, the clock will
     * be used for the seed. All other attributes will be zeroed out.
     *
     * @param data      The JSON object specifying the emitter
     */
    ParticleEmitter(const std::shared_ptr<JsonValue>& data);
};

#pragma mark -
#pragma mark Particle
/**
 * This class is the class representing a particle in a simulation.
 *
 * Ideally, we would have allowed users to define their own particle classes,
 * and templatized {@link ParticleSystem}. However, this creates a lot of
 * design issues. Templating has issues with proliferation (e.g. forcing any
 * general purpose class that uses a particle system to be templated as well).
 * The alternative is to use subclassing to limit the template to the method
 * and not the entire class. But this causes issues with allocation and heap
 * fragmentation (because the particle system would need to be a pointer of
 * pointers).
 *
 * The solution used by this class is delegation. This class is essentially a
 * struct with no methods of its own. It has several base attributes, but also
 * includes a void pointer for user-defined data.
 *
 * The user manipulates these particles with three function pointers passed to
 * {@link ParticleSystem}: {@link ParticleAllocator}, {@link ParticleDeallocator},
 * and {@link ParticleUpdater}. See the descriptions for these function types.
 * Both {@link ParticleAllocator} and {@link ParticleUpdater} are required.
 * {@link ParticleDeallocator} is only required if {@link ParticleAllocator}
 * modifies the `userdata` attribute of this class.
 */
class Particle3 {
public:
    /** The particle position */
    Vec3 position;
    /** The particle velocity */
    Vec3 velocity;
    /** The particle color */
    Color4 color;
    /** The particle size (scale to apply to template) */
    float size;
    /** The remaining life of the particle. If <0 this particle is dead and unused. */
    float life;
    /** The *squared* distance to the camera. If dead, this value is -1.0f */
    float distance;
    /** The delay in seconds until this particle is emitted */
    float delay;
    /**
     * Optional user-data for more complex simulations.
     *
     * This value can be allocated by {@link ParticleAllocator}. But if that
     * happens, the {@link ParticleDeallocator} should clean up afterwards to
     * prevent any memory leaks.
     */
    void* userdata;
    
    /**
     * Creates an default particle.
     *
     * This particle is uninitialized and unused. The particle system will
     * initialize it before use.
     */
    Particle3() : life(-1), distance(-1), delay(0), size(0), userdata(nullptr) {}
};

/**
 * @typedef ParticleAllocator
 *
 * This type represents a function to allocate a {@link Particle3} object.
 *
 * Particle systems recycle their particle objects, so allocation simply means
 * assigning attributes after they have been recycled. That is the purpose
 * of this function. It takes a {@link ParticleEmitter} object and uses that
 * to reassign the attributes of a {@link Particle3} object.
 *
 * This function is allowed to allocate and assign custom data to the userdata
 * attribute in a particle. However, if it does so, the user should also create
 * a {@link ParticleDeallocator} function to reclaim this memory as nessary.
 *
 * The allocator should set all attributes except for `distance` and `delay`.
 * Those attributes are managed by the {@link ParticleSystem} and any values
 * set here will be overwritten.
 *
 * This function type is equivalent to
 *
 *      std::function<void(const ParticleEmitter& source, Particle3* part)>
 *
 * @param source    The particle emitter
 * @param part      The particle to update
 */
typedef std::function<void(const ParticleEmitter& source, Particle3* part)> ParticleAllocator;

/**
 * @typedef ParticleDeallocator
 *
 * This type represents a function to deallocate a {@link Particle3} object.
 *
 * Particle systems recycle their particle objects, and we need to be clean up
 * any allocated memory when they are recycled. That is the purpose of this
 * function. Note that this this function is only necessary is the
 * {@link ParticleAllocator} allocated memory to the userdata attribute of
 * a {@link Particle3} object. Otherwise, it can be ignored.
 *
 * The function type is equivalent to
 *
 *      std::function<void(Particle3* part)>
 *
 * @param part  The particle to recycle
 */
typedef std::function<void(Particle3* part)> ParticleDeallocator;

/**
 * @typedef ParticleUpdater
 *
 * This type represents a function to update a {@link Particle3} object.
 *
 * This function provides the user-defined simulation for the particle system.
 * It takes a {@link Particle3} object and defines a {@link ParticleInstance}
 * for rendering.
 *
 * While this function should update the attributes of the {@link Particle3},
 * it is not necessary to update the `life` attribute, as that value is managed
 * by the system. Any changes to that attribute will unnaturally shorten or
 * lengthen the lifespan of the particle. In addition, any values set to
 * `distance` and `delay` will be overwritten.
 *
 * This function should return true if the update was successful. It should
 * return false if the update failed and the particle should be deleted.
 *
 * This function type is equivalent to
 *
 *      std::function<bool(float delta, Particle3* part, ParticleInstance* inst)>
 *
 * @param delta The time passed since the last call to update
 * @param part  The particle to simulate
 * @param inst  The object to hold the simulation results
 */
typedef std::function<bool(float delta, Particle3* part, ParticleInstance* inst)> ParticleUpdater;


#pragma mark -
#pragma mark Particle System
/**
 * This class implements a (3d) particle system.
 *
 * A particle system is a {@link graphics::Mesh} instanced many times to
 * display a large collection of images. Instances are particles, which are
 * represented the {@link Particle3} class.
 *
 * Particle simulation is defined via a user-defined {@link ParticleUpdater}
 * function. Without this function, no instance data will be created for the
 * particles, so nothing can be rendered to the screen.
 *
 * While we do not support user-defined particles, it is possible to add user
 * data to a particle object with the function types {@link ParticleAllocator}
 * and {@link ParticleDeallocator}. In fact, {@link ParticleAllocator} is
 * required to emit any particles. Without it, no particles will be created.
 * On the other hand, {@link ParticleDeallocator} is optional and only required
 * to prevent possible memory leaks.
 *
 * Each particle system has its own {@link InstanceBuffer}. This is done for
 * performance reasons. This is different that a {@link SpriteBatch} which has
 * a single buffer that would be used for all objects of the same time. This
 * particle system and its internal buffer should be combined with a particle
 * shader to display this particles.
 *
 * While particle systems are designed for 3d particles, they work perfectly
 * well in 2d scene graphs. In that case, the particle classes should always
 * set the z-value for the particle instances to 0. In addition, you should
 * call {@link #set2d} to prevent unnecessary z-sorting.
 */
class ParticleSystem {
private:
    /** The emitters attached to this particle system */
    std::unordered_map<std::string, ParticleEmitter> _emitters;
    /** The instance/vertex buffer for rendering */
    std::shared_ptr<InstanceBuffer> _renderBuffer;
    /** The particle instances */
    ParticleInstance* _instances;
    /** The user defined particles */
    Particle3* _particles;
    /** The particle mesh template */
    Mesh<ParticleVertex> _mesh;
    /** The number of supported particles */
    size_t _capacity;
    /** The number of allocated particles */
    size_t _allocated;
    /** How long this particle systems has been running (in seconds) */
    double _duration;
    /** Whether we need to greedily recycle particles */
    bool _greedy;
    /** The oldest, non-recycled particle */
    int  _oldest;
    /** Whether to optimize this particle system for 2d */
    bool _is2d;
    
    /** Function pointer for allocating particles */
    ParticleAllocator _allocator;
    
    /** Function pointer for deallocating particles */
    ParticleDeallocator _deallocator;
    
    /** Function pointer for updating particles */
    ParticleUpdater _updater;
    
public:
#pragma mark Constructors
    /**
     * Creates a new unitialized particle system.
     *
     * This particle system has degenerate values for all attributes. No
     * particles will be generated until it is initialized.
     */
    ParticleSystem();
    
    /**
     * Deletes this particle system, disposing all of the resources.
     */
    ~ParticleSystem() { dispose(); }
    
    /**
     * Disposes the emitters and allocation lists for this particle system.
     *
     * You must reinitialize the particle system to use it.
     */
    void dispose();
    
    /**
     * Initializes this particle system to have the given capacity.
     *
     * The particle system will have an empty mesh and no emitters. It will
     * also have no user-defined update or allocation functions. It will not
     * generate any particles until all of these are set.
     *
     * @param capacity  The particle capacity
     *
     * @return true if initialization was successful
     */
    bool init(size_t capacity);
    
    /**
     * Initializes this particle system with the given capacity and mesh.
     *
     * The particle system will have no emitters. It will also have no
     * user-defined update or allocation functions. It will not generate any
     * particles until all of these are set.
     *
     * This version of the initializer will copy the original mesh.
     *
     * @param capacity  The particle capacity
     * @param mesh      The template mesh to copy
     *
     * @return true if initialization was successful
     */
    bool initWithMesh(size_t capacity, const Mesh<ParticleVertex>& mesh);
    
    /**
     * Initializes this particle system with the given capacity and mesh.
     *
     * The particle system will have no emitters. It will also have no
     * user-defined update or allocation functions. It will not generate any
     * particles until all of these are set.
     *
     * This version of the initializer will acquire the resources of the
     * original mesh. Use std::move to use this initializer.
     *
     * @param capacity  The particle capacity
     * @param mesh      The template mesh to acquire
     *
     * @return true if initialization was successful
     */
    bool initWithMesh(size_t capacity, Mesh<ParticleVertex>&& mesh);
    
    /**
     * Initializes a particle template from the given JsonValue
     *
     * The JsonValue should either be an array or an JSON object. If it is an
     * array, the elements should all be float arrays of length four,
     * representing the individual {@link ParticleVertex} vertices. These
     * vertices will be interpretted as a triangle fan.
     *
     * On the other hand, if it is a JSON object, it supports the following
     * attributes:
     *
     *     "capacity":  An int with the maximum capacity
     *     "mesh":      An array or JSON object representing the mesh (see below)
     *     "emitters":  An object with key/emitter pairs
     *
     * All attributes except "capacity" are optional. If "emitters" is missing
     * or empty, there are no emitters. The values in the key/value pairs for
     * the emitter are per the specification for {@link ParticleEmitter}.
     *
     * If the "mesh" is missing, the mesh template is empty. If it is an array,
     * the elements should all be float arrays of length four, representing the
     * individual {@link ParticleVertex} vertices. Otherwise, it should have the
     * following attributes:
     *
     *     "vertices":      An array of float arrays of length four
     *     "indices":       An intenger list of triangle indices (in multiples of 3)
     *     "triangulator":  One of 'monotone', 'earclip', 'delaunay', 'fan', or 'strip'
     *
     * All attributes are optional. If "vertices" are missing, the mesh will
     * be empty. If both "indices" and "triangulator" are missing, the mesh
     * will use a triangle fan. The "triangulator" choice will only be applied
     * if the "indices" are missing.
     *
     * The particle system will only create a graphics buffer if buffer is true.
     * This is to handle cases where the sprite mesh is created in a separate
     * thread (as OpenGL only allows graphics buffers to be made on the main
     * thread).
     *
     * Note that the JSON does not a provide a way to specify the user-defined
     * allocation and update functions. No particles will be generated until
     * these are set.
     *
     * @param data      The JSON object specifying the particle system
     * @param buffer    Whether to create a graphics buffer
     *
     * @return true if initialization was successful
     */
    bool initWithData(const std::shared_ptr<JsonValue> data,
                      bool buffer = true);
    
    /**
     * Returns a newly allocated particle system with the given capacity.
     *
     * The particle system will have an empty template and no emitters. It will
     * also have no user-defined update or allocation functions. It will not
     * generate any particles until all of these are set.
     *
     * @param capacity  The particle capacity
     *
     * @return a newly allocated particle system with the given capacity.
     */
    static std::shared_ptr<ParticleSystem> alloc(size_t capacity) {
        std::shared_ptr<ParticleSystem> result = std::make_shared<ParticleSystem>();
        return (result->init(capacity) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated particle system with the given capacity and mesh.
     *
     * The particle system will have no emitters. It will also have no
     * user-defined update or allocation functions. It will not generate any
     * particles until all of these are set.
     *
     * This version of the initializer will acquire the resources of the
     * original mesh. Use std::move to use this initializer.
     *
     * @param capacity  The particle capacity
     * @param mesh      The template mesh to copy
     *
     * @return a newly allocated particle system with the given capacity and mesh.
     */
    static std::shared_ptr<ParticleSystem> allocWithMesh(size_t capacity, const Mesh<ParticleVertex>& mesh) {
        std::shared_ptr<ParticleSystem> result = std::make_shared<ParticleSystem>();
        return (result->initWithMesh(capacity,mesh) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated particle system with the given capacity and mesh.
     *
     * The particle system will have no emitters. It will also have no
     * user-defined update or allocation functions. It will not generate any
     * particles until all of these are set.
     *
     * This version of the initializer will acquire the resources of the
     * original mesh. Use std::move to use this initializer.
     *
     * @param capacity  The particle capacity
     * @param mesh      The template mesh to acquire
     *
     * @return a newly allocated particle system with the given capacity and mesh.
     */
    static std::shared_ptr<ParticleSystem> allocWithMesh(size_t capacity, Mesh<ParticleVertex>&& mesh) {
        std::shared_ptr<ParticleSystem> result = std::make_shared<ParticleSystem>();
        return (result->initWithMesh(capacity,mesh) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated particle system from the given JsonValue
     *
     * The JsonValue should either be an array or an JSON object. If it is an
     * array, the elements should all be float arrays of length four,
     * representing the individual {@link ParticleVertex} vertices. These
     * vertices will be interpretted as a triangle fan.
     *
     * On the other hand, if it is a JSON object, it supports the following
     * attributes:
     *
     *     "capacity":  An int with the maximum capacity
     *     "mesh":      An array or JSON object representing the mesh (see below)
     *     "emitters":  An object with key/emitter pairs
     *
     * All attributes except "capacity" are optional. If "emitters" is missing
     * or empty, there are no emitters. The values in the key/value pairs for
     * the emitter are per the specification for {@link ParticleEmitter}.
     *
     * If the "mesh" is missing, the mesh template is empty. If it is an array,
     * the elements should all be float arrays of length four, representing the
     * individual {@link ParticleVertex} vertices. Otherwise, it should have the
     * following attributes:
     *
     *     "vertices":      An array of float arrays of length four
     *     "indices":       An intenger list of triangle indices (in multiples of 3)
     *     "triangulator":  One of 'monotone', 'earclip', 'delaunay', 'fan', or 'strip'
     *
     * All attributes are optional. If "vertices" are missing, the mesh will
     * be empty. If both "indices" and "triangulator" are missing, the mesh
     * will use a triangle fan. The "triangulator" choice will only be applied
     * if the "indices" are missing.
     *
     * The particle system will only create a graphics buffer if buffer is true.
     * This is to handle cases where the sprite mesh is created in a separate
     * thread (as OpenGL only allows graphics buffers to be made on the main
     * thread).
     *
     * Note that the JSON does not a provide a way to specify the user-defined
     * allocation and update functions. No particles will be generated until
     * these are set.
     *
     * @param data      The JSON object specifying the particle system
     * @param buffer    Whether to create a graphics buffer
     *
     * @return a newly allocated particle system from the given JsonValue
     */
    static std::shared_ptr<ParticleSystem> allocWithData(const std::shared_ptr<JsonValue> data,
                                                         bool buffer = true) {
        std::shared_ptr<ParticleSystem> result = std::make_shared<ParticleSystem>();
        return (result->initWithData(data,buffer) ? result : nullptr);
        
    }
    
#pragma mark Particles
    /**
     * Returns the array of particles.
     *
     * This array will have {@link #getCapacity} length. However, only the first
     * {@link #getAllocated} elements will be in use.
     *
     * @return the array of particles.
     */
    const Particle3* getParticles() const {
        return _particles;
    }
    
    /**
     * Returns the array of instance data.
     *
     * This array should be passed to the shader for drawing. This array will
     * have {@link #getCapacity} length. However, only the first
     * {@link #getAllocated} elements will be in use.
     *
     * @return the array of instance data.
     */
    const ParticleInstance* getInstances() const {
        return _instances;
    }
    
    /**
     * Returns the mesh template associated with this particle system.
     *
     * This mesh can be safely changed mid-simulation. It only affects how
     * particles are rendered, not their state. However, changing the mesh
     * affects the {@link #getInstanceBuffer}. Therefore, the mesh should
     * never be modified directly. Changes should go through {@link #setMesh}.
     *
     * @return the mesh template associated with this particle system.
     */
    const Mesh<ParticleVertex>& getMesh() const {
        return _mesh;
    }
    
    /**
     * Sets the mesh template associated with this particle system.
     *
     * This mesh can be safely changed mid-simulation. It only affects how
     * particles are rendered, not their state. However, changing the mesh
     * affects the {@link #getInstanceBuffer}. Therefore, the mesh should
     * never be modified directly. Changes should go through {@link #setMesh}.
     *
     * @param mesh  The particle mesh template
     */
    void setMesh(const Mesh<ParticleVertex>& mesh);
    
    /**
     * Sets the mesh template associated with this particle system.
     *
     * This mesh can be safely changed mid-simulation. It only affects how
     * particles are rendered, not their state. However, changing the mesh
     * affects the {@link #getInstanceBuffer}. Therefore, the mesh should
     * never be modified directly. Changes should go through {@link #setMesh}.
     *
     * @param mesh  The particle mesh template
     */
    void setMesh(Mesh<ParticleVertex>&& mesh);
    
    /**
     * Returns the emitters for this particle system.
     *
     * Each emitter is identified by a user-specified key. It is safe to
     * change the attributes of an emitter mid-simulation, but any changes
     * will only be applied to new particles, not existing ones.
     *
     * @return the emitters for this particle system.
     */
    const std::unordered_map<std::string, ParticleEmitter>& getEmitters() const {
        return _emitters;
    }
    
    /**
     * Adds an emitter to the particle system.
     *
     * This method will do nothing if the key is already in use by another
     * emitter. The emitter will be immediately integrated into the simulation.
     *
     * @param key   The key identifing this emitter
     * @param data  The emitter data
     */
    void addEmitter(const std::string key, const ParticleEmitter& data);
    
    /**
     * Removes the emitter with the given key.
     *
     * Any particles previously created by the emitter will remain part of the
     * simultation until their life value reaches 0. This method will have no
     * effect if there is no emitter with the given key.
     *
     * @param key   The key identifing the emitter
     */
    void removeEmitter(const std::string key);
    
#pragma mark Attributes
    /**
     * Returns the instance buffer for this particle system.
     *
     * This buffer is used to render the particles. It should be combined
     * with the particle shader.
     *
     * @return the instance buffer for this particle system.
     */
    const std::shared_ptr<InstanceBuffer> getInstanceBuffer() const { return _renderBuffer; }
    
    /**
     * Returns the capacity of this particle system.
     *
     * The capacity the maximum number of particles that can be allocated at
     * any given time.
     *
     * @return the capacity of this particle system.
     */
    size_t getCapacity() const { return _capacity; }
    
    /**
     * Returns the number of particles currently allocated.
     *
     * @return the number of particles currently allocated.
     */
    size_t getAllocated() const { return _allocated; }
    
    /**
     * Returns whether this particle system is optimized for 2d.
     *
     * A 2d particle system has no z-value and does not require z-sorting.
     *
     * @return whether this particle system is optimized for 2d
     */
    bool is2d() const { return _is2d; }
    
    /**
     * Sets whether this particle system is optimized for 2d.
     *
     * A 2d particle system has no z-value and does not require z-sorting.
     *
     * @param value Whether this particle system is optimized for 2d
     */
    void set2d(bool value) { _is2d = value; }
    
#pragma mark Simulation
    /**
     * Returns the allocation function associated with this system.
     *
     * If this function pointer is null, no particles will be allocated
     * from any of the emitters.
     *
     * @return the allocation function associated with this system.
     */
    ParticleAllocator getAllocator() const { return _allocator; }
    
    /**
     * Sets the allocation function associated with this system.
     *
     * If this function pointer is null, no particles will be allocated
     * from any of the emitters.
     *
     * @param func  The allocation function associated with this system.
     */
    void setAllocator(ParticleAllocator func) {
        _allocator = func;
    }
    
    /**
     * Returns the deallocation function associated with this system.
     *
     * This function pointer is optional. It is only needed to clean up
     * particles where memory was previously allocated. If there is no chance
     * of a memory leak, it can be omitted.
     *
     * @return the deallocation function associated with this system.
     */
    ParticleDeallocator getDeallocator() const { return _deallocator; }
    
    /**
     * Sets the desllocation function associated with this system.
     *
     * This function pointer is optional. It is only needed to clean up
     * particles where memory was previously allocated. If there is no chance
     * of a memory leak, it can be omitted.
     *
     * @param func  The deallocation function associated with this system.
     */
    void setDeallocator(ParticleDeallocator func) {
        _deallocator = func;
    }
    
    /**
     * Returns the update function associated with this system.
     *
     * If this function pointer is null, no instance data will be created for
     * the particles, so nothing can be rendered to the screen. Indeed, the
     * result of {@link #getInstances} is undefined if this value is null.
     *
     * @return the update function associated with this system.
     */
    ParticleUpdater getUpdater() const { return _updater; }
    
    /**
     * Sets the update function associated with this system.
     *
     * If this function pointer is null, no instance data will be created for
     * the particles, so nothing can be rendered to the screen. Indeed, the
     * result of {@link #getInstances} is undefined if this value is null.
     *
     * @param func  The update function associated with this system.
     */
    void setUpdater(ParticleUpdater func) {
        _updater = func;
    }
    
    /**
     * Updates the simulation by the given amount of time.
     *
     * Most of the work of this method is implemented by the particle class.
     * This method manages particle emission (with delay) and camera distance.
     *
     * @param delta     The time passed in the simulation
     * @param camera    The camera position in world space
     */
    void update(float delta, const Vec3 camera);
    
    /**
     * Draws the render buffer with the given shader.
     *
     * @param shader    The shader to draw with
     */
    void draw(const std::shared_ptr<Shader>& shader);
    
private:
    /**
     * Emits an new particles over the given time period.
     *
     * This may emit many particles if the emitter time interval is less than
     * delta. This method will use the delay attribute to simulate such
     * particles continuously.
     *
     * @param delta The time passed in the simulation
     */
    void emit(float delta);
    
    /**
     * Returns a reference to a newly allocated particle.
     *
     * Particles are allocated from internal memory. If the maximum number of
     * particles has been reached, this will recycle the oldest particle.
     * However, behavior is undefined if more than {@link #getCapacity}
     * particles must be recycled.
     *
     * @return a reference to a newly allocated particle.
     */
    Particle3& allocate();
    
    /**
     * Allocates the instance buffer for this particle system
     *
     * @return true if the buffer was successfully created
     */
    bool createInstanceBuffer();
    
    // Allow the loader to serialize this asset
    friend class ParticleLoader;
};

    }

}

#endif /* __CU_PARTICLE_SYSTEM_H__ */
