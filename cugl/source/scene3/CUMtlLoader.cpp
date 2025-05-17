//
//  CUMtlLoader.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides a specific implementation of the Loader class to load
//  WaveFront MTL files. Most of the time materials are loaded implicitedly by
//  an OBJ file. However, occasionally we have a bare OBJ model for which we
//  wish to swap out multiple materials.
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
//  Version: 11/15/25
//
#include <cugl/scene3/CUMtlLoader.h>
#include <cugl/scene3/CUObjParser.h>
#include <cugl/core/assets/CUAssetManager.h>
#include <cugl/graphics/loaders/CUTextureLoader.h>
#include <cugl/core/util/CUFiletools.h>
#include <cugl/core/CUApplication.h>

using namespace cugl;
using namespace cugl::scene3;
using namespace cugl::graphics;

/**
 * Updates a texture info object with data from the given json
 *
 * If the json is nullptr, this function ensures that the texture path is
 * relative (not absolute) but makes no other changes.
 *
 * @param info  The texture information
 * @param json  The JSON with the texture data
 */
static void updateTextureInfo(const std::shared_ptr<TextureInfo>& info, JsonValue* json) {
    if (info == nullptr) {
        return;
    }
    
    if (json == nullptr) {
        if (cugl::filetool::is_absolute(info->path)) {
            std::string root = Application::get()->getAssetDirectory();
            info->path = info->path.substr(root.size());
        }
        return;
    }
    
    if (json->isString()) {
        info->path = json->asString();
        return;
    }
    
    info->path = json->getString("file",info->path);
    info->minflt = gl_filter(json->getString("minfilter","nearest"));
    info->magflt = gl_filter(json->getString("magfilter","linear"));
    info->wrapS  = gl_wrap(json->getString("wrapS","clamp"));
    info->wrapT  = gl_wrap(json->getString("wrapT","clamp"));
    info->mipmaps = json->getBool("mipmaps",false);
}

#pragma mark -
#pragma mark Asset Loading
/**
 * Loads the portion of the texture that is safe to load outside the main thread.
 *
 * For best performance, all MTL textures should be loaded in a previous
 * pass. However, if that is not the case, then this method can load any
 * additional textures that are needed. This method uses the preload pass
 * of {@link graphics::TextureLoader}.
 *
 * @param info      The texture information
 * @param loader    The texture loader for the parent asset manager
 */
void MtlLoader::preloadTexture(const std::shared_ptr<TextureInfo>& info,
                               const std::shared_ptr<TextureLoader>& loader) {
    if (info == nullptr || loader == nullptr) {
        return;
    }
    
    auto texture = loader->get(info->name);
    if (texture == nullptr) {
        SDL_Surface* surface = loader->preload(info->path);
        if (surface != nullptr) {
            _surfaces[info->name] = surface;
        }
    }
}

/**
 * Creates a texture from the given information.
 *
 * For best performance, all MTL textures should be loaded in a previous
 * pass. However, if that is not the case, then this method can load any
 * additional textures that are needed. This method uses the materialize
 * pass of {@link graphics::TextureLoader}.
 *
 * @param info      The texture information
 * @param loader    The texture loader for the parent asset manager
 */
std::shared_ptr<Texture> MtlLoader::materializeTexture(const std::shared_ptr<TextureInfo>& info,
                                                       const std::shared_ptr<TextureLoader>& loader) {
    if (info == nullptr || loader == nullptr) {
        return nullptr;
    }

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
}

/**
 * Loads the portion of this asset that is safe to load outside the main thread.
 *
 * It is not safe to create an OpenGL buffer in a separate thread. However,
 * it is generally safe to create a {@link Material}, as -- aside from its
 * textures -- it does not require OpenGL for asset creation.
 *
 * If the MTL file has any associated textures, this method will create a
 * thread-safe instance using {@link graphics::TextureLoader#preload}.
 * Those textures will be materialized the same time the Material is
 * materialized on the main thread.
 *
 * Note that MTL files may have more than one material in them. For that
 * reason, you should access each material with `key.name` where `name`
 * is the name of the material. You may only use the key by itself if there
 * only one material in the file.
 *
 * @param key       The key to access the asset after loading
 * @param source    The pathname to the asset
 *
 * @return the incomplete material
 */
