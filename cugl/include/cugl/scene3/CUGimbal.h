//
//  CUGimbal.h
//  Cornell University Game Library (CUGL)
//
//  This module is to make it easy to navigate a 3D scene without having to
// understand coordinate systems. It is ideal for viewing OBJ models.
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
//  Version: 11/15/25
//
#ifndef __CU_GIMBAL_H__
#define __CU_GIMBAL_H__
#include <cugl/core/math/CUVec3.h>

namespace cugl {

// Forward references
class PerspectiveCamera;
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
 * This class represents a camera gimbal.
 *
 * A camera gimbal is a harness for moving a persepective (3d) camera about the
 * screen. Controls are with respect to screen space, not camera space. In this
 * space, the screen is the x-y plane with the origin in the bottom left corner
 * of the display. The z-axis is perpendicular to the screen with postive z
 * going into the screen.
 *
 * The gimbal has both an eye position and a target. The eye position is the
 * location of the camera. All rotation happens about the target. Zooming
 * adjusts the distance between the eye and the target, while panning moves
 * the eye and target together.
 */
class Gimbal {
private:
    /** The camera controlled by this gimbal */
    std::shared_ptr<PerspectiveCamera> _camera;
    
    /** The eye position */
    Vec3 _eye;
    /** The up position */
    Vec3 _up;
    /** The target (look-at) position */
    Vec3 _target;
    /** The eye distance (e.g. zoom) */
    float _distance;
    /** The minimum zoom */
    float _minZoom;
    /** The maximum zoom */
    float _maxZoom;

    /** Position snapshot for reset */
    Vec3 _origPos;
    /** Dir snapshot for reset */
    Vec3 _origDir;
    /** Up snapshot for reset */
    Vec3 _origUp;

    bool _dirty;
    
#pragma mark Constructors
public:
    /**
     * Creates a new degenerate batch on the stack.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    Gimbal(): _distance(0), _minZoom(0), _maxZoom(0), _dirty(false) {}
    
    /**
     * Deletes this gimbal, disposing all resources
     */
    ~Gimbal() { dispose(); }
    

    /**
     * Disposes all of the resources used by this gimbal.
     *
     * A disposed batch can be safely reinitialized.
     */
    void dispose();
    
    /**
     * Initializes a gimbal with no camera.
     *
     * You should call {@link #setCamera} before using the gimbal.
     *
     * @return true if initialization was successful.
     */
    bool init();

    /**
     * Initializes a gimbal for the specified camera
     *
     * @param camera    The camera to control
     *
     * @return true if initialization was successful.
     */
    bool initWithCamera(const std::shared_ptr<PerspectiveCamera>& camera);
    
    /**
     * Returns a newly allocated gimbal with no camera.
     *
     * You should call {@link #setCamera} before using the gimbal.
     *
     * @return a newly allocated gimbal with no camera.
     */
    static std::shared_ptr<Gimbal> alloc() {
        std::shared_ptr<Gimbal> result = std::make_shared<Gimbal>();
        return (result->init() ? result : nullptr);
    }

    /**
     * Returns a newly allocated gimbal for the specified camera
     *
     * @param camera    The camera to control
     *
     * @return a newly allocated gimbal for the specified camera
     */
    static std::shared_ptr<Gimbal> allocWithCamera(const std::shared_ptr<PerspectiveCamera>& camera) {
        std::shared_ptr<Gimbal> result = std::make_shared<Gimbal>();
        return (result->initWithCamera(camera) ? result : nullptr);
    }

#pragma mark Attributes
    /**
     * Returns the current camera
     *
     * This value can be null.
     *
     * @return the current camera
     */
    std::shared_ptr<PerspectiveCamera> getCamera() const { return _camera; }

    /**
     * Sets the current camera
     *
     * This value can be null.
     *
     * @param camera    the current camera
     */
    void setCamera(const std::shared_ptr<PerspectiveCamera>& camera);

    /**
     * Returns the eye position of this gimbal.
     *
     * This vector corresponds to the position of the camera. The value returned
     * is not intended to be modified. Modifying this value can have unintended
     * side effects on the next call to {@link #update}.
     *
     * @return the eye position of this gimbal.
     */
    Vec3 getEye() const { return _eye; }

    /**
     * Returns the target position of this gimbal.
     *
     * This vector corresponds to the look-at position of the camera. The value
     * returned is not intended to be modified. Modifying this value can have
     * unintended side effects on the next call to {@link #update}.
     *
     * @return the target position of this gimbal.
     */
    Vec3 getTarget() const { return _target; }
    
    /**
     * Returns the up direction of this gimbal.
     *
     * This vector corresponds to the upwards direction of the camera. The value
     * returned is not intended to be modified. Modifying this value can have
     * unintended side effects on the next call to {@link #update}.
     *
     * @return the up direction of this gimbal.
     */
    Vec3 getUp() const { return _up; }
    
    /**
     * Returns the zoom factor of this gimbal.
     * <p>
     * The zoom is the distance between the eye and the target. It defines the
     * radius of rotation about the target. It can be any value between
     * {@link #getMinimumZoom} and {@link #getMaximumZoom}.
     *
     * @return the zoom factor of this gimbal.
     */
    float getZoom() const { return _distance; }

