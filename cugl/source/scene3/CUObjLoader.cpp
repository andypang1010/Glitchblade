//
//  CUObjLoader.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides a specific implementation of the Loader class to load
//  OBJ models. An OBJ asset is identified by its .obj, any associated .mtl
//  files and any associated textures. Hence there are reasons to load an OBJ
//  asset multiple times, though this is rare.
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
#include <cugl/scene3/CUObjLoader.h>
#include <cugl/scene3/CUMtlLoader.h>
#include <cugl/scene3/CUObjParser.h>
#include <cugl/core/assets/CUAssetManager.h>
#include <cugl/core/util/CUFiletools.h>
#include <cugl/core/CUApplication.h>

using namespace cugl;
using namespace cugl::scene3;
using namespace cugl::graphics;

#pragma mark -
#pragma mark Asset Loading
/**
 * Loads the portion of a material library that is safe to load outside the main thread.
 *
 * For best performance, all MTL objects should be loaded in a previous
 * pass. However, if that is not the case, then this method can load any
 * additional materials that are needed. This method uses the preload pass
 * of {@link scene3::MtlLoader}.
 *
 * @param lib       The material lib to load
 * @param loader    The material loader for the parent asset manager
 */
void ObjLoader::preloadLibrary(const std::shared_ptr<MaterialLib>& lib,
                    const std::shared_ptr<MtlLoader>& loader) {
    if (lib == nullptr || loader == nullptr) {
        return;
    }
    
    /*
    auto library = loader->getLibrary(lib->name);
    if (library == nullptr) {
        SDL_Surface* surface = loader->preload(info->path);
        if (surface != nullptr) {
            _surfaces[info->name] = surface;
        }
    }
     */
}

/**
 * Creates a material library from the given information.
 *
 * For best performance, all MTL objects should be loaded in a previous
 * pass. However, if that is not the case, then this method can load any
 * additional materials that are needed. This method uses the materialize
 * pass of {@link scene3::MtlLoader}.
 *
 * @param lib       The material lib to materialize
 * @param loader    The material loader for the parent asset manager
 */
void ObjLoader::materializeLibrary(const std::shared_ptr<MaterialLib>& info,
                                   const std::shared_ptr<MtlLoader>& loader) {
    if (info == nullptr || loader == nullptr) {
        return;
    }

    /*
    auto result = loader->get(info->path);
    if (result == nullptr) {
        auto it = _surfaces.find(info->name);
        if (it != _surfaces.end()) {
            // This deallocates surface
            loader->materialize(info->name, it->second, nullptr);
            result = loader->get(info->name);
            _surfaces.erase(it);
            
            // Do the settings
            if (result != nullptr) {
                result->setMagFilter(info->magflt);
                result->setMinFilter(info->magflt);
                result->setWrapS(info->wrapS);
                result->setWrapT(info->wrapT);
                if (info->mipmaps) {
                    result->buildMipMaps();
                }
            }
        }
    }
    
    return result;
     */
}

/**
 * Loads the portion of this asset that is safe to load outside the main thread.
 *
 * It is not safe to create an OpenGL buffer in a separate thread. However,
 * it is safe to create a {@link ObjModel}, so long as it does not have a
 * graphics buffer. Hence this method does the maximum amount of work that
 * can be done in asynchronous OBJ loading.
 *
 * If the OBJ file has any associated materials, this method will create a
 * thread-safe instance using {@link MtlLoader#preload}. Those materials
 * will be materialized the same time the OBJ mesh is materialized.
 *
 * @param key       The key to access the asset after loading
 * @param source    The pathname to the asset
 *
 * @return the incomplete OBJ model
 */
std::shared_ptr<ObjModel> ObjLoader::preload(const std::string key, const std::string source) {
    
    // Make sure we reference the asset directory
    bool absolute = cugl::filetool::is_absolute(source);
    CUAssertLog(!absolute, "This loader does not accept absolute paths for assets");

    std::string root = Application::get()->getAssetDirectory();
    std::string prefix = cugl::filetool::split_path(source).first;
    prefix.push_back(cugl::filetool::path_sep);
    std::string path = root+source;

    std::shared_ptr<ModelInfo> info = _parser->parseObj(source,path,false);
    std::shared_ptr<ObjModel> result = ObjModel::allocWithInfo(info,0);
    if (result == nullptr) {
        return nullptr;
    }
    
    // Make sure we have a material loader
    auto matloader = std::dynamic_pointer_cast<MtlLoader>(_manager->access<Material>());

    // Get the materials libraries
    std::unordered_map<std::string,std::shared_ptr<MaterialLib>> libmap;
    for(auto it = info->libraries.begin(); it != info->libraries.end(); ++it) {
        auto lib = matloader->preload(it->first,root+prefix+it->first);
        libmap[it->first] = lib;
    }
    _libraries[key] = libmap;
    
    return result;
}

