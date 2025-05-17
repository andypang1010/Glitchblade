//
//  CUQuaternion.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a quaternion, which is a way of representing
//  rotations in 3d sapce..  It has support for basic arithmetic, as well as
//  as the standard quaternion interpolations.
//
//  Even though this class is a candidate for vectorization, we have avoided it.
//  Vectorization only pays off when working with long arrays of vectors. In
//  addition, the vectorization APIs are a continual moving target. Most of the
//  time, it is best to enable auto-vectorization in your compiler. Indeed, in
//  our experiments, naive code with -O3 outperforms the manual vectorization
//  by almost a full order of magnitude.
//
//  Because math objects are intended to be on the stack, we do not provide
//  any shared pointer support in this class.
//
//  This module is based on an original file from GamePlay3D: http://gameplay3d.org.
//  It has been modified to support the CUGL framework.
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
#include <SDL.h>
#include <sstream>
#include <cugl/core/util/CUDebug.h>
#include <cugl/core/util/CUStringTools.h>
#include <cugl/core/math/CUQuaternion.h>
#include <cugl/core/math/CUVec3.h>
#include <cugl/core/math/CUVec4.h>
#include <cugl/core/math/CUMat4.h>

using namespace cugl;


#pragma mark -
#pragma mark Constructors
/**
 * Constructs a new quaternion from the values in the specified array.
 *
 * The elements of the array are in the order x, y, z, and w.
 *
 * @param array An array containing the elements of the quaternion.
 */
Quaternion::Quaternion(float* array) {
    CUAssertLog(array, "Source array is null");
    x = array[0]; y = array[1]; z = array[2]; w = array[3];
}

/**
 * Constructs a quaternion equal to the rotation from the specified axis and angle.
 *
 * @param axis  A vector describing the axis of rotation.
 * @param angle The angle of rotation (in radians).
 */
Quaternion::Quaternion(const Vec3 axis, float angle) {
    createFromAxisAngle(axis, angle, this);
}


#pragma mark -
#pragma mark Static Constructors
/**
 * Creates a quaternion equal to the rotational part of the matrix.
 *
 * The result is stored in dst.
 *
 * @param m     The matrix.
 * @param dst   A quaternion to store the conjugate in.
 *
 * @return A reference to dst for chaining
 */
Quaternion* Quaternion::createFromRotationMatrix(const Mat4& m, Quaternion* dst) {
    CUAssertLog(dst, "Assignment pointer is null");
    Mat4::decompose(m, nullptr, dst, nullptr);
    return dst;
}

/**
 * Creates this quaternion equal to the rotation from the specified axis and angle.
 *
 * The result is stored in dst.
 *
 * @param axis  A vector describing the axis of rotation.
 * @param angle The angle of rotation (in radians).
 * @param dst   A quaternion to store the conjugate in.
 *
 * @return A reference to dst for chaining
 */
Quaternion* Quaternion::createFromAxisAngle(const Vec3 axis, float angle, Quaternion* dst) {
    CUAssertLog(dst, "Assignment pointer is null");
    float halfAngle = angle * 0.5f;
    float sinHalfAngle = sinf(halfAngle);
    
    Vec4 normal(axis,0.0f);
    normal.normalize();
    *dst = normal*sinHalfAngle;
    dst->w = cosf(halfAngle);
    return dst;
}


#pragma mark -
#pragma mark Setters

/**
 * Sets the elements of this quaternion from the values in the specified array.
 *
 * The elements of the array are in the order x, y, z, and w.
 *
 * @param array An array containing the elements of the quaternion.
 *
 * @return A reference to this (modified) Quaternion for chaining.
 */
Quaternion& Quaternion::set(const float* array) {
    CUAssertLog(array, "Source array is null");
    x = array[0]; y = array[1]; z = array[2]; w = array[3];
    return *this;
}

/**
 * Sets the quaternion equal to the rotation from the specified axis and angle.
 *
 * @param axis  The axis of rotation.
 * @param angle The angle of rotation (in radians).
 *
 * @return A reference to this (modified) Quaternion for chaining.
 */
Quaternion& Quaternion::set(const Vec3 axis, float angle) {
    return *(createFromAxisAngle(axis, angle, this));
}


