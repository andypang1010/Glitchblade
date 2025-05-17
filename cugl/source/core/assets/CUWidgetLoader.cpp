//
//  CUJsonLoader.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides a specific implementation of the Loader class to load
//  (non-directory) json assets. It is essentially a wrapper around JsonReader
//  that allows it to be used with AssetManager.
//
//  As with all of our loaders, this loader is designed to be attached to an
//  asset manager. In addition, this class uses our standard shared-pointer
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
//  Author: Graham Rutledge, Walker White
//  Version: 7/3/24 (CUGL 3.0 reorganization)
//
#include <cugl/core/assets/CUWidgetLoader.h>
#include <cugl/core/io/CUJsonReader.h>
#include <cugl/core/util/CUFiletools.h>
#include <cugl/core/CUApplication.h>

using namespace cugl;

/** What the source name is if we do not know it */
#define UNKNOWN_SOURCE  "<unknown>"

/**
 * Finishes loading the widget file, cleaning up the wait queues.
 *
 * Allocating a widget asset can be done safely in a separate thread.
 * Hence this method is really just an internal helper for convenience.
 *
 * This method supports an optional callback function which reports whether
 * the asset was successfully materialized.
 *
 * @param key       The key to access the asset after loading
 * @param widget    The widget asset fully loaded
 * @param callback  An optional callback for asynchronous loading
 *
 * @return true if materialization was successful
 */
bool WidgetLoader::materialize(const std::string key, const std::shared_ptr<WidgetValue>& widget,
                              LoaderCallback callback) {
    bool success = false;
    if (widget != nullptr) {
        _assets[key] = widget;
		std::shared_ptr<JsonValue> json = widget->getJson()->get("dependencies");
		if (json != nullptr) {
			for (int ii = 0; ii < json->size(); ii++) {
				std::shared_ptr<JsonValue> child = json->get(ii);
				load(child);
			}
		}
        success = true;
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
 * This method will split the loading across the {@link WidgetValue#alloc} and
 * the internal {@link materialize} method. This ensures that asynchronous
 * loading is safe.
 *  
 * @param key       The key to access the asset after loading
 * @param source    The pathname to the asset
 * @param callback  An optional callback for asynchronous loading
 * @param async     Whether the asset was loaded asynchronously
 *  
 * @return true if the asset was successfully loaded
 */
bool WidgetLoader::read(const std::string key, const std::string source, LoaderCallback callback, bool async) {
    if (_assets.find(key) != _assets.end() || _queue.find(key) != _queue.end()) {
        return false;
    }
    
    // Make sure we reference the asset directory
    bool absolute = cugl::filetool::is_absolute(source);
    CUAssertLog(!absolute, "This loader does not accept absolute paths for assets");

    std::string root = Application::get()->getAssetDirectory();
    std::string path = root+source;
    
    bool success = false;
    if (_loader == nullptr || !async) {
        enqueue(key);
        std::shared_ptr<JsonReader> reader = JsonReader::allocWithAsset(path);
        std::shared_ptr<JsonValue> json = (reader == nullptr ? nullptr : reader->readJson());
		std::shared_ptr<WidgetValue> widget = WidgetValue::alloc(json);
        success = materialize(key,widget,callback);
    } else {
        _loader->addTask([=,this](void) {
            this->enqueue(key);
            std::shared_ptr<JsonReader> reader = JsonReader::allocWithAsset(path);
            std::shared_ptr<JsonValue> json = (reader == nullptr ? nullptr : reader->readJson());
			std::shared_ptr<WidgetValue> widget = WidgetValue::alloc(json);
            Application::get()->schedule([=,this](void) {
                this->materialize(key,widget,callback);
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
 * This method will split the loading across the {@link WidgetValue#alloc} and
 * the internal {@link materialize} method. This ensures that asynchronous
 * loading is safe.
 *
 * This version of read provides support for JSON directories. A widget
 * directory entry is comprised of a single named string attribute.
 *
 * @param json      The directory entry for the asset
 * @param callback  An optional callback for asynchronous loading
 * @param async     Whether the asset was loaded asynchronously
 *
 * @return true if the asset was successfully loaded
 */
bool WidgetLoader::read(const std::shared_ptr<JsonValue>& json, LoaderCallback callback, bool async) {
    std::string key = json->key();
    if (_assets.find(key) != _assets.end() || _queue.find(key) != _queue.end()) {
        return false;
    }
    std::string source = json->asString(UNKNOWN_SOURCE);
    
    bool success = false;
    if (_loader == nullptr || !async) {
        enqueue(key);
        std::shared_ptr<JsonReader> reader = JsonReader::allocWithAsset(source);
        std::shared_ptr<JsonValue> json = (reader == nullptr ? nullptr : reader->readJson());
		std::shared_ptr<WidgetValue> widget = WidgetValue::alloc(json);
        success = materialize(key,widget,callback);
    } else {
        _loader->addTask([=,this](void) {
            this->enqueue(key);
            std::shared_ptr<JsonReader> reader = JsonReader::allocWithAsset(source);
            std::shared_ptr<JsonValue> json = (reader == nullptr ? nullptr : reader->readJson());
			std::shared_ptr<WidgetValue> widget = WidgetValue::alloc(json);
            Application::get()->schedule([=,this](void) {
                this->materialize(key,widget,callback);
                return false;
            });
        });
    }
    
    return success;
}