/**
 * Loads the portion of this asset that is safe to load outside the main thread.
 *
 * It is not safe to create an OpenGL buffer in a separate thread. However,
 * it is safe to create a {@link ObjModel}, so long as it does not have a
 * graphics buffer. Hence this method does the maximum amount of work that
 * can be done in asynchronous OBJ loading.
 *
 * If the MTL file has any associated textures, this method will create a
 * thread-safe instance using {@link TextureLoader#preload}. Those textures
 * will be materialized the same time the OBJ mesh is materialized.
 *
 * @param json  The JSON entry specifying the asset
 *
 * @return the incomplete OBJ model
 */
std::shared_ptr<ObjModel> ObjLoader::preload(const std::shared_ptr<JsonValue>& json) {
    if (json->isString()) {
        return preload(json->key(),json->asString());
    } else if (!json->has("file")) {
        return nullptr;
    }
    
    std::string key = json->key();
    std::string source = json->getString("file");
    
    // Make sure we reference the asset directory
    bool absolute = cugl::filetool::is_absolute(source);
    CUAssertLog(!absolute, "This loader does not accept absolute paths for assets");

    std::string root = Application::get()->getAssetDirectory();
    std::string prefix = cugl::filetool::split_path(source).first;
    prefix.push_back(cugl::filetool::path_sep);
    std::string path = root+source;

    std::shared_ptr<ModelInfo> info = _parser->parseObj(key,path,false);
    std::shared_ptr<ObjModel> result = ObjModel::allocWithInfo(info,0);
    if (result == nullptr) {
        return nullptr;
    }
    
    // Make sure we have a material loader
    auto matloader = std::dynamic_pointer_cast<MtlLoader>(_manager->access<Material>());

    JsonValue* mtljson = nullptr;
    if (json->has("mtls")) {
        mtljson = json->get("mtls").get();
    }
    
    // Get the materials libraries
    std::unordered_map<std::string,std::shared_ptr<MaterialLib>> libmap;
    for(auto it = info->libraries.begin(); it !=info->libraries.end(); ++it) {
        std::shared_ptr<MaterialLib> lib = nullptr;
        
        if (mtljson != nullptr && mtljson->has(it->first)) {
            auto child = mtljson->get(it->first);
            if (child->isString()) {
                std::string subkey = key+"."+child->key();
                child->setKey(subkey);
                lib = matloader->getLibrary(child->asString());
            }
            if (lib == nullptr) {
                lib = matloader->preload(child);
            }
        } else {
            lib = matloader->preload(it->first,prefix+it->first);
        }
        libmap[it->first] = lib;
    }
    _libraries[key] = libmap;

    return result;

}

/**
 * Creates an OpenGL buffer for the model, and assigns it the given key.
 *
 * This method finishes the asset loading started in {@link preload}. This
 * step is not safe to be done in a separate thread. Instead, it takes
 * place in the main CUGL thread via {@link Application#schedule}.
 *
 * Any additional textures loaded by the associated MTL will be materialized
 * at this time. This method will only work if all of the textures and
 * MTL files associated with this OBJ file are in the same directory as it.
 *
 * This method supports an optional callback function which reports whether
 * the asset was successfully materialized.
 *
 * @param key       The key to access the asset after loading
 * @param model     The model to materialize
 * @param callback  An optional callback for asynchronous loading
 *
 * @return true if materialization was successful
 */
