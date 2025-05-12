//
//  CUScene.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a basic scene in the application. Most
//  applications are composed of scenes, with some way of switching between
//  them. Historically, CUGL was composed primarily of 2d scenes which are
//  supported by the Scene package. However, this is not required, and so
//  this base implementation is 2d/3d agnostic.
//
//  That means of course that this base scene has no associated scene graph.
//  That is the responsibility of any subclasses.
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
#ifndef __CU_SCENE_H__
#define __CU_SCENE_H__
#include <string>
#include <memory>
#include <cugl/core/math/cu_math.h>

namespace cugl {
/**
 * This class provides the root class of an application scene.
 *
 * Most applications are composed of one or more scenes, with the application
 * root responsible for switching between them. Typically these scenes have
 * associated scene graphs. However scene graphs are specific to a rendering
 * pipeline. Therefore it is useful to have a base class that is pipeline
 * agnostic.
 *
 * Scenes are designed to fill the entire screen. However, to better support
 * aspect ratio independent design, many screens are height or width locked.
 * In each case that means that this particular dimension is fixed to a
 * particular size. This makes it possible to layout UI elements with Figma
 * or other tools.
 *
 * While Scenes are designed to fill the entire screen, it is possible to
 * render multiple scenes on top of each other. This can be useful for
 * transition effects or for UI layers where you want the camera to be
 * separate from the main game layer.
 *
 * This class has no static allocators as it is an abstract class intended
 * to be subclassed
 */
class Scene {
#pragma mark Values
protected:
    /** The camera for this scene */
    std::shared_ptr<Camera> _camera;
    /** The name of this scene */
    std::string _name;
    /** The scene size (for initializing the camera). */
    Size _size;
    
    /** Whether or note this scene is still active */
    bool _active;
    
#pragma mark -
#pragma mark Constructors
public:
    /**
     * Creates a new degenerate Scene on the stack.
     *
     * The scene has no view size and must be initialized.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    Scene() :
    _camera(nullptr),
    _active(false),
    _name("") {
    }
    
    /**
     * Deletes this scene, disposing all resources
     */
    ~Scene() { dispose(); }
    
    /**
     * Disposes all of the resources used by this scene.
     *
     * A disposed Scene can be safely reinitialized. Any children owned by this
     * scene will be released. They will be deleted if no other object owns them.
     */
    virtual void dispose() {
        _camera = nullptr;
        _active = false;
        _size.set(0,0);
        _name = "";
    }

    /**
     * Initializes a Scene to fill the entire screen.
     *
     * @return true if initialization was successful.
     */
    virtual bool init();

    /**
     * Initializes a Scene with the given size hint.
     *
     * Scenes are designed to fill the entire screen. If you want a scene that
     * is only part of the screen, that should be implemented with a specific
     * scene graph. However, the size of that screen can vary from device to
     * device. To make scene design easier, designs are typically locked to a
     * dimension: width or height.
     *
     * This is the purpose of the size hint. If either of the values of hint
     * are non-zero, then the scene will lock that dimension to that particular
     * size. If both are non-zero, it will choose its dimension according to the
     * device orientation. Landscape will be height, while portrait will pick
     * width. Devices with no orientation will always priortize height over
     * width.
     *
     * @param hint      The size hint
     *
     * @return true if initialization was successful.
     */
    virtual bool initWithHint(Size hint);
    
    /**
     * Initializes a Scene with the given size hint.
     *
     * Scenes are designed to fill the entire screen. If you want a scene that
     * is only part of the screen, that should be implemented with a specific
     * scene graph. However, the size of that screen can vary from device to
     * device. To make scene design easier, designs are typically locked to a
     * dimension: width or height.
     *
     * This is the purpose of the size hint. If either of the values of hint
     * are non-zero, then the scene will lock that dimension to that particular
     * size. If both are non-zero, it will choose its dimension according to the
     * device orientation. Landscape will be height, while portrait will pick
     * width. Devices with no orientation will always priortize height over
     * width.
     *
     * @param width     The width size hint
     * @param height    The height size hint
     *
     * @return true if initialization was successful.
     */
    virtual bool initWithHint(float width, float height) {
        return initWithHint(Size(width,height));
    }
    
#pragma mark -
#pragma mark Attributes
    /**
     * Returns a string that is used to identify the scene.
     *
     * @return a string that is used to identify the scene.
     */
    const std::string getName() const { return _name; }
    