std::shared_ptr<MaterialLib> MtlLoader::preload(const std::string key, const std::string source) {
    // Make sure we reference the asset directory
    bool absolute = cugl::filetool::is_absolute(source);
    CUAssertLog(!absolute, "This loader does not accept absolute paths for assets");

    // Make sure we have a texture loader
    auto texloader = std::dynamic_pointer_cast<TextureLoader>(_manager->access<Texture>());

    // Get the material library
    std::shared_ptr<MaterialLib> result = _parser->parseMtl(key,source);
    
    // Now allocate the individual materials
    if (result != nullptr) {
        for(auto it = result->matinfos.begin(); it != result->matinfos.end(); ++it) {
            auto mtlinfo = it->second;
            auto material = Material::allocWithInfo(mtlinfo,false);
                
            updateTextureInfo(mtlinfo->map_Ka,nullptr);
            preloadTexture(mtlinfo->map_Ka,texloader);
            
            updateTextureInfo(mtlinfo->map_Kd,nullptr);
            preloadTexture(mtlinfo->map_Kd,texloader);
            
            updateTextureInfo(mtlinfo->map_Ks,nullptr);
            preloadTexture(mtlinfo->map_Ks,texloader);
            
            updateTextureInfo(mtlinfo->map_Kn,nullptr);
            preloadTexture(mtlinfo->map_Kn,texloader);
            
            result->materials[it->first] = material;
        }
    }
    
    return result;
}

/**
 * Loads the portion of this asset that is safe to load outside the main thread.
 *
 * It is not safe to create an OpenGL buffer in a separate thread. However,
 * it is generally safe to create a {@link Material}, as -- aside from its
 * textures -- it does not require OpenGL for asset creation.
 *
 * If the MTL file has any associated textures, this method will create a
 * thread-safe instance using {@link graphics::TextureLoader#preload}.
 * Those textures will be materialized the same time the Material is
 * materialized on the main thread.
 *
 * A MTL JSON entry can have several forms. In the simplest case, a MTL
 * entry can just be a string. In that case, it is assumed to be a path to
 * a MTL file. If that MTL file has any associated textures, they are
 * loaded implicitly (and they must be in the same directory as the MTL
 * file).
 *
 * On the other hand, if the MTL entry is itself a JSON object, then it
 * typically has the following values:
 *
 *      "file":         The path to the MTL file
 *      "textures":     An object of key:value pairs defining textures
 *
 * The "textures" entry is optional. For each texture, the key should match
 * the name of the texture in the MTL file. Any missing textures will
 * attempt to be loaded using the associated {@link graphics::TextureLoader}.
 *
 * The values for the texture entries should be strings or JSONs. If they
 * are string, they should be either be a key referencing a previously
 * loaded texture, or a path the texture file (the loader interprets it
 * as a path only if there is no key with that name). If it is a JSON, then
 * the JSON should follow the same rules as {@link graphics::TextureLoader}.
 *
 * Note that a MTL file can have multiple materials inside of it. For that
 * reason, you should always access materials with `key.name` where `name`
 * is the name of the material. You may only use the key by itself if there
 * only one material in the file.
 *
 * Finally, it is also possible for the JSON entry to define the material
 * explicitly. In that case, it would have the following values:
 *
 *      "name":             The texture name
 *      "ambient color":    The diffuse color
 *      "ambient map":      The diffuse texture
 *      "diffuse color":    The diffuse color
 *      "diffuse map":      The diffuse texture
 *      "specular color":   The specular color
 *      "specular map":     The specular texture
 *
 * Any other material properties currently require an MTL file. The maps
 * should follow the same rules as textures; either they are a key of a
 * previously loaded texture, or they are the path to a texture file. The
 * colors are either a four-element integer array (values 0..255) or a
 * string. Any string should be a web color or a Tkinter color name.
 * Materials loaded this way are referred to directly by their key. They
 * do not have any associated MTL library.
 *
 * @param json  The JSON entry specifying the asset
 *
 * @return the incomplete OBJ model
 */
