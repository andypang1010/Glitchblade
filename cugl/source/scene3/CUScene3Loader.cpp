//
//  CUScene3Loader.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides a specific implementation of the Loader class to load
//  a 3d scene graph. Scene graphs are always specified as a JSON tree. This
//  loader somewhat experimental, as the language is still evolving. In
//  particular, we continue to add new node types.
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
#include <cugl/scene3/cu_scene3.h>
#include <cugl/core/assets/CUAssetManager.h>
#include <cugl/core/assets/CUWidgetValue.h>
#include <cugl/core/io/CUJsonReader.h>
#include <cugl/core/util/CUStringTools.h>
#include <cugl/core/util/CUFiletools.h>
#include <cugl/core/CUApplication.h>
#include <algorithm>
#include <locale>

using namespace cugl;
using namespace cugl::scene3;

/** If the type is unknown */
#define UNKNOWN_STR  "<unknown>"

/**
 * Initializes a new asset loader.
 *
 * This method bootstraps the loader with any initial resources that it
 * needs to load assets. Attempts to load an asset before this method is
 * called will fail.
 *
 * This method is abstract and should be overridden in the specific
 * implementation for each asset.
 *
 * @param threads   The thread pool for asynchronous loading support
 *
 * @return true if the asset loader was initialized successfully
 */
bool Scene3Loader::init(const std::shared_ptr<ThreadPool>& threads) {
    _loader=threads;
    
    // Define the supported types
    _types["node"] = Widget::NODE;
    _types["obj"] = Widget::OBJMODEL;
    _types["model"] = Widget::OBJMODEL;
    _types["billboard"] = Widget::BILLBOARD;
    _types["particle"] = Widget::PARTICLE;
    
    return true;
}

/**
 * Recursively builds the scene from the given JSON tree.
 *
 * This method allows us to maximize the asynchronous creation of scenes.
 * The key is assigned as the name of the root Node of the scene.
 *
 * The JSON tree should be a tree of widget objects, where each widget
 * object has the following attribute values:
 *
 *      "type":     The node type (a Node or any subclass)
 *      "data":     Data (images, labels) that define the widget. This
 *                  JSON object has a node-specific format.
 *      "children": Any child Nodes of this one. This JSON object has a
 *                  named attribute for each child.
 *
 * With the exception of "type", all of these attributes are JSON objects.
 *
 * @param key       The key to access the scene after loading
 * @param json      The JSON object defining the scene
 *
 * @return the root scene node
 */
std::shared_ptr<scene3::SceneNode> Scene3Loader::build(const std::string key,
                                                       const std::shared_ptr<JsonValue>& json) const {
    std::string type = json->getString("type",UNKNOWN_STR);
    auto it = _types.find(cugl::strtool::tolower(type));
    if (it == _types.end()) {
        return nullptr;
    }
    
    std::shared_ptr<JsonValue> data = json->get("data");
    std::shared_ptr<scene3::SceneNode> node = nullptr;
    switch (it->second) {
        case Widget::NODE:
            node = scene3::SceneNode::allocWithData(_manager,data);
            break;
        case Widget::OBJMODEL:
            node = scene3::ObjNode::allocWithData(_manager,data);
            break;
        case Widget::BILLBOARD:
            node = scene3::BillboardNode::allocWithData(_manager,data);
            break;
        case Widget::PARTICLE:
            node = scene3::ParticleNode::allocWithData(_manager,data);
            break;
        case Widget::UNKNOWN:
            break;
    }
    
    if (node == nullptr) {
        return nullptr;
    }
    
    // Recurse over the children
    std::shared_ptr<JsonValue> children = json->get("children");
    if (children != nullptr) {
        for (int ii = 0; ii < children->size(); ii++) {
            std::shared_ptr<JsonValue> item = children->get(ii);
            std::string key = item->key();
            if (key != "comment") {
                std::shared_ptr<scene3::SceneNode> kid = build(key, item);
                if (kid != nullptr) {
                    node->addChild(kid);
                }
            }
        }
    }
    
    node->setName(key);
    return node;
}

