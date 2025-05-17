//
//  CUParticleLoader.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a specific implementation of the Loader class to load
//  particle systems. An particle system is defined by a JSON entry specifying
//  the location and rate of its various emitters.
//
//  Note that a loader cannot specify the user-defined allocation and update
//  functions. No particles will be generated until these are set.
//
//  As with all of our loaders, this loader is designed to be attached to an
//  asset manager.  In addition, this class uses our standard shared-pointer
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
#include <cugl/graphics/loaders/CUParticleLoader.h>
#include <cugl/core/util/CUFiletools.h>
#include <cugl/core/io/CUJsonReader.h>
#include <cugl/core/CUApplication.h>

using namespace cugl;
using namespace cugl::graphics;

/**
 * Loads the portion of this asset that is safe to load outside the main thread.
 *
 * It is not safe to create an OpenGL buffer in a separate thread. However,
 * it is safe to create a {@link ParticleSystem}, so long as it does not
 * have a graphics buffer. Hence this method does the maximum amount of
 * work that can be done in asynchronous particle loading.
 *
 * @param source    The pathname to the asset
 *
 * @return the incomplete particle system
 */
std::shared_ptr<ParticleSystem> ParticleLoader::preload(const std::string key, const std::string source) {
    
    // Make sure we reference the asset directory
    bool absolute = cugl::filetool::is_absolute(source);
    CUAssertLog(!absolute, "This loader does not accept absolute paths for assets");

    std::string root = Application::get()->getAssetDirectory();
    std::string path = root+source;

    auto reader = JsonReader::alloc(path);
    std::shared_ptr<JsonValue> json = reader->readJson();
    reader->close();
    reader = nullptr;
    
    if (json == nullptr) {
        return nullptr;
    }
    json->setKey(key);
    
    return preload(json);
}

/**
 * Loads the portion of this asset that is safe to load outside the main thread.
 *
 * It is not safe to create an OpenGL buffer in a separate thread. However,
 * it is safe to create a {@link ParticleSystem}, so long as it does not
 * have a graphics buffer. Hence this method does the maximum amount of
 * work that can be done in asynchronous particle loading.
 *
 * @param json  The JSON entry specifying the asset
 *
 * @return the incomplete particle system
 */
std::shared_ptr<ParticleSystem> ParticleLoader::preload(const std::shared_ptr<JsonValue>& json) {
    if (json->isString()) {
        return preload(json->key(),json->asString());
    }
    return ParticleSystem::allocWithData(json,false);
}

/**
 * Creates an OpenGL buffer for the particle system, and assigns it the given key.
 *
 * This method finishes the asset loading started in {@link preload}. This
 * step is not safe to be done in a separate thread. Instead, it takes
 * place in the main CUGL thread via {@link Application#schedule}.
 *
 * This method supports an optional callback function which reports whether
 * the asset was successfully materialized.
 *
 * @param key       The key to access the asset after loading
 * @param system    The particle system to materialize
 * @param callback  An optional callback for asynchronous loading
 *
 * @return true if materialization was successful
 */
bool ParticleLoader::materialize(const std::string key,
                                 const std::shared_ptr<ParticleSystem>& system,
                                 LoaderCallback callback) {
    bool success = false;
    if (system != nullptr) {
        success = system->createInstanceBuffer();
    }
    
    if (success) {
        _assets[key] = system;
    }

    if (callback != nullptr) {
        callback(key,success);
    }
    
    _queue.erase(key);
    return success;
}

/**
 * Internal method to support asset loading.
 *
 * This method supports either synchronous or asynchronous loading, as
 * specified by the given parameter. If the loading is asynchronous,
 * the user may specify an optional callback function.
 *
 * This method will split the loading across the {@link preload} and
 * {@link materialize} methods. This ensures that asynchronous loading
 * is safe.
 *
 * A particle system is defined by a JSON file, whose contents satisfy the
 * specification of {@link ParticleSystem#allocWithData}.
 *
 * @param key       The key to access the asset after loading
 * @param source    The pathname to the asset
 * @param callback  An optional callback for asynchronous loading
 * @param async     Whether the asset was loaded asynchronously
 *
 * @return true if the asset was successfully loaded
 */
bool ParticleLoader::read(const std::string key, const std::string source,
                          LoaderCallback callback, bool async) {
    if (_assets.find(key) != _assets.end() || _queue.find(key) != _queue.end()) {
        return false;
    }

    bool success = true;
    if (_loader == nullptr || !async) {
        enqueue(key);
        std::shared_ptr<ParticleSystem> system = preload(key,source);
        success = materialize(key, system, callback);
    } else {
        _loader->addTask([=,this](void) {
            this->enqueue(key);
            std::shared_ptr<ParticleSystem> system = this->preload(key,source);
            Application::get()->schedule([=,this](void){
                this->materialize(key,system,callback);
                return false;
            });
        });
    }
    
    return success;
}

/**
 * Internal method to support asset loading.
 *
 * This method supports either synchronous or asynchronous loading, as
 * specified by the given parameter. If the loading is asynchronous,
 * the user may specify an optional callback function.
 *
 * This method will split the loading across the {@link preload} and
 * {@link materialize} methods. This ensures that asynchronous loading
 * is safe.
 *
 * This version of read provides support for JSON directories. A particle
 * system directory entry is either an array or a JSON object (or the name
 * of a file containing either of these), satisfying the specification of
 * {@link ParticleSystem#allocWithData}.
 *
 * @param json      The directory entry for the asset
 * @param callback  An optional callback for asynchronous loading
 * @param async     Whether the asset was loaded asynchronously
 *
 * @return true if the asset was successfully loaded
 */
bool ParticleLoader::read(const std::shared_ptr<JsonValue>& json,
                          LoaderCallback callback, bool async) {
    std::string key = json->key();
    if (_assets.find(key) != _assets.end() || _queue.find(key) != _queue.end()) {
        return false;
    }

    bool success = true;
    if (_loader == nullptr || !async) {
        enqueue(key);
        std::shared_ptr<ParticleSystem> system = preload(json);
        success = materialize(key, system, callback);
    } else {
        _loader->addTask([=,this](void) {
            this->enqueue(key);
            std::shared_ptr<ParticleSystem> system = this->preload(json);
            Application::get()->schedule([=,this](void){
                this->materialize(key,system,callback);
                return false;
            });
        });
    }
    
    return success;
}
