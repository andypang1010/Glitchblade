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
#include <cugl/scene3/CUGimbal.h>
#include <cugl/core/math/CUPerspectiveCamera.h>

using namespace cugl;
using namespace cugl::scene3;

/** The default minimium zoom */
#define DEFAULT_MIN_ZOOM 0.1f
/** The default maximium zoom */
#define DEFAULT_MAX_ZOOM 10.0f

#pragma mark Traversal Helpers
/**
 * Returns a new position along the great circle defined by source and dest
 *
 * The vectors source and dest are assumed to be vectors on a sphere centered
 * at the origin. As such they define a great circle along this sphere. The
 * given angle, represents counter-clockwise movement with respect to the
 * vector (source x dest), which is the plane containing this great circle.
 *
 * This formula is takien from "Getting Around on a Sphere" by Clifford Shaffer
 * in Graphics Gems 2.
 *
 * @param source    The current location on the sphere
 * @param dest      A point on the sphere defining a great circle from source
 * @param angle     An angle represeting counterclockwise movement on the circle
 *
 * @return a new position along the great circle defined by source and dest
 */
static Vec3 traverseGreatCircle(Vec3 source, Vec3 dest, float angle) {
    Vec3 norm = dest.getCross(source);  // Switch from CC to CCW
    norm.normalize();

    Vec3 result;
    float ca = cosf(angle);
    float sa = sinf(angle);
    result.x = ((norm.x*norm.x + ca*(1-norm.x*norm.x))*source.x+
                (norm.x*norm.y*(1-ca) - norm.z*sa)*source.y+
                (norm.z*norm.x*(1-ca) + norm.y*sa)*source.z);
    result.y = ((norm.x*norm.y*(1-ca) + norm.z*sa)*source.x+
                (norm.y*norm.y + ca*(1-norm.y*norm.y))*source.y+
                (norm.y*norm.z*(1-ca) - norm.x*sa)*source.z);
    result.z = ((norm.z*norm.x*(1-ca) - norm.y*sa)*source.x+
                (norm.y*norm.z*(1-ca) + norm.x*sa)*source.y+
                (norm.z*norm.z + ca*(1-norm.z*norm.z))*source.z);

    return result;
}

// TODO: Add this to the vector class
/**
 * Returns the rotation of v about the given axis by the given angle.
 *
 * Rotation is done via the right hand rule.
 *
 * @param v     The vector to rotate
 * @param axis  The axis of rotation
 * @param angle The angle of rotation
 *
 * @return the rotation of v about the given axis by the given angle.
 */
static Vec3 rotateAbout(Vec3 v, Vec3 axis, float angle) {
    axis.normalize();
    v = v*cosf(angle) + (axis.getCross(v))*sinf(angle)+axis*(axis.dot(v)*(1-cosf(angle)));
    return v;
}


#pragma mark Constructors
/**
 * Initializes a gimbal with no camera.
 *
 * You should call {@link #setCamera} before using the gimbal.
 *
 * @return true if initialization was successful.
 */
bool Gimbal::init() {
    _minZoom = DEFAULT_MIN_ZOOM;
    _maxZoom = DEFAULT_MAX_ZOOM;
    _distance = _minZoom;
    return true;
}

/**
 * Initializes a gimbal for the specified camera
 *
 * @param camera    The camera to control
 *
 * @return true if initialization was successful.
 */
bool Gimbal::initWithCamera(const std::shared_ptr<PerspectiveCamera>& camera) {
    _camera = camera;
    _minZoom = DEFAULT_MIN_ZOOM;
    _maxZoom = DEFAULT_MAX_ZOOM;
    _distance = _minZoom;
    
    
    _eye = _camera->getPosition();
    _up  = _camera->getUp();
    _distance = _eye.length();
    
    _origPos = _eye;
    _origUp  = _up;

    return true;
}

/**
 * Disposes all of the resources used by this gimbal.
 *
 * A disposed batch can be safely reinitialized.
 */
void Gimbal::dispose() {
    _camera = nullptr;
    _up.setZero();
    _eye.setZero();
    _target.setZero();
    _origUp.setZero();
    _origPos.setZero();
    _dirty = false;
    _distance = 0;
    _minZoom = 0;
    _maxZoom = 0;
}