    /**
     * Returns the minimum zoom factor allowed.
     *
     * The zoom is the distance between the eye and the target. It defines the
     * radius of rotation about the target. It can be any value between
     * {@link #getMinimumZoom} and {@link #getMaximumZoom}.
     *
     * This value should be > 0 and less than {@link #getMaximumZoom}. Other
     * values will result in undefined behavior.
     *
     * @return the minimum zoom factor allowed..
     */
    float getMinimumZoom() const { return _minZoom; }

    /**
     * Sets the minimum zoom factor allowed.
     *
     * The zoom is the distance between the eye and the target. It defines the
     * radius of rotation about the target. It can be any value between
     * {@link #getMinimumZoom} and {@link #getMaximumZoom}.
     *
     * This value should be > 0 and less than {@link #getMaximumZoom}. Other
     * values will result in undefined behavior.
     *
     * @param min   the minimum zoom factor allowed..
     */
    void setMinimumZoom(float min);

    /**
     * Returns the maximum zoom factor allowed.
     *
     * The zoom is the distance between the eye and the target. It defines the
     * radius of rotation about the target. It can be any value between
     * {@link #getMinimumZoom} and {@link #getMaximumZoom}.
     *
     * This value should be > 0 and greater than {@link #getMinimumZoom}. Other
     * values will result in undefined behavior.
     *
     * @return the maximum zoom factor allowed..
     */
    float getMaximumZoom() const { return _maxZoom; }

    /**
     * Sets the maximum zoom factor allowed.
     *
     * The zoom is the distance between the eye and the target. It defines the
     * radius of rotation about the target. It can be any value between
     * {@link #getMinimumZoom} and {@link #getMaximumZoom}.
     *
     * This value should be > 0 and less than {@link #getMaximumZoom}. Other
     * values will result in undefined behavior.
     *
     * @param max   the maximum zoom factor allowed..
     */
    void setMaximumZoom(float max);
    
#pragma mark Camera Manipulation
    /**
     * Rotates the scene about the screen y-axis.
     *
     * This has the effect of rotating the image horizontally. A positive value
     * will rotate the camera counter-clockwise (so to camera goes to the right
     * while the image goes left). A negative value will rotate clockwise.
     *
     * This method will have no effect until {@link update} is called. This will
     * allow you to make other changes to the camera before updating it.
     *
     * @param angle The rotation angle in radians
     */
    void rotateY(float angle);

    /**
     * Rotates the scene about the screen x-axis.
     *
     * This has the effect of rotating the image vertically. A positive value
     * will rotate the camera counter-clockwise (so to camera goes to down
     * while the image goes up). A negative value will rotate clockwise.
     *
     * This method will have no effect until {@link update} is called. This will
     * allow you to make other changes to the camera before updating it.
     *
     * @param angle The rotation angle in radians
     */
    void rotateX(float angle);
    
    /**
     * Rotates the scene about the screen z-axis.
     *
     * This has the effect of rotating the with the the plane of the screen,
     * with the rotation origin in the center of the display. A positive value
     * will rotate the camera counter-clockwise along the z-axis. As the z-axis
     * going into the screen, this has the effect of rotating the image in a
     * counter-clockwise direction as well. A negative value will rotate
     * clockwise.
     *
     * This method will have no effect until {@link update} is called. This will
     * allow you to make other changes to the camera before updating it.
     *
     * @param angle The rotation angle in radians
     */
    void rotateZ(float angle);

    /**
     * Adjusts the zoom by the given amount.
     *
     * The zoom is the distance between the eye and the target. It defines the
     * radius of rotation about the target. It can be any value between
     * {@link #getMinimumZoom} and {@link #getMaximumZoom}.  Values outside
     * this range are clamped to the range.
     *
     * This method will have no effect until {@link update} is called. This will
     * allow you to make other changes to the camera before updating it.
     *
     * @param dt    The amount to adjust the zoom.
     */
    void adjustZoom(float dt);

    /**
     * Pans the camera by the given vector.
     *
     * Panning takes place in screen space. So panning the x-coordinate by
     * a positive amount will move the camera to the right, and panning the
     * the y-coordinate by a positive amount will move the camera upwards.
     * Panning the z-coordinate will superficially look the same as zooming.
     * However, panning modifies both the eye and the target position, meaning
     * that the zoom length is unchanged.
     *
     * @param delta The direction to pan
     */
    void pan(Vec3 delta);
    
    /**
     * Pans the camera by the given amount.
     *
     * Panning takes place in screen space. So panning the x-coordinate by
     * a positive amount will move the camera to the right, and panning the
     * the y-coordinate by a positive amount will move the camera upwards.
     * Panning the z-coordinate will superficially look the same as zooming.
     * However, panning modifies both the eye and the target position, meaning
     * that the zoom length is unchanged.
     *
     * @param dx    The x-coordinate pan
     * @param dy    The y-coordinate pan
     * @param dz    The z-coordinate pan
     */
    void pan(float dx, float dy, float dz);

    /**
     * Resets the gimbal to its initial configuration.
     *
     * The initial configuration is the state of the camera at the time that
     * this object was constructed or {@link #setCamera} was called.
     */
    void reset();

    /**
     * Updates the camera to match the gimbal state.
     *
     * If the gimbal has been moved (rotating, zooming, or panning), this
     * method must be called for this movement to have an effect.
     */
    void update();
    
};


    }
}
#endif /* __CU_GIMBAL_H__ */