    /**
     * Returns the string that is used to identify the scene.
     *
     * @param name  A string that is used to identify the scene.
     */
    void setName(const std::string name) { _name = name; }
    
    /**
     * Returns the camera for this scene.
     *
     * @return the camera for this scene.
     */
    std::shared_ptr<Camera> getCamera() { return _camera; }
    
    /**
     * Returns the camera for this scene.
     *
     * @return the camera for this scene.
     */
    const std::shared_ptr<Camera> getCamera() const { return _camera; }
    
    /**
     * Returns a string representation of this scene for debugging purposes.
     *
     * If verbose is true, the string will include class information.  This
     * allows us to unambiguously identify the class.
     *
     * @param verbose Whether to include class information
     *
     * @return a string representation of this scene for debuggging purposes.
     */
    virtual std::string toString(bool verbose = false) const;
    
    /** Cast from a Scene to a string. */
    operator std::string() const { return toString(); }
    
#pragma mark -
#pragma mark View Size
    /**
     * Returns the viewport size of this Scene.
     *
     * @return the viewport size of this Scene.
     */
    const Size getSize() const {
        return _camera == nullptr ? _size : _camera->getViewport().size;
    }
    
    /**
     * Returns the viewport of this Scene.
     *
     * @return the viewport of this Scene.
     */
    const Rect getBounds() const {
        return _camera == nullptr ? Rect(Vec2::ZERO,_size) : _camera->getViewport();
    }
    
    /**
     * Returns the world space equivalent of a point in screen coordinates.
     *
     * Ideally, window space and screen space would be the same space.  They
     * are both defined by the viewport and have the same offset and dimension.
     * However, screen coordinates have the origin in the top left while window
     * coordinates have the origin in the bottom left.
     *
     * This method is important for converting event coordinates (such as a
     * mouse click) to world coordinates. Note that not all scenes support
     * the third dimension.
     *
     * @param screenCoords  The point in screen coordinates
     *
     * @return the world space equivalent of a point in screen coordinates.
     */
    Vec3 screenToWorldCoords(const Vec2 screenCoords) const {
        return _camera == nullptr ? (Vec3)screenCoords : _camera->screenToWorldCoords(screenCoords);
    }
    
    /**
     * Returns the screen space equivalent of a point in world coordinates.
     *
     * Ideally, window space and screen space would be the same space.  They
     * are both defined by the viewport and have the same offset and dimension.
     * However, screen coordinates have the origin in the top left while window
     * coordinates have the origin in the bottom left.
     *
     * This method is important for converting world coordinates to event
     * coordinates (such as a mouse click). Note that not all scenes support
     * the third dimension.
     *
     * @param worldCoords   The point in wprld coordinates
     *
     * @return the screen space equivalent of a point in world coordinates.
     */
    Vec2 worldToScreenCoords(const Vec3 worldCoords) const {
        return _camera == nullptr ? (Vec2)worldCoords : _camera->worldToScreenCoords(worldCoords);
    }

    
#pragma mark -
#pragma mark Scene Logic
    /**
     * Returns true if the scene is currently active
     *
     * @return true if the scene is currently active
     */
    bool isActive( ) const { return _active; }
    
    /**
     * Sets whether the scene is currently active
     *
     * @param value whether the scene is currently active
     */
    virtual void setActive(bool value) { _active = value; }
    
    /**
     * The method called to update the scene.
     *
     * This method should be overridden with the specific scene logic.
     *
     * @param timestep  The amount of time (in seconds) since the last frame
     */
    virtual void update(float timestep) {}
    
    /**
     * Resets the status of the scene to its original configuration.
     */
    virtual void reset() {}
    
    /**
     * Renders this screen to the scene.
     *
     * This method make no assumption about the graphics pipeline. Each scene
     * is capable of having its own pipeline.
     */
    virtual void render() {}
    
};

}
#endif /* __CU_SCENE_2_H__ */