#pragma mark -
#pragma mark Static Arithmetic
/**
 * Adds the specified quaternions and stores the result in dst.
 *
 * @param q1    The first quaternion.
 * @param q2    The second quaternion.
 * @param dst   A quaternion to store the result in.
 *
 * @return A reference to dst for chaining
 */
Quaternion* Quaternion::add(const Quaternion& q1, const Quaternion& q2, Quaternion* dst) {
    CUAssertLog(dst, "Assignment pointer is null");
    dst->x = q1.x+q2.x; dst->y = q1.y+q2.y; dst->z = q1.z+q2.z; dst->w = q1.w+q2.w;
    return dst;
}

/**
 * Subtacts the quaternions q2 from q1 and stores the result in dst.
 *
 * @param q1    The first quaternion.
 * @param q2    The second quaternion.
 * @param dst   A quaternion to store the result in.
 *
 * @return A reference to dst for chaining
 */
Quaternion* Quaternion::subtract(const Quaternion& q1, const Quaternion& q2, Quaternion* dst) {
    CUAssertLog(dst, "Assignment pointer is null");
    dst->x = q1.x-q2.x; dst->y = q1.y-q2.y; dst->z = q1.z-q2.z; dst->w = q1.w-q2.w;
    return dst;
}

/**
 * Multiplies the specified quaternions and stores the result in dst.
 *
 * This method performs standard quaternion multiplication
 *
 * @param q1    The first quaternion.
 * @param q2    The second quaternion.
 * @param dst   A quaternion to store the result in.
 *
 * @return A reference to dst for chaining
 */
Quaternion* Quaternion::multiply(const Quaternion& q1, const Quaternion& q2, Quaternion* dst) {
    CUAssertLog(dst, "Assignment pointer is null");
    float x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
    float y = q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x;
    float z = q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w;
    float w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;
    
    dst->x = x;
    dst->y = y;
    dst->z = z;
    dst->w = w;

    return dst;
}

/**
 * Divides a quaternion by another and stores the result in dst.
 *
 * This method performs standard quaternion division.  That is, it multiplies
 * the first quaternion by the inverse of the second.
 *
 * @param q1    The initial quaternion.
 * @param q2    The quaternion to divide by.
 * @param dst   A quaternion to store the result in.
 *
 * @return A reference to dst for chaining
 */
Quaternion* Quaternion::divide(const Quaternion& q1, const Quaternion& q2, Quaternion* dst) {
    Quaternion inverse;
    invert(q2,&inverse);
    return multiply(q1,inverse,dst);
}

/**
 * Scales the specified quaternion by s and stores the result in dst.
 *
 * @param q1    The first quaternion.
 * @param s     The scalar value.
 * @param dst   A quaternion to store the result in.
 *
 * @return A reference to dst for chaining
 */
Quaternion* Quaternion::scale(const Quaternion& q1, float s, Quaternion* dst) {
    CUAssertLog(dst, "Assignment pointer is null");
    dst->x = q1.x*s; dst->y = q1.y*s; dst->z = q1.z*s; dst->w = q1.w*s;
    return dst;
}

/**
 * Conjugates the specified quaternion and stores the result in dst.
 *
 * @param quat  The quaternion to conjugate.
 * @param dst   A quaternion to store the result in.
 *
 * @return A reference to dst for chaining
 */
Quaternion* Quaternion::conjugate(const Quaternion& quat, Quaternion* dst) {
    CUAssertLog(dst, "Assignment pointer is null");
    dst->x = -quat.x; dst->y = -quat.y; dst->z = -quat.z; dst->w = quat.w;
    return dst;
}

/**
 * Inverts the specified quaternion and stores the result in dst.
 *
 * Note that the inverse of a quaternion is equal to its conjugate
 * when the quaternion is unit-length. For this reason, it is more
 * efficient to use the conjugate method directly when you know your
 * quaternion is already unit-length.
 *
 * If the inverse cannot be computed, this method stores a quaternion
 * with NaN values in dst.
 *
 * @param quat  The quaternion to invert.
 * @param dst   A quaternion to store the result in.
 *
 * @return A reference to dst for chaining
 */
