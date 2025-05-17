//
//  CUMtlLoader.h
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
#ifndef __CU_MTL_LOADER_H__
#define __CU_MTL_LOADER_H__
#include <cugl/core/assets/CULoader.h>
#include <cugl/graphics/loaders/CUTextureLoader.h>
#include <cugl/scene3/CUMaterial.h>
#include <cugl/scene3/CUObjParser.h>
#include <unordered_map>
#include <unordered_set>

namespace cugl {

    /**
     * The classes to construct a 3-d scene graph.
     *
     * Unlike the scene2 package, the 3-d scene graph classes are quite limited.
     * We only have support for OBJ/MTL files, as well as simple billboarding.
     * There is no support for bone animation or physically based rendering.
     *
     * The reason for this limitation is because this is a student training
     * engine, and we often like to task students at adding those features.
     * In addition, unlike our 2-d scene graph with Figma support, there are
     * a lot of third party libraries out there that handle rendering better
     * for 3-d scenes.
     */
    namespace scene3 {

// Forward declaration
class ObjLoader;

    
/**
 * This class is a specific implementation of Loader<Material>
 *
 * This asset loader allows us to allocate materials from the associated
 * MTL and {@link graphics::Texture} files. While most MTL files package their
 * texture files in the same directory, that is not required. Using the JSON
 * specification, it is possible to put these in different directories.
 *
 * Note that an MTL loader is only responsible for materials. It is NOT
 * responsible for loading {@link graphics::Texture} files. If a MTL uses any
 * textures, there should be a {@link graphics::TextureLoader} associated with
 * the {@link AssetManager} for loading these textures. If there is no such
 * loader, then textures will fail to load.
 *
 * In addition, an MTL file often has multiple materials inside of it. See
 * the description of {@link #read} for how this loader handles MTL files with
 * multiple materials.
 *
 * This implementation uses a two phase loading system. First, it loads as much
 * of the asset as possible without using OpenGL. This allows us to load the
 * model in a separate thread. It then finishes off the remainder of asset
 * loading using {@link Application#schedule}. This is a good template for
 * asset loaders in general.
 *
 * As with all of our loaders, this loader is designed to be attached to an
 * asset manager. Use the method {@link getHook()} to get the appropriate
 * pointer for attaching the loader.
 */
class MtlLoader : public Loader<Material> {
private:
    /** This macro disables the copy constructor (not allowed on assets) */
    CU_DISALLOW_COPY_AND_ASSIGN(MtlLoader);
    
protected:
    /** Integrated parser for finding objects and materials */
    std::shared_ptr<ObjParser> _parser;
    
    /** Any textures that must be loaded on the fly */
    std::unordered_map<std::string,SDL_Surface*> _surfaces;
    
    /** Keep track of material libraries */
    std::unordered_map<std::string,std::shared_ptr<MaterialLib>> _libraries;
    
    /** These two loaders go together */
    friend class ObjLoader;

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
    void preloadTexture(const std::shared_ptr<TextureInfo>& info,
                        const std::shared_ptr<graphics::TextureLoader>& loader);
    
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
    std::shared_ptr<graphics::Texture> materializeTexture(const std::shared_ptr<TextureInfo>& info,
                                                          const std::shared_ptr<graphics::TextureLoader>& loader);
    
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
    std::shared_ptr<MaterialLib> preload(const std::string key, const std::string source);
    
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
    std::shared_ptr<MaterialLib> preload(const std::shared_ptr<JsonValue>& json);
    
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
    bool materialize(const std::string key, const std::shared_ptr<MaterialLib>& lib,
                     LoaderCallback callback);
    
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
    virtual bool read(const std::string key, const std::string source,
                      LoaderCallback callback, bool async) override;
    
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
     * An MTL entries can have several forms. In the simplest case, an MTL
     * entry can just be a string. In that case, it is assumed to be a path to
     * a MTL file. If that MTL file has any associated textures, they are
     * loaded implicitly (and they must be in the same directory as the MTL
     * file).
     *
     * Note that a MTL file can have multiple materials inside of it. For that
     * reason, this asset loader associates each material with the key that
     * is the key of the MTL file, followed by a period and the name of the
     * material. So if the MTL file with key "fire" has two materials "bright"
     * and "dim", they would be refered to as "fire.bright" and "fire.dim".
     *
     * If a MTL entry is a JSON object, then it typically has the following
     * values:
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
    virtual bool read(const std::shared_ptr<JsonValue>& json,
                      LoaderCallback callback, bool async) override;
    
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
    virtual bool purgeJson(const std::shared_ptr<JsonValue>& json) override;
    
public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new, uninitialized MTL loader
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a loader on
     * the heap, use one of the static constructors instead.
     */
    MtlLoader() : Loader<Material>() { _jsonKey = "mtls"; _priority = 1; }
    
    /**
     * Disposes all resources and assets of this loader
     *
     * Any assets loaded by this object will be immediately released by the
     * loader. However, a texture may still be available if it is referenced
     * by another smart pointer.
     *
     * Once the loader is disposed, any attempts to load a new asset will
     * fail. You must reinitialize the loader to begin loading assets again.
     */
    void dispose() override {
        _parser = nullptr;
        _jsonKey  = "";
        _priority = 0;
        _assets.clear();
        _surfaces.clear();
        _queue.clear();
        _loader = nullptr;
    }
    
    /**
     * Initializes a new MTL loader.
     *
     * This method bootstraps the loader with any initial resources that it
     * needs to load assets. Attempts to load an asset before this method is
     * called will fail.
     *
     * This loader will have no associated threads. That means any asynchronous
     * loading will fail until a thread is provided via {@link setThreadPool}.
     *
     * @return true if the asset loader was initialized successfully
     */
    bool init() override {
        return init(nullptr);
    }
    
    /**
     * Initializes a new MTL loader.
     *
     * This method bootstraps the loader with any initial resources that it
     * needs to load assets. Attempts to load an asset before this method is
     * called will fail.
     *
     * @param threads   The thread pool for asynchronous loading support
     *
     * @return true if the asset loader was initialized successfully
     */
    bool init(const std::shared_ptr<ThreadPool>& threads) override {
        _loader=threads;
        _parser = std::make_shared<ObjParser>();
        return _parser != nullptr;
    }
    
    /**
     * Returns a newly allocated MTL loader.
     *
     * This method bootstraps the loader with any initial resources that it
     * needs to load assets. In particular, the OpenGL context must be active.
     * Attempts to load an asset before this method is called will fail.
     *
     * This loader will have no associated threads. That means any asynchronous
     * loading will fail until a thread is provided via {@link setThreadPool}.
     *
     * @return a newly allocated OBJ loader.
     */
    static std::shared_ptr<MtlLoader> alloc() {
        std::shared_ptr<MtlLoader> result = std::make_shared<MtlLoader>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated MTL loader.
     *
     * This method bootstraps the loader with any initial resources that it
     * needs to load assets. In particular, the OpenGL context must be active.
     * Attempts to load an asset before this method is called will fail.
     *
     * @param threads   The thread pool for asynchronous loading
     *
     * @return a newly allocated OBJ loader.
     */
    static std::shared_ptr<MtlLoader> alloc(const std::shared_ptr<ThreadPool>& threads) {
        std::shared_ptr<MtlLoader> result = std::make_shared<MtlLoader>();
        return (result->init(threads) ? result : nullptr);
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
    std::shared_ptr<MaterialLib> getLibrary(std::string key);
};

	}
}

#endif /* __CU_MTL_LOADER_H__ */