bool ObjLoader::materialize(const std::string key,
                            const std::shared_ptr<ObjModel>& model,
                            LoaderCallback callback) {
    if (model == nullptr) {
        _queue.erase(key);
        if (callback != nullptr) {
            callback(key,false);
        }
        return false;
    }

    // Create the OpenGL buffers
    bool success = true;
    for(auto it = model->getMeshes().begin(); success && it != model->getMeshes().end(); ++it) {
        success = (*it)->createBuffer();
    }
    
    // Now we need to complete the materials
    auto matloader = std::dynamic_pointer_cast<MtlLoader>(_manager->access<Material>());
    auto libraries = _libraries.find(key);
    if (libraries != _libraries.end()) {
        auto libset = libraries->second;
        for(auto it = libset.begin(); it != libset.end(); ++it) {
            if (!it->second->complete) {
                matloader->materialize(it->first, it->second, nullptr);
            }
        }
        
        // If we have materials, hook them up to the meshes
        for(auto it = model->getMeshes().begin(); success && it != model->getMeshes().end(); ++it) {
            auto mesh = *it;
            std::string name = mesh->getMaterialName();

            std::shared_ptr<Material> material = nullptr;
            for(auto jt = libset.begin(); material == nullptr && jt != libset.end(); ++jt) {
                auto kt = jt->second->materials.find(name);
                if (kt != jt->second->materials.end()) {
                    material = kt->second;
                }
            }
            
            if (material != nullptr) {
                mesh->setMaterial(material);
            }
        }
    }

    if (callback != nullptr) {
        callback(key,success);
    }
    
    _assets[key] = model;
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
 * This method will only work if all of the textures and MTL files
 * associated with this OBJ file are in the same directory as it.
 *
 * @param key       The key to access the asset after loading
 * @param source    The pathname to the asset
 * @param callback  An optional callback for asynchronous loading
 * @param async     Whether the asset was loaded asynchronously
 *
 * @return true if the asset was successfully loaded
 */
bool ObjLoader::read(const std::string key, const std::string source,
                     LoaderCallback callback, bool async) {
    if (_assets.find(key) != _assets.end() || _queue.find(key) != _queue.end()) {
        return false;
    }

    bool success = true;
    if (_loader == nullptr || !async) {
        enqueue(key);
        std::shared_ptr<ObjModel> model = preload(key, source);
        success = materialize(key,model,callback);
    } else {
        _loader->addTask([=,this](void) {
            this->enqueue(key);
            std::shared_ptr<ObjModel> model = this->preload(key,source);
            Application::get()->schedule([=,this](void){
                this->materialize(key,model,callback);
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
 * This version of read provides support for JSON directories. An OBJ
 * directory entry has the following values
 *
 *      "file":         The path to the OBJ file
 *      "mtls":         An object of key:value pairs defining MTL libraries
 *
 * The "mtls" entry is optional. For each MTL library, the key should match
 * the name of the MTL file referenced in the obj file. If there are any
 * missing MTL libraries (or the "mtls" entry is missing entirely), then
 * the loader will attempt to use the same directory as the OBJ file.
 *
 * An MTL entry is either a string (which is a reference to the path to the
 * MTL file) or a JSON object. Such a JSON object would have the following
 * values:
 *
 *      "file":         The path to the MTL file
 *      "textures":     An object of key:value pairs defining textures
 *
 * The "textures" entry is optional. For each texture, the key should match
 * the name of the texture in the MTL file. Any missing textures will
 * attempt to be loaded using the associated {@link TextureLoader}.
 *
 * The values for the texture entries should be strings or JSONs. If they
 * are string, they should be either be a key referencing a previously
 * loaded texture, or a path the texture file (the loader interprets it
 * as a path only if there is no key with that name). If it is a JSON, then
 * the JSON should follow the same rules as {@link TextureLoader}.
 *
 * @param json      The directory entry for the asset
 * @param callback  An optional callback for asynchronous loading
 * @param async     Whether the asset was loaded asynchronously
 *
 * @return true if the asset was successfully loaded
 */
bool ObjLoader::read(const std::shared_ptr<JsonValue>& json,
                     LoaderCallback callback, bool async) {
    std::string key = json->key();
    if (_assets.find(key) != _assets.end() || _queue.find(key) != _queue.end()) {
        return false;
    }

    bool success = true;
    if (_loader == nullptr || !async) {
        enqueue(key);
        std::shared_ptr<ObjModel> model = preload(json);
        std::shared_ptr<ModelInfo> info = _parser->parseJson(json);
        success = materialize(key,model,callback);
    } else {
        _loader->addTask([=,this](void) {
            this->enqueue(key);
            std::shared_ptr<ObjModel> model = this->preload(json);
            Application::get()->schedule([=,this](void){
                this->materialize(key,model,callback);
                return false;
            });
        });
    }
    
    return success;
}

/**
 * Unloads the asset for the given directory entry
 *
 * An asset may still be available if it is referenced by a smart pointer.
 * See the description of the specific implementation for how assets
 * are released.
 *
 * This method clears the internal buffers of any materials or textures
 * associated with this model.
 *
 * @param json      The directory entry for the asset
 *
 * @return true if the asset was successfully unloaded
 */
bool ObjLoader::purgeJson(const std::shared_ptr<JsonValue>& json) {
    std::string key = json->key();
    auto it = _assets.find(key);
    if (it == _assets.end()) {
        return false;
    }
    _assets.erase(it);
    
    auto jt = _materials.find(key);
    if (jt != _materials.end()) {
        _materials.erase(jt);
    }

    auto kt = _matast.find(key);
    if (kt != _matast.end()) {
        _matast.erase(kt);
    }

    return true;
}