Quaternion* Quaternion::invert(const Quaternion& quat, Quaternion* dst) {
    CUAssertLog(dst, "Assignment pointer is null");
    float n = quat.normSquared();
    n = (n < CU_MATH_FLOAT_SMALL ? NAN : 1.0f/n);
    dst->x = -quat.x*n; dst->y = -quat.y*n; dst->z = -quat.z*n; dst->w = quat.w*n;
    return dst;
}

/**
 * Normalizes the specified quaternion and stores the result in dst.
 *
 * If the quaternion already has unit length or if the length of the
 *  quaternion is zero, this method copies quat into dst.
 *
 * @param quat  The quaternion to normalize.
 * @param dst   A quaternion to store the result in.
 *
 * @return A reference to dst for chaining
 */
Quaternion* Quaternion::normalize(const Quaternion& quat, Quaternion* dst) {
    CUAssertLog(dst, "Assignment pointer is null");
    float n = quat.norm();
    n = (n < CU_MATH_EPSILON ? NAN : (n == 1.0f ? 1.0f : 1.0f/n));
    dst->x = quat.x*n; dst->y = quat.y*n; dst->z = quat.z*n; dst->w = quat.w*n;
    return dst;
}

/**
 * Negates the specified quaternion and stores the result in dst.
 *
 * @param quat  The quaternion to negate.
 * @param dst   A quaternion to store the result in.
 *
 * @return A reference to dst for chaining
 */
Quaternion* Quaternion::negate(const Quaternion& quat, Quaternion* dst) {
    CUAssertLog(dst, "Assignment pointer is null");
    dst->x = -quat.x; dst->y = -quat.y; dst->z = -quat.z; dst->w = -quat.w;
    return dst;
}

/**
 * Returns the dot product of the two quaternions
 *
 * @param q1    The first quaternion.
 * @param q2    The second quaternion.
 *
 * @return The dot product.
 */
float Quaternion::dot(const Quaternion& q1, const Quaternion& q2) {
    return (q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w);
}


#pragma mark -
#pragma mark Comparisons
/**
 * Returns true if this quaternion is equal to the given quaternion.
 *
 * Comparison is exact, which may be unreliable given that the attributes
 * are floats.
 *
 * @param q The quaternion to compare against.
 *
 * @return True if this quaternion is equal to the given quaternion.
 */
bool Quaternion::operator==(const Quaternion& q) const {
    return x == q.x && y == q.y && z == q.z && w == q.w;
}

/**
 * Returns true if this quaternion is not equal to the given quaternion.
 *
 * Comparison is exact, which may be unreliable given that the attributes
 * are floats.
 *
 * @param q The quaternion to compare quaternion.
 *
 * @return True if this quaternion is not equal to the given quaternion.
 */
bool Quaternion::operator!=(const Quaternion& q) const {
    return x != q.x || y != q.y || z != q.z || w != q.w;
}

/**
 * Returns true if the quaternions are within tolerance of each other.
 *
 * The tolerance bounds the norm of the difference between the two
 * quaternions
 *
 * @param q         The vector to compare against.
 * @param epsilon   The comparison tolerance.
 *
 * @return true if the quaternions are within tolerance of each other.
 */
bool Quaternion::equals(const Quaternion& q, float epsilon) const {
    bool a = fabsf(x-q.x) < epsilon;
    bool b = fabsf(y-q.y) < epsilon;
    bool c = fabsf(z-q.z) < epsilon;
    bool d = fabsf(w-q.w) < epsilon;
    return a && b && c && d;
}


#pragma mark -
#pragma mark Linear Attributes
/**
 * Converts this Quaternion4f to axis-angle notation.
 *
 * The angle is in radians.  The axis is normalized.
 *
 * @param e The Vec3f which stores the axis.
 *
 * @return The angle (in radians).
 */
float Quaternion::toAxisAngle(Vec3* e) const {
    CUAssertLog(e, "Assignment pointer is null");
    
    Quaternion q(x, y, z, w);
    q.normalize();
    e->x = q.x;
    e->y = q.y;
    e->z = q.z;
    e->normalize();
    
    return (2.0f * acosf(q.w));
}

/**
 * Returns true this quaternion contains all zeros.
 *
 * Comparison is exact, which may be unreliable given that the attributes
 * are floats.
 *
 * @return true if this quaternion contains all zeros, false otherwise.
 */
bool Quaternion::isZero() const {
    return x == 0.0f && y == 0.0f && z == 0.0f && w == 0.0f;
}