/**
 * Records the given Node with this loader, so that it may be unloaded later.
 *
 * This method finishes the asset loading started in {@link preload}. This
 * step is not safe to be done in a separate thread, as it accesses the
 * main asset table. Therefore, it takes place in the main CUGL thread
 * via {@link Application#schedule}. The scene is stored using the name
 * of the root Node as a key.
 *
 * This method supports an optional callback function which reports whether
 * the asset was successfully materialized.
 *
 * @param node      The scene asset
 * @param callback  An optional callback for asynchronous loading
 *
 * @return true if materialization was successful
 */
bool Scene3Loader::materialize(const std::shared_ptr<scene3::SceneNode>& node, LoaderCallback callback) {
    bool success = false;
    
    std::string key = "";
    if (node != nullptr) {
        key = node->getName();
        success = attach(key, node);
    }

    if (callback != nullptr) {
        callback(key,success);
    }
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
 * @param key       The key to access the asset after loading
 * @param source    The pathname to the asset
 * @param size      The font size (overriding the default)
 * @param callback  An optional callback for asynchronous loading
 * @param async     Whether the asset was loaded asynchronously
 *
 * @return true if the asset was successfully loaded
 */
bool Scene3Loader::read(const std::string key, const std::string source,
                        LoaderCallback callback, bool async) {
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
        std::shared_ptr<scene3::SceneNode> node = build(key,json);
        success = materialize(node,callback);
    } else {
        _loader->addTask([=,this](void) {
            this->enqueue(key);
            std::shared_ptr<JsonReader> reader = JsonReader::allocWithAsset(path);
            std::shared_ptr<JsonValue> json = (reader == nullptr ? nullptr : reader->readJson());
            std::shared_ptr<scene3::SceneNode> node = build(key,json);
            Application::get()->schedule([=,this](void) {
                this->materialize(node,callback);
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
 * This method is like the traditional read method except that it assumes
 * the JSON data has already been parsed. The JSON tree should
 * be a tree of widget objects, where each widget object has the following
 * attribute values:
 *
 *      "type":     The node type (a Node or any subclass)
 *      "data":     Data (images, labels) that define the widget. This
 *                  JSON object has a node-specific format.
 *      "children": Any child Nodes of this one. This JSON object has a
 *                  named attribute for each child.
 *
 * With the exception of "type", all of these attributes are JSON objects.
 *
 * @param json      The directory entry for the asset
 * @param callback  An optional callback for asynchronous loading
 * @param async     Whether the asset was loaded asynchronously
 *
 * @return true if the asset was successfully loaded
 */
bool Scene3Loader::read(const std::shared_ptr<JsonValue>& json, LoaderCallback callback, bool async) {
    std::string key = json->key();
    if (_assets.find(key) != _assets.end() || _queue.find(key) != _queue.end()) {
        return false;
    }
    
    bool success = false;
    if (_loader == nullptr || !async) {
        enqueue(key);
        std::shared_ptr<scene3::SceneNode> node = build(key,json);
        success = materialize(node,callback);
    } else {
        _loader->addTask([=,this](void) {
            this->enqueue(key);
            std::shared_ptr<scene3::SceneNode> node = build(key,json);
            Application::get()->schedule([=,this](void) {
                this->materialize(node,callback);
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
 * This version of the method not only unloads the given {@link Scene3Node},
 * but also any children attached to it in the JSON specification.
 *
 * @param json      The directory entry for the asset
 *
 * @return true if the asset was successfully unloaded
 */
bool Scene3Loader::purgeJson(const std::shared_ptr<JsonValue>& json) {
    bool success = purgeKey(json->key());
    if (json->has("children")) {
        auto kids = json->get("children");
        for(int ii = 0; ii < kids->size(); ii++) {
            success = purgeJson(kids->get(ii)) && success;
        }
    }
    return success;
}

/**
 * Attaches all generate nodes to the asset dictionary.
 *
 * As the asset dictionary must be updated in the main thread, we do
 * not update it until the entire node tree has been materialized. This
 * method assumes that each Node is named with its asset look-up key.
 *
 * @param key       The key to access the asset after loading
 * @param node      The scene asset
 *
 * @return true if the node was successfully attached
 */
bool Scene3Loader::attach(const std::string key, const std::shared_ptr<scene3::SceneNode>& node) {
    _assets[key] = node;
    bool success = true;
    for(int ii = 0; ii < node->getChildren().size(); ii++) {
        auto item = node->getChild(ii);
        std::string local = key+"."+item->getName();
        success = attach(local, item) && success;
    }
    return success;
}



