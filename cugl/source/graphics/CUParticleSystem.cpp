//
//  CUParticleSystem.cpp
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
//  system to be templated as well). The alternate is subclassing, which causes
//  issues with allocation and heap fragmentation.
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
#include <cugl/graphics/CUParticleSystem.h>
#include <cugl/graphics/CUInstanceBuffer.h>

using namespace cugl;
using namespace cugl::graphics;

#pragma mark Particle Vertex
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
ParticleVertex::ParticleVertex(const std::shared_ptr<JsonValue>& json) {
    set(json);
}

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
ParticleVertex& ParticleVertex::set(const std::shared_ptr<JsonValue>& json) {
    CUAssertLog(json->isArray() || json->isObject(), "Vertex %s is not valid",
                json->toString().c_str());
    if (json->isArray()) {
        float values[4];
        int pos = 0;
        for(int ii = 0; ii < json->size() && ii < 4; ii++) {
            values[pos++] = json->get(ii)->asFloat(0.0f);
        }
        for(int ii = pos; ii < 4; ii++) {
            values[ii] = 0;
        }
        position.set(values[0],values[1]);
        texCoord.set(values[2],values[3]);
    } else if (json->isObject()) {
        if (json->has("position")) {
            JsonValue* child = json->get("position").get();
            CUAssertLog(child->isArray(), "Vertex %s is not an array", child->toString().c_str());
            CUAssertLog(child->size() == 2, "Vertex %s does not have 2 elements", child->toString().c_str());
            position.x = child->get(0)->asFloat(0.0f);
            position.y = child->get(1)->asFloat(0.0f);
        }
        if (json->has("texcoord")) {
            JsonValue* child = json->get("texcoord").get();
            CUAssertLog(child->isArray(), "Texture Coord %s is not an array",
                        child->toString().c_str());
            CUAssertLog(child->size() == 2, "Texture Coord %s does not have 2 elements",
                        child->toString().c_str());
            texCoord.x = child->get(0)->asFloat(0.0f);
            texCoord.y = child->get(1)->asFloat(0.0f);
        }
    }
    
    return *this;
}

#pragma mark -
#pragma mark Particle Emitter
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
 *     "interval":  A float representing the emission interval in seconds
 *
 * All of these values are optional. If "random" is missing, the clock will
 * be used for the seed. All other attributes will be zeroed out.
 *
 * @param data      The JSON object specifying the emitter
 */
ParticleEmitter::ParticleEmitter(const std::shared_ptr<JsonValue>& data) {
    if (data->has("random")) {
        JsonValue* gen = data->get("random").get();
        if (gen->isNumber()) {
            generator = Random::allocWithSeed(gen->asLong());
        } else {
            std::vector<Uint64> array;
            for(auto it = gen->children().begin(); it != gen->children().end(); ++it) {
                array.push_back((*it)->asLong());
            }
            generator = Random::allocWithArray(array.data(), array.size());
        }
    }
    
    if (data->has("position")) {
        JsonValue* pos = data->get("position").get();
        CUAssertLog(pos->size() >= 3, "'position' must be a three element number array");
        position.x = pos->get(0)->asFloat(0.0f);
        position.y = pos->get(1)->asFloat(0.0f);
        position.z = pos->get(2)->asFloat(0.0f);
    }
    
    if (data->has("velocity")) {
        JsonValue* vel = data->get("velocity").get();
        CUAssertLog(vel->size() >= 3, "'velocity' must be a three element number array");
        velocity.x = vel->get(0)->asFloat(0.0f);
        velocity.y = vel->get(1)->asFloat(0.0f);
        velocity.z = vel->get(2)->asFloat(0.0f);
    }
    
    lifespan = data->getFloat("lifespan",0);
    if (lifespan < 0) { lifespan = 0; }
    
    interval = data->getFloat("interval",0);
    if (interval < 0) { interval = 0; }
}

#pragma mark -
#pragma mark Particle System

/**
 * Creates a new unitialized particle system.
 *
 * This particle system degenerate values for all attributes. No particles
 * will be generated until it is initialized.
 */
ParticleSystem::ParticleSystem() :
_instances(nullptr),
_particles(nullptr),
_duration(0),
_capacity(0),
_allocated(0),
_greedy(false),
_oldest(0),
_is2d(false) {}

/**
 * Disposes the emitters and allocation lists for this particle system.
 *
 * You must reinitialize the particle system to use it.
 */