/**
 * Returns true if this quaternion is the identity.
 *
 * @return true if this quaternion is the identity.
 */
bool Quaternion::isIdentity() const {
    return x == 0.0f && y == 0.0f && z == 0.0f && w == 1.0f;
}


#pragma mark -
#pragma mark Static Interpolation
/**
 * Interpolates between two quaternions using linear interpolation.
 *
 * The interpolation curve for linear interpolation between quaternions
 * gives a straight line in quaternion space.
 *
 * The interpolation coefficient MUST be between 0 and 1 (inclusive). Any
 * other values will cause an error.
 *
 * @param q1    The first quaternion.
 * @param q2    The second quaternion.
 * @param t     The interpolation coefficient in [0,1].
 * @param dst   A quaternion to store the result in.
 *
 * @return A reference to dst for chaining
 */
Quaternion* Quaternion::lerp(const Quaternion& q1, const Quaternion& q2, float t, Quaternion* dst) {
    CUAssertLog(dst, "Assignment pointer is null");
    CUAssertLog(!(t < 0.0f || t > 1.0f), "Interpolation coefficient out of range: %.3f", t);
    *dst = q1+t*(q2-q1);
    return dst;
}

/**
 * Interpolates between two quaternions using spherical linear interpolation.
 *
 * Spherical linear interpolation provides smooth transitions between
 * different orientations and is often useful for animating models or
 * cameras in 3D.
 *
 * The interpolation coefficient MUST be between 0 and 1 (inclusive). Any
 * other values will cause an error.
 *
 * Note: For accurate interpolation, the input quaternions must be at
 * (or close to) unit length. This method does not automatically normalize
 * the input quaternions, so it is up to the caller to ensure they call
 * normalize beforehand, if necessary.
 *
 * Code adapted from
 * http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/slerp/
 *
 * @param q1    The first quaternion.
 * @param q2    The second quaternion.
 * @param t     The interpolation coefficient in [0,1].
 * @param dst   A quaternion to store the result in.
 *
 * @return A reference to dst for chaining
 */
Quaternion* Quaternion::slerp(const Quaternion& q1, const Quaternion& q2, float t, Quaternion* dst) {
    CUAssertLog(dst, "Assignment pointer is null");
    CUAssertLog(!(t < 0.0f || t > 1.0f), "Interpolation coefficient out of range: %.3f", t);
    CUAssertLog(q1.isUnit(), "First quaternion is not a unit quaternion");
    CUAssertLog(q2.isUnit(), "Second quaternion is not a unit quaternion");

    // Calculate angle between them.
    float cosHalfTheta = dot(q1,q2);
    
    // if qa=qb or qa=-qb then theta = 0 and we can return qa
    if (fabsf(cosHalfTheta) >= 1.0){
        *dst = q1;
        return dst;
    }
    // Calculate temporary values.
    float halfTheta = acosf(cosHalfTheta);
    float sinHalfTheta = sqrtf(1.0f - cosHalfTheta*cosHalfTheta);
    
    // if theta = 180 degrees then result is not fully defined
    // we could rotate around any axis normal to qa or qb
    const float ANGLE_THRESH = 0.001f;
    if (fabsf(sinHalfTheta) < ANGLE_THRESH) {
        *dst = (q1+q2)*0.5f;
        return dst;
    }
    float ratioA = sinf((1 - t) * halfTheta) / sinHalfTheta;
    float ratioB = sinf(t * halfTheta) / sinHalfTheta;

    //calculate Quaternion.
    *dst = q1*ratioA+q2*ratioB;
    return dst;
}

/**
 * Interpolates between two quaternions using normalized linear interpolation.
 *
 * Normalized linear interpolation provides smooth transitions between
 * different orientations and is often useful for animating models or
 * cameras in 3D.
 *
 * The interpolation coefficient MUST be between 0 and 1 (inclusive). Any
 * other values will cause an error.
 *
 * In addition, the input quaternions must be at (or close to) unit length.
 * When assertions are enabled, it will test this via the isUnit() method.
 *
 * @param q1    The first quaternion.
 * @param q2    The second quaternion.
 * @param t     The interpolation coefficient in [0,1].
 * @param dst   A quaternion to store the result in.
 *
 * @return A reference to dst for chaining
 */