std::shared_ptr<MaterialLib> MtlLoader::preload(const std::shared_ptr<JsonValue>& json) {
    if (json->isString()) {
        return preload(json->key(),json->asString());
    } else if (!json->has("file") && !json->has("name")) {
        return nullptr;
    }
    
    std::string key = json->key();

    // Make sure we have a texture loader
    auto texloader = std::dynamic_pointer_cast<TextureLoader>(_manager->access<Texture>());

    if (json->has("file")) {
        std::string source = json->getString("file");
        bool absolute = cugl::filetool::is_absolute(source);
        CUAssertLog(!absolute, "This loader does not accept absolute paths for assets");
        
        
        // Get the material library
        std::shared_ptr<MaterialLib> result = _parser->parseMtl(key,source);
        
        // Now allocate the individual materials
        if (result != nullptr) {
            JsonValue* child = nullptr;
            if (json->has("textures")) {
                child = json->get("textures").get();
            }
            
            for(auto it = result->matinfos.begin(); it != result->matinfos.end(); ++it) {
                auto mtlinfo = it->second;
                auto material = Material::allocWithInfo(mtlinfo,false);
                
                
                JsonValue* textinfo = nullptr;
                if (mtlinfo->map_Ka != nullptr && child != nullptr && child->has(mtlinfo->map_Ka->name)) {
                    textinfo = child->get(mtlinfo->map_Ka->name).get();
                }
                updateTextureInfo(mtlinfo->map_Ka,textinfo);
                preloadTexture(mtlinfo->map_Ka,texloader);
                
                if (mtlinfo->map_Kd != nullptr && child != nullptr && child->has(mtlinfo->map_Kd->name)) {
                    textinfo = child->get(mtlinfo->map_Kd->name).get();
                }
                updateTextureInfo(mtlinfo->map_Kd,textinfo);
                preloadTexture(mtlinfo->map_Kd,texloader);
                
                if (mtlinfo->map_Ks != nullptr && child != nullptr && child->has(mtlinfo->map_Ks->name)) {
                    textinfo = child->get(mtlinfo->map_Ks->name).get();
                }
                updateTextureInfo(mtlinfo->map_Ks,textinfo);
                preloadTexture(mtlinfo->map_Ks,texloader);
                
                if (mtlinfo->map_Kn != nullptr && child != nullptr && child->has(mtlinfo->map_Kn->name)) {
                    textinfo = child->get(mtlinfo->map_Kn->name).get();
                }
                updateTextureInfo(mtlinfo->map_Kn,textinfo);
                preloadTexture(mtlinfo->map_Kn,texloader);
                
                result->materials[it->first] = material;
            }
        }
        
        return result;
    } else if (json->has("name")) {
        auto info = std::make_shared<MaterialInfo>();
        auto material = Material::alloc();
        
        info->name = json->getString("name");
        material->setName(info->name);
        
        info->Ka = json_color(json->get("ambient color").get(),"#000000");
        info->Kd = json_color(json->get("diffuse color").get(),"#ffffff");
        info->Ks = json_color(json->get("specular color").get(),"#000000");

        material->setAmbientTint(info->Ka);
        material->setDiffuseTint(info->Kd);
        material->setSpecularTint(info->Ks);
        
        if (json->has("ambient map")) {
            info->map_Ka = std::make_shared<TextureInfo>();
            updateTextureInfo(info->map_Ka,json->get("ambient map").get());
            preloadTexture(info->map_Ka,texloader);
        }

        if (json->has("diffuse map")) {
            info->map_Kd = std::make_shared<TextureInfo>();
            updateTextureInfo(info->map_Kd,json->get("diffuse map").get());
            preloadTexture(info->map_Kd,texloader);
        }

        if (json->has("specular map")) {
            info->map_Ks = std::make_shared<TextureInfo>();
            updateTextureInfo(info->map_Ks,json->get("specular map").get());
            preloadTexture(info->map_Ks,texloader);
        }

        // Now make a library to put them
        std::shared_ptr<MaterialLib> result = std::make_shared<MaterialLib>();
        result->name = "__implicit__";
        result->path = "";
        result->matinfos[info->name] = info;
        result->materials[info->name] = material;
        return result;
    }
    
    return nullptr;
}

