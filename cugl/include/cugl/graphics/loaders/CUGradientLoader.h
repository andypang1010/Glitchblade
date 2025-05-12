//
//  CUGradientLoader.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a specific implementation of the Loader class to load
//  gradients. This loader is not strictly necessary, as gradients do not
//  require anything more than simple JSON parsing. However, it can be nice
//  to have a logical name to refer to a gradient.
//
//  This class uses our standard shared-pointer architecture.
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
#ifndef __CU_GRADIENT_LOADER_H__
#define __CU_GRADIENT_LOADER_H__
#include <cugl/core/assets/CULoader.h>
#include <cugl/graphics/CUGradient.h>

namespace cugl {

    /**
     * The classes and functions needed to construct a graphics pipeline.
     *
     * Initially these were part of the core CUGL module (everyone wants graphics,
     * right). However, after student demand for a headless option that did not
     * have so many OpenGL dependencies, this was factored out.
     */
    namespace graphics {

/**
 * This class is a specific implementation of Loader<Gradient>
 *
 * This asset loader allows us to allocate color gradients from a JSON
 * specification. The format of this json data is the same as that required
 * by {@link Gradient#allocWithData}.
 *
 * Technically, this loader uses the two phase loading system that all other
 * loaders do. However, gradients can be full materialized off the main thread,
 * so this is not a major issue.
 *
 * As with all of our loaders, this loader is designed to be attached to an
 * asset manager. Use the method {@link getHook()} to get the appropriate
 * pointer for attaching the loader.
 */
class GradientLoader : public Loader<Gradient> {
private:
    /** This macro disables the copy constructor (not allowed on assets) */
    CU_DISALLOW_COPY_AND_ASSIGN(GradientLoader);
    
protected:
#pragma mark Asset Loading
    /**
     * Loads the portion of this asset that is safe to load outside the main thread.
     *
     * In the case of the gradient, this is the entire asset. The only thing
     * that materialize does is assign the key, ensuring that key access is
     * safe on the main thread.
     *
     * @param key       The key to access the asset after loading
     * @param source    The pathname to the asset
     *
     * @return the loaded gradient
     */
    std::shared_ptr<Gradient> preload(const std::string key, const std::string source);
    
    /**
     * Loads the portion of this asset that is safe to load outside the main thread.
     *
     * In the case of the gradient, this is the entire asset. The only thing
     * that materialize does is assign the key, ensuring that key access is
     * safe on the main thread.
     *
     * @param json  The JSON entry specifying the asset
     *
     * @return the loaded gradient
     */
    std::shared_ptr<Gradient> preload(const std::shared_ptr<JsonValue>& json);
    
    /**
     * Assigns the gradient to the given key.
     *
     * This method finishes the asset loading started in {@link preload}. As
     * the gradient is fully materialized, all it does is assign the key to
     * the asset. We do this to make sure the key access is thread safe.
     *
     * This method supports an optional callback function which reports whether
     * the asset was successfully materialized.
     *
     * @param key       The key to access the asset after loading
     * @param gradient  The gradient to assign the key
     * @param callback  An optional callback for asynchronous loading
     *
     * @return true if materialization was successful
     */
    bool materialize(const std::string key, const std::shared_ptr<Gradient>& gradient,
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
     * A gradient is a JSON file, whose contents satisfy the specification of
     * {@link Gradient#allocWithData}.
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
     * This version of read provides support for JSON directories. A gradient
     * entry is a JSON object (or the name of a file containing a JSON object),
     * satisfying the specification of {@link Gradient#allocWithData}.
     *
     * @param json      The directory entry for the asset
     * @param callback  An optional callback for asynchronous loading
     * @param async     Whether the asset was loaded asynchronously
     *
     * @return true if the asset was successfully loaded
     */
    virtual bool read(const std::shared_ptr<JsonValue>& json,
                      LoaderCallback callback, bool async) override;
    
public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new, uninitialized gradient loader
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate a loader on
     * the heap, use one of the static constructors instead.
     */
    GradientLoader() : Loader<Gradient>() { _jsonKey = "gradients"; _priority = 0; }
    
    /**
     * Disposes all resources and assets of this loader
     *
     * Once the loader is disposed, any attempts to load a new asset will
     * fail. You must reinitialize the loader to begin loading assets again.
     */
    void dispose() override {
        _jsonKey  = "";
        _priority = 0;
        _assets.clear();
        _queue.clear();
        _loader = nullptr;
    }
    
    /**
     * Returns a newly allocated gradient loader.
     *
     * This method bootstraps the loader with any initial resources that it
     * needs to load assets. In particular, the OpenGL context must be active.
     * Attempts to load an asset before this method is called will fail.
     *
     * This loader will have no associated threads. That means any asynchronous
     * loading will fail until a thread is provided via {@link setThreadPool}.
     *
     * @return a newly allocated gradient loader.
     */
    static std::shared_ptr<GradientLoader> alloc() {
        std::shared_ptr<GradientLoader> result = std::make_shared<GradientLoader>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated gradient loader.
     *
     * This method bootstraps the loader with any initial resources that it
     * needs to load assets. In particular, the OpenGL context must be active.
     * Attempts to load an asset before this method is called will fail.
     *
     * @param threads   The thread pool for asynchronous loading
     *
     * @return a newly allocated gradient loader.
     */
    static std::shared_ptr<GradientLoader> alloc(const std::shared_ptr<ThreadPool>& threads) {
        std::shared_ptr<GradientLoader> result = std::make_shared<GradientLoader>();
        return (result->init(threads) ? result : nullptr);
    }
    
};

    }

}

#endif /* __CU_GRADIENT_LOADER_H__ */
