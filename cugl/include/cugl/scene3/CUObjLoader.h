//
//  CUObjLoader.h
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
//  Version: 11/15/25
//
#ifndef __CU_OBJ_LOADER_H__
#define __CU_OBJ_LOADER_H__
#include <cugl/core/assets/CULoader.h>
#include <cugl/graphics/loaders/CUTextureLoader.h>
#include <cugl/scene3/CUObjModel.h>
#include <cugl/scene3/CUObjParser.h>

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

/**
 * This class is a specific implementation of Loader<ObjModel>
 *
 * This asset loader allows us to allocate OBJ models from the associated OBJ,
 * MTL, and {@link graphics::Texture} files. While most OBJ files package their
 * MTL and texture files in the same directory as the OBJ file, that is not
 * required. Using the JSON specification, it is possible to put these in
 * different directories.
 *
 * Note that an OBJ loader is only responsible for loading OBJ and MTL files.
 * It is NOT responsible for loading {@link graphics::Texture} files. If a MTL 
 * uses any textures, there should be a {@link graphics::TextureLoader}
 * associated with the {@link AssetManager} for loading these textures. If
 * there is no such loader, then textures will fail to load.
 *
 * Note that this implementation uses a two phase loading system. First, it
 * loads as much of the asset as possible without using OpenGL. This allows
 * us to load the model in a separate thread.  It then finishes off the
 * remainder of asset loading using {@link Application#schedule}. This is a
 * good template for asset loaders in general.
 *
 * As with all of our loaders, this loader is designed to be attached to an
 * asset manager. Use the method {@link getHook()} to get the appropriate
 * pointer for attaching the loader.
 */
class ObjLoader : public Loader<ObjModel> {
private:
    /** This macro disables the copy constructor (not allowed on assets) */
    CU_DISALLOW_COPY_AND_ASSIGN(ObjLoader);
    
protected:
    /** Integrated parser for finding objects and materials */
    std::shared_ptr<ObjParser> _parser;
    
    /** The materials parsed for each model asset  */
    std::unordered_map<std::string, std::unordered_map<std::string,std::shared_ptr<Material>>> _materials;
    /** The materials AST for each model asset  */
    std::unordered_map<std::string, std::unordered_map<std::string,std::shared_ptr<MaterialInfo>>> _matast;
    /** Any textures that must be loaded on the fly */
    std::unordered_map<std::string,SDL_Surface*> _surfaces;
    
    /** The MTL libraries assocated with each model asset */
    std::unordered_map<std::string, std::unordered_map<std::string,std::shared_ptr<MaterialLib>>> _libraries;
    

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
    void preloadLibrary(const std::shared_ptr<MaterialLib>& lib,
                            const std::shared_ptr<MtlLoader>& loader);
    
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
    void materializeLibrary(const std::shared_ptr<MaterialLib>& lib,
                            const std::shared_ptr<MtlLoader>& loader);
    
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
    std::shared_ptr<ObjModel> preload(const std::string key, const std::string source);
    
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
     * @param json  The JSON entry specifying the asset
     *
     * @return the incomplete OBJ model
     */
    std::shared_ptr<ObjModel> preload(const std::shared_ptr<JsonValue>& json);
    
    /**
     * Creates an OpenGL buffer for the model, and assigns it the given key.
     *
     * This method finishes the asset loading started in {@link preload}. This
     * step is not safe to be done in a separate thread. Instead, it takes
     * place in the main CUGL thread via {@link Application#schedule}.
     *
     * Any associated MTL materials will be materialized at this time. This
     * method will only work if all of the textures and MTL files associated
     * with this OBJ file are in the same directory as it.
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
    bool materialize(const std::string key, const std::shared_ptr<ObjModel>& model,
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
     * The value of MTL entries can either be a key of a previously loaded
     * MTL library, or it can be a JSON entry of the type supported by
     * {@link MtlLoader}. In the case of the latter, this will cause the
     * the material to be loaded implicitly.
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
     * Creates a new, uninitialized OBJ loader
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a loader on
     * the heap, use one of the static constructors instead.
     */
    ObjLoader() : Loader<ObjModel>() { _jsonKey = "objs"; _priority = 2; }
    
    /**
     * Disposes all resources and assets of this loader
     *
     * Any assets loaded by this object will be immediately released by the
     * loader. However, a texture may still be available if it is referenced
     * by another smart pointer.  OpenGL will only release a OBJ mesh once all
     * smart pointer attached to the asset are null.
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
     * Initializes a new OBJ loader.
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
     * Initializes a new OBJ loader.
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
     * Returns a newly allocated OBJ loader.
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
    static std::shared_ptr<ObjLoader> alloc() {
        std::shared_ptr<ObjLoader> result = std::make_shared<ObjLoader>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated OBJ loader.
     *
     * This method bootstraps the loader with any initial resources that it
     * needs to load assets. In particular, the OpenGL context must be active.
     * Attempts to load an asset before this method is called will fail.
     *
     * @param threads   The thread pool for asynchronous loading
     *
     * @return a newly allocated OBJ loader.
     */
    static std::shared_ptr<ObjLoader> alloc(const std::shared_ptr<ThreadPool>& threads) {
        std::shared_ptr<ObjLoader> result = std::make_shared<ObjLoader>();
        return (result->init(threads) ? result : nullptr);
    }
    
};

    }
}


#endif /* __CU_OBJ_LOADER_H__ */