/**
 * Finishes allocation of the material and assigns it the given key.
 *
 * This method finishes the asset loading started in {@link preload}. This
 * step is not safe to be done in a separate thread. Instead, it takes
 * place in the main CUGL thread via {@link Application#schedule}.
 *
 * Any additional textures loaded by the MTL file will be materialized
 * at this time. This method will only work if all of the textures are
 * in the same directory as the MTL file.
 *
 * This method supports an optional callback function which reports whether
 * the asset was successfully materialized.
 *
 * @param key       The key to access the asset after loading
 * @param lib       The material library to materialize
 * @param callback  An optional callback for asynchronous loading
 *
 * @return true if materialization was successful
 */
bool MtlLoader::materialize(const std::string key,
                            const std::shared_ptr<MaterialLib>& lib,
                            LoaderCallback callback) {
    if (lib == nullptr && !lib->matinfos.empty()) {
        _queue.erase(key);
        if (callback != nullptr) {
            callback(key,false);
        }
        return false;
    }
    
    // Now we process the texture for all of the materials
    auto texloader = std::dynamic_pointer_cast<TextureLoader>(_manager->access<Texture>());
    for(auto it = lib->matinfos.begin(); it != lib->matinfos.end(); ++it) {
        auto jt = lib->materials.find(it->first);
        if (jt != lib->materials.end()) {
            auto material = jt->second;
            
            if (material->getAmbientMap() == nullptr) {
                material->setAmbientMap(materializeTexture(it->second->map_Ka, texloader));
            }
            if (material->getDiffuseMap() == nullptr) {
                material->setDiffuseMap(materializeTexture(it->second->map_Kd, texloader));
            }
            if (material->getSpecularMap() == nullptr) {
                material->setSpecularMap(materializeTexture(it->second->map_Ks, texloader));
            }
            if (material->getBumpMap() == nullptr) {
                material->setBumpMap(materializeTexture(it->second->map_Kn, texloader));
            }
        }
    }
    
    if (callback != nullptr) {
        callback(key,true);
    }
    
    
    // Time to assign some keys
    if (lib->name == "__implicit__" && lib->materials.size() == 1) {
        auto it = lib->materials.begin();
        _libraries[key] = lib;
        _assets[key] = it->second;
    } else {
        _libraries[key] = lib;
        for(auto it = lib->materials.begin(); it != lib->materials.end(); ++it) {
            std::string subkey = key+"."+it->second->getName();
            _assets[subkey] = it->second;
        }
        // Just in case
        _assets[key] = lib->materials.begin()->second;
    }
    lib->complete = true;
    _queue.erase(key);
    return true;
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
 * This method will only work if all of the textures are in the same
 * directory as the MTL file.
 *
 * Note that MTL files may have more than one material in them. For that
 * reason, you should access each material with `key.name` where `name`
 * is the name of the material. You may only use the key by itself if there
 * only one material in the file.
 *
 * @param key       The key to access the asset after loading
 * @param source    The pathname to the asset
 * @param callback  An optional callback for asynchronous loading
 * @param async     Whether the asset was loaded asynchronously
 *
 * @return true if the asset was successfully loaded
 */
bool MtlLoader::read(const std::string key, const std::string source,
                     LoaderCallback callback, bool async) {
    if (_assets.find(key) != _assets.end() || _queue.find(key) != _queue.end()) {
        return false;
    }

    bool success = true;
    if (_loader == nullptr || !async) {
        enqueue(key);
        std::shared_ptr<MaterialLib> lib = preload(key,source);
        success = materialize(key,lib,callback);
    } else {
        _loader->addTask([=,this](void) {
            this->enqueue(key);
            std::shared_ptr<MaterialLib> lib = this->preload(key,source);
            Application::get()->schedule([=,this](void){
                this->materialize(key,lib,callback);
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
 * A MTL JSON entry can have several forms. In the simplest case, a MTL
 * entry can just be a string. In that case, it is assumed to be a path to
 * a MTL file. If that MTL file has any associated textures, they are
 * loaded implicitly (and they must be in the same directory as the MTL
 * file).
 *
 * On the other hand, if the MTL entry is itself a JSON object, then it
 * typically has the following values:
 *
 *      "file":         The path to the MTL file
 *      "textures":     An object of key:value pairs defining textures
 *
 * The "textures" entry is optional. For each texture, the key should match
 * the name of the texture in the MTL file. Any missing textures will
 * attempt to be loaded using the associated {@link graphics::TextureLoader}.
 *
 * The values for the texture entries should be strings or JSONs. If they
 * are string, they should be either be a key referencing a previously
 * loaded texture, or a path the texture file (the loader interprets it
 * as a path only if there is no key with that name). If it is a JSON, then
 * the JSON should follow the same rules as {@link graphics::TextureLoader}.
 *
 * Note that a MTL file can have multiple materials inside of it. For that
 * reason, you should always access materials with `key.name` where `name`
 * is the name of the material. You may only use the key by itself if there
 * only one material in the file.
 *
 * Finally, it is also possible for the JSON entry to define the material
 * explicitly. In that case, it would have the following values:
 *
 *      "name":             The texture name
 *      "ambient color":    The diffuse color
 *      "ambient map":      The diffuse texture
 *      "diffuse color":    The diffuse color
 *      "diffuse map":      The diffuse texture
 *      "specular color":   The specular color
 *      "specular map":     The specular texture
 *
 * Any other material properties currently require an MTL file. The maps
 * should follow the same rules as textures; either they are a key of a
 * previously loaded texture, or they are the path to a texture file. The
 * colors are either a four-element integer array (values 0..255) or a
 * string. Any string should be a web color or a Tkinter color name.
 * Materials loaded this way are referred to directly by their key. They
 * do not have any associated MTL library.
 *
 * @param json      The directory entry for the asset
 * @param callback  An optional callback for asynchronous loading
 * @param async     Whether the asset was loaded asynchronously
 *
 * @return true if the asset was successfully loaded
 */
bool MtlLoader::read(const std::shared_ptr<JsonValue>& json,
                     LoaderCallback callback, bool async) {
    std::string key = json->key();
    if (_assets.find(key) != _assets.end() || _queue.find(key) != _queue.end()) {
        return false;
    }

    bool success = true;
    if (_loader == nullptr || !async) {
        enqueue(key);
        std::shared_ptr<MaterialLib> lib = preload(json);
        success = materialize(key,lib,callback);
    } else {
        _loader->addTask([=,this](void) {
            this->enqueue(key);
            std::shared_ptr<MaterialLib> lib = this->preload(json);
            Application::get()->schedule([=,this](void){
                this->materialize(key,lib,callback);
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
bool MtlLoader::purgeJson(const std::shared_ptr<JsonValue>& json) {
    std::string key = json->key();
 
    auto it = _assets.find(key);
    if (it != _assets.end()) {
        _assets.erase(it);
        return true;
    }
    
    auto jt = _libraries.find(key);
    if (jt != _libraries.end()) {
        auto lib = jt->second;
        for(auto k : lib->matinfos) {
            std::string subkey = key+"."+k.first;
            it = _assets.find(subkey);
            if (it != _assets.end()) {
                _assets.erase(it);
            }
        }
        _libraries.erase(jt);
        return true;
    }

    return false;
}

/**
 * Returns a material library associated with the given key
 *
 * While this loader is designed to only load materials, they are typically
 * packaged in material libraries. This method exposes that organization.
 * This method will return nullptr if no library for the given key has
 * finished loading.
 *
 * @param key   The key to identify the material library
 *
 * @return a material library associated with the given key
 */
std::shared_ptr<MaterialLib> MtlLoader::getLibrary(std::string key) {
    auto it = _libraries.find(key);
    return it != _libraries.end() ? it->second : nullptr;
}