#pragma mark Attributes

/**
 * Sets the current camera
 *
 * This value can be null.
 *
 * @param camera    the current camera
 */
void Gimbal::setCamera(const std::shared_ptr<PerspectiveCamera>& camera) {
    if (camera == nullptr) {
        _camera = nullptr;
        _eye.setZero();
        _distance = _minZoom;
        return;
    }

    _camera = camera;
    _eye.set(camera->getPosition());
    _up.set(camera->getUp());
    _distance = _eye.length();

    _origPos.set(_eye);
    _origUp.set(_up);
}

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
void Gimbal::setMinimumZoom(float min) {
    CUAssertLog(min > 0, "Minimum zoom %f is not positive", min);
    CUAssertLog(min <= _maxZoom, "Minimum zoom %f exceeds maximum %f.", min, _maxZoom);
    _minZoom = min;
}

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
void Gimbal::setMaximumZoom(float max) {
    CUAssertLog(max > 0, "Maximum zoom %f is not positive", max);
    CUAssertLog(_minZoom <= max, "Maximum value %f is less than minimum %f.", max, _minZoom);
    _maxZoom = max;
}

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
void Gimbal::rotateY(float angle)  {
    if (!_dirty) {
        _up.set(_camera->getUp());
    }
    
    Vec3 dest = _up.getCross(_camera->getDirection());
    dest.normalize();
    dest *= _distance;
    
    _eye -= _target;
    _eye = traverseGreatCircle(_eye, dest, angle);
    _eye.normalize();
    _eye *= _distance;
    _eye += _target;
    _dirty = true;
}

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
void Gimbal::rotateX(float angle)  {
    if (!_dirty) {
        _up.set(_camera->getUp());
    }
    
    Vec3 dest = _up*_distance;
    
    _eye -= _target;
    _eye = traverseGreatCircle(_eye, dest, angle);
    _eye.normalize();
    _eye *= _distance;
    _eye += _target;
    _dirty = true;
}

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
void Gimbal::rotateZ(float angle) {
    if (!_dirty) {
        _up.set(_camera->getUp());
    }
    
    Vec3 axis = _camera->getDirection();
    axis.normalize();
    _up = rotateAbout(_up, axis, angle);
    _dirty = true;
}

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
void Gimbal::adjustZoom(float dt) {
    _distance += dt;
    if (_distance < _minZoom) {
        _distance = _minZoom;
    } else if (_distance > _maxZoom) {
        _distance = _maxZoom;
    }
    
    _eye -= _target;
    _eye.normalize();
    _eye *= _distance;
    _eye += _target;
    _dirty = true;
}

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
void Gimbal::pan(Vec3 delta) {
    pan(delta.x,delta.y,delta.z);
}

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
void Gimbal::pan(float dx, float dy, float dz) {
    _up.set(_camera->getUp());
    
    Vec3 dir1 = _camera->getDirection();
    Vec3 dir2 = dir1;
    dir2.cross(_up);
    
    Vec3 p;
    p.x = dir2.x*dx+_up.x*dy+dir1.x*dz;
    p.z = dir2.y*dx+_up.y*dy+dir1.y*dz;
    p.x = dir2.z*dx+_up.z*dy+dir1.z*dz;
    
    _target += p;
    _eye += p;
    _dirty = true;
}

/**
 * Resets the gimbal to its initial configuration.
 *
 * The initial configuration is the state of the camera at the time that
 * this object was constructed or {@link #setCamera} was called.
 */
void Gimbal::reset() {
    Vec3 dir = (-_origPos).getNormalization();
    _camera->setPosition(_origPos);
    _camera->setDirection(dir);
    _camera->setUp(_origUp);
    _camera->lookAt(Vec3::ZERO);
    _camera->update();
    
    _eye = _origPos;
    _up = _origUp;
    _target.setZero();
    _distance = _eye.length();
    _dirty = false;
}

/**
 * Updates the camera to match the gimbal state.
 *
 * If the gimbal has been moved (rotating, zooming, or panning), this
 * method must be called for this movement to have an effect.
 */
void Gimbal::update() {
    if (_dirty) {
        _camera->setPosition( _eye );
        _camera->setUp( _up );
        _camera->lookAt( _target );
        _camera->update();
        _dirty = false;
    }
}