Quaternion* Quaternion::nlerp(const Quaternion& q1, const Quaternion& q2, float t, Quaternion* dst) {
    CUAssertLog(q1.isUnit(), "First quaternion is not a unit quaternion");
    CUAssertLog(q2.isUnit(), "Second quaternion is not a unit quaternion");
    lerp(q1,q2,t,dst);
    dst->normalize();
    return dst;
}

/**
 * Rotates the vector by this quaternion and stores the result in dst.
 *
 * The rotation is defined by the matrix associated with the vector.
 *
 * @param v     The vector to rotate.
 * @param quat  The rotation quaternion.
 * @param dst   A vector to store the result in.
 *
 * @return A reference to dst for chaining
 */
Vec3* Quaternion::rotate(const Vec3 v, const Quaternion& quat, Vec3* dst) {
    Vec4 uv, uuv;
    Vec4 qvec = quat;
    Vec4 vvec(v,0.0f);
    qvec.w = 0.0f;
    
    Vec4::cross(qvec, vvec, &uv);
    Vec4::cross(qvec, uv,  &uuv);

    uv  *= (2.0f * quat.w);
    uuv *= 2.0f;
    vvec = vvec + uv + uuv;
    dst->x = vvec.x; dst->y = vvec.y; dst->z = vvec.z;
    return dst;
}

#pragma mark -
#pragma mark Conversion Methods
/**
 * Returns a string representation of this vector for debuggging purposes.
 *
 * If verbose is true, the string will include class information.  This
 * allows us to unambiguously identify the class.
 *
 * @param verbose Whether to include class information
 *
 * @return a string representation of this vector for debuggging purposes.
 */
std::string Quaternion::toString(bool verbose) const {
    std::stringstream ss;
    ss << (verbose ? "cugl::Quaternion[" : "");
    ss << strtool::to_string(w);
    ss << "+";
    ss << strtool::to_string(x);
    ss << "i+";
    ss << strtool::to_string(y);
    ss << "j+";
    ss << strtool::to_string(z);
    ss << (verbose ? "k]" : "k");
    return ss.str();
}

/** Cast from Quaternion to a Vec4. */
Quaternion::operator Vec4() const {
    return Vec4(x,y,z,w);
}


/**
 * Sets the coordinates of this quaternion to those of the given vector.
 *
 * @param vector The vector to convert
 *
 * @return A reference to this (modified) Qauternion for chaining.
 */
Quaternion& Quaternion::operator= (const Vec4 vector) {
    return set(vector);
}

/**
 * Sets the coordinates of this quaternion to those of the given vector.
 *
 * @param vector The vector to convert
 *
 * @return A reference to this (modified) Qauternion for chaining.
 */
Quaternion& Quaternion::set(const Vec4 vector) {
    x = vector.x; y = vector.y; z = vector.z; w = vector.w;
    return *this;
}


/**
 * Cast from Quaternion to a Matrix.
 *
 * The matrix is a rotation matrix equivalent to the rotation represented
 * by this quaternion.
 */
Quaternion::operator Mat4() const {
    Mat4 result(*this);
    return result;
}

/**
 * Constructs a quaternion equal to the rotational part of the specified matrix.
 *
 * This constructor may fail, particularly if the scale component of the
 * matrix is too small.  In that case, this method intializes this
 * quaternion to the zero quaternion.
 *
 * @param m The matrix to extract the rotation.
 */
Quaternion::Quaternion(const Mat4& m) {
    createFromRotationMatrix(m, this);
}

/**
 * Sets the quaternion equal to the rotational part of the specified matrix.
 *
 * This constructor may fail, particularly if the scale component of the
 * matrix is too small.  In that case, this method intializes this
 * quaternion to the zero quaternion.
 *
 * @param m The matrix.
 *
 * @return A reference to this (modified) Quaternion for chaining.
 */
Quaternion& Quaternion::set(const Mat4& m) {
    return *(createFromRotationMatrix(m, this));
}


#pragma mark -
#pragma mark Constants

/** The zero quaternion Quaternion(0,0,0,0) */
const Quaternion Quaternion::ZERO(0,0,0,0);
/** The identity quaternion Quaternion(0,0,0,1) */
const Quaternion Quaternion::IDENTITY(0,0,0,1);