void ParticleSystem::dispose() {
    if (_instances != nullptr) {
        delete[] _instances;
        delete[] _particles;
        _particles = nullptr;
        _instances = nullptr;
        _renderBuffer = nullptr;
        _emitters.clear();
        _duration = 0;
        _capacity = 0;
        _allocated = 0;
        _greedy = false;
        _is2d = false;
        _oldest = 0;
    }
}

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
bool ParticleSystem::init(size_t capacity) {
    _particles = new Particle3[capacity];
    _instances = new ParticleInstance[capacity];
    _capacity = capacity;
    
    if (capacity == 0 || _particles == nullptr || _instances == nullptr) {
        return false;
    }
    
    // Allocate the vertex buffer
    if (_mesh.vertices.size()) {
        createInstanceBuffer();
    }
    return true;
}

/**
 * Initializes this particle system to have the given capacity.
 *
 * The particle system will have no emitters. It will also have no
 * user-defined update or allocation functions. It will not generate any
 * particles until all of these are set.
 *
 * @param capacity  The particle capacity
 * @param mesh      The template mesh
 *
 * @return true if initialization was successful
 */
bool ParticleSystem::initWithMesh(size_t capacity, const Mesh<ParticleVertex>& mesh) {
    _particles = new Particle3[capacity];
    _instances = new ParticleInstance[capacity];
    _capacity = capacity;
    
    if (capacity == 0 || _particles == nullptr || _instances == nullptr) {
        return false;
    }
    
    // Allocate the vertex buffer
    _mesh = mesh;
    if (_mesh.vertices.size()) {
        createInstanceBuffer();
    }
    return true;
}

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
bool ParticleSystem::initWithMesh(size_t capacity, Mesh<ParticleVertex>&& mesh) {
    _particles = new Particle3[capacity];
    _instances = new ParticleInstance[capacity];
    _capacity = capacity;
    
    if (capacity == 0 || _particles == nullptr || _instances == nullptr) {
        return false;
    }
    
    // Allocate the vertex buffer
    _mesh = std::move(mesh);
    if (_mesh.vertices.size()) {
        createInstanceBuffer();
    }
    return true;
}

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
bool ParticleSystem::initWithData(const std::shared_ptr<JsonValue> data, bool buffer) {
    if (!data->has("capacity")) {
        return false;
    }
    
    _capacity = data->getLong("capacity",0);
    bool success = _capacity > 0;

    if (success && data->has("mesh")) {
        parse_mesh<ParticleVertex>(_mesh, data->get("mesh"));
    }
    success = success && _mesh.vertices.size() > 0;
        
    if (data->has("emitters")) {
        JsonValue* emits = data->get("emitters").get();
        for(auto it = emits->children().begin(); it != emits->children().end(); ++it) {
            JsonValue* child = it->get();
            _emitters.try_emplace(child->key(), *it);
        }
    }


    if (success) {
        _particles = new Particle3[_capacity];
        _instances = new ParticleInstance[_capacity];
        if (_particles == nullptr || _instances == nullptr) {
            return false;
        }
    }
    
    // Allocate the vertex buffer
    if (buffer && success) {
        success = createInstanceBuffer();
    }
    return success;
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
void ParticleSystem::setMesh(const Mesh<ParticleVertex>& mesh) {
    _mesh = mesh;
    createInstanceBuffer();
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
void ParticleSystem::setMesh(Mesh<ParticleVertex>&& mesh) {
    _mesh = std::move(mesh);
    createInstanceBuffer();
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
void ParticleSystem::addEmitter(const std::string key, const ParticleEmitter& data) {
    auto success = _emitters.try_emplace(key,data);
    if (success.second) {
        success.first->second.remainder = _duration;
    }
}

/**
 * Removes the emitter with the given key.
 *
 * Any particles previously created by the emitter will remain part of the
 * simultation until their life value reaches 0. This method will have no
 * effect if there is no emitter with the given key.
 *
 * @param key   The key identifing the emitter
 */
void ParticleSystem::removeEmitter(const std::string key) {
    auto it = _emitters.find(key);
    if (it != _emitters.end()) {
        _emitters.erase(it);
    }
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
void ParticleSystem::update(float delta, const Vec3 camera) {
    // Compute the camera location with respect to this system
    float step;

    _duration += delta;
    emit(delta);
    
    // Now update the particles
    size_t offset = 0;
    for(size_t pos = 0; pos < _allocated; pos++) {
        Particle3& curr = _particles[pos];

        // Step forward in time
        step = delta-curr.delay;
        curr.life -= step;
        
        bool dispose = curr.life <= 0.0f;
        if (!dispose && _updater != nullptr) {
            if (_updater(step, &curr, _instances+offset)) {
                // Update the camera distance
                Vec4 vector = _instances[offset].position;
                Vec3 position(vector.x,vector.y,vector.z);
                curr.distance = (position-camera).lengthSquared();
                _instances[offset++].distance = curr.distance;
            } else {
                dispose = true;
            }
        }
        
        if (dispose) {
            if (_deallocator) {
                _deallocator(&curr);
            }
            // Put the particle at the end
            curr.distance = -1.0f;
        }

        curr.delay = 0;
    }
        
    std::sort(_particles,_particles+_allocated,[](const Particle3 &a, const Particle3 &b) {
        return a.distance > b.distance;
    });

    // Only sort the instances if not 2d
    if (!_is2d) {
        std::sort(_instances,_instances+offset,[](const ParticleInstance &a, const ParticleInstance &b) {
            return a.distance > b.distance;
        });
    }
    
    // Reset any changes used for greedy allocation
    _allocated = offset;
    _greedy = false;
    _oldest = 0;

    // Update the buffer
    if (_renderBuffer != nullptr) {
        _renderBuffer->loadInstanceData(_instances, (GLsizei)_allocated, GL_STREAM_DRAW);
    }
}

/**
 * Draws the render buffer with the given shader.
 *
 * @param shader    The shader to draw with
 */
void ParticleSystem::draw(const std::shared_ptr<Shader>& shader) {
    if (_renderBuffer == nullptr) {
        return;
    }
    _renderBuffer->attach(shader);
    _renderBuffer->drawInstanced(_mesh.command, (GLsizei)_mesh.indices.size(),
                                 (GLsizei)_allocated);
    _renderBuffer->unbind();
}

/**
 * Emits an new particles over the given time period.
 *
 * This may emit many particles if the emitter time interval is less than
 * delta. This method will use the delay attribute to simulate such
 * particles continuously.
 *
 * @param delta The time passed in the simulation
 */
void ParticleSystem::emit(float delta) {
    for(auto it = _emitters.begin(); it != _emitters.end(); ++it) {
        ParticleEmitter& source = it->second;
        while (source.remainder < _duration && source.interval > 0) {
            Particle3& particle = allocate();
            if (_allocator) {
                _allocator(source,&particle);
            }
            particle.delay = delta-(_duration-source.remainder);
            source.remainder += source.interval;
        }
        source.duration += delta;
    }
}

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
Particle3& ParticleSystem::allocate() {
    // Because of sort this is sufficient
    if (_allocated < _capacity) {
        return _particles[_allocated++];
    }
    
    // If we only just learned we are full, sort by age
    if (!_greedy) {
        std::sort(_particles,_particles+_capacity,[](const Particle3 &a, const Particle3 &b) {
            return a.life < b.life;
        });
        _greedy = true;
        _oldest = 0;
    }
    
    if (_oldest < _capacity) {
        return _particles[_oldest++];
    }
    
    // Buffer overflow; reset the first one
    return _particles[0];
}


/**
 * Allocates the instance buffer for this particle system
 */
bool ParticleSystem::createInstanceBuffer() {
    _renderBuffer = InstanceBuffer::alloc((GLsizei)_mesh.vertices.size(),
                                          (GLsizei)sizeof(ParticleVertex),
                                          (GLsizei)_capacity,
                                          (GLsizei)sizeof(ParticleInstance));
    if (_renderBuffer == nullptr) {
        return false;
    }
    
    // Template attributes
    _renderBuffer->setupAttribute("aPosition", 3, GL_FLOAT, GL_FALSE,
                                  offsetof(ParticleVertex,position));
    _renderBuffer->setupAttribute("aTexCoord", 2, GL_FLOAT, GL_FALSE,
                                  offsetof(ParticleVertex,texCoord));
    
    // Instance attributes
    _renderBuffer->setupInstanceAttribute("aCenter", 4, GL_FLOAT, GL_FALSE, 
                                          offsetof(ParticleInstance,position));
    _renderBuffer->setupInstanceAttribute("aColor", 4, GL_UNSIGNED_BYTE, GL_TRUE, 
                                          offsetof(ParticleInstance,color));
    _renderBuffer->setupInstanceAttribute("aOffset", 2, GL_FLOAT, GL_FALSE, 
                                          offsetof(ParticleInstance,texOffset));
    
    // Load the static data
    _renderBuffer->loadVertexData(_mesh.vertices.data(),
                                  (GLsizei)_mesh.vertices.size(),
                                  GL_STATIC_DRAW);
    _renderBuffer->loadIndexData( _mesh.indices.data(),
                                  (GLsizei)_mesh.indices.size(),
                                  GL_STATIC_DRAW);
    _renderBuffer->unbind();
    return true;
}
