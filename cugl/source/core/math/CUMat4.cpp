//
//  CUMat4.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a 4d matrix, which is the standard transform
//  matrix in OpenGL.  The class has support for basic camera creation, as well
//  as the traditional transforms.  It can transform any of Vec2, Vec3, and Vec4.
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
#include <sstream>
#include <algorithm>

#include <cugl/core/util/CUStringTools.h>
#include <cugl/core/util/CUDebug.h>
#include <cugl/core/math/CUMathBase.h>
#include <cugl/core/math/CUMat4.h>
#include <cugl/core/math/CUQuaternion.h>
#include <cugl/core/math/CUAffine2.h>
#include <cugl/core/math/CURect.h>

using namespace cugl;

#pragma mark -

#define MATRIX_SIZE ( sizeof(float) * 16)

#pragma mark -
#pragma mark Constructors
/**
 * Creates the identity matrix.
 *
 *     1  0  0  0
 *     0  1  0  0
 *     0  0  1  0
 *     0  0  0  1
 */
Mat4::Mat4() {
    std::memset(m, 0, MATRIX_SIZE);
    m[0] = 1; m[5] = 1; m[10] = 1; m[15] = 1;
}

/**
 * Constructs a matrix initialized to the specified values.
 *
 * @param m11 The first element of the first row.
 * @param m12 The second element of the first row.
 * @param m13 The third element of the first row.
 * @param m14 The fourth element of the first row.
 * @param m21 The first element of the second row.
 * @param m22 The second element of the second row.
 * @param m23 The third element of the second row.
 * @param m24 The fourth element of the second row.
 * @param m31 The first element of the third row.
 * @param m32 The second element of the third row.
 * @param m33 The third element of the third row.
 * @param m34 The fourth element of the third row.
 * @param m41 The first element of the fourth row.
 * @param m42 The second element of the fourth row.
 * @param m43 The third element of the fourth row.
 * @param m44 The fourth element of the fourth row.
 */
Mat4::Mat4(float m11, float m12, float m13, float m14,
           float m21, float m22, float m23, float m24,
           float m31, float m32, float m33, float m34,
           float m41, float m42, float m43, float m44) {
    set(m11,m12,m13,m14,m21,m22,m23,m24,m31,m32,m33,m34,m41,m42,m43,m44);
}

/**
 * Creates a matrix initialized to the specified column-major array.
 *
 * The passed-in array is in column-major order, so the memory layout of
 * the array is as follows:
 *
 *     0   4   8   12
 *     1   5   9   13
 *     2   6   10  14
 *     3   7   11  15
 *
 * @param mat An array containing 16 elements in column-major order.
 */
Mat4::Mat4(const float* mat) {
    CUAssertLog(mat, "Source array is null");
    std::memcpy(&(this->m[0]), mat, MATRIX_SIZE);
}

/**
 * Constructs a new matrix that is the copy of the specified one.
 *
 * @param copy The matrix to copy.
 */
Mat4::Mat4(const Mat4& copy) {
    std::memcpy(&(this->m[0]), &(copy.m[0]), MATRIX_SIZE);
}

/**
 * Constructs a new matrix that contains the resources of the specified one.
 *
 * @param copy The matrix contributing resources.
 */
Mat4::Mat4(Mat4&& copy) {
    std::memcpy(&(this->m[0]), &(copy.m[0]), MATRIX_SIZE);
}


#pragma mark -
#pragma mark Static Constructors
/**
 * Creates a view matrix based on the specified input parameters.
 *
 * @param eyeX      The eye x-coordinate position.
 * @param eyeY      The eye y-coordinate position.
 * @param eyeZ      The eye z-coordinate position.
 * @param targetX   The target's center x-coordinate position.
 * @param targetY   The target's center y-coordinate position.
 * @param targetZ   The target's center z-coordinate position.
 * @param upX       The up vector x-coordinate value.
 * @param upY       The up vector y-coordinate value.
 * @param upZ       The up vector z-coordinate value.
 * @param dst       A matrix to store the result in.
 *
 * @return A reference to dst for chaining
 */
Mat4* Mat4::createLookAt(float eyeX,    float eyeY,     float eyeZ,
                         float targetX, float targetY,  float targetZ,
                         float upX,     float upY,      float upZ,      Mat4* dst) {
    CUAssertLog(dst, "Assignment matrix is null");
   
    Vec4 eye(eyeX, eyeY, eyeZ, 0.0f);
    Vec4 target(targetX, targetY, targetZ, 0.0f);
    Vec4 up(upX, upY, upZ, 0.0f);
    up.normalize();
    
    Vec4 zaxis;
    Vec4::subtract(eye, target, &zaxis);
    zaxis.normalize();
    
    Vec4 xaxis;
    Vec4::cross(up, zaxis, &xaxis);
    xaxis.normalize();
    
    Vec4 yaxis;
    Vec4::cross(zaxis, xaxis, &yaxis);
    yaxis.normalize();
    
    dst->m[0] = xaxis.x;
    dst->m[1] = yaxis.x;
    dst->m[2] = zaxis.x;
    dst->m[3] = 0.0f;
    
    dst->m[4] = xaxis.y;
    dst->m[5] = yaxis.y;
    dst->m[6] = zaxis.y;
    dst->m[7] = 0.0f;
    
    dst->m[8] = xaxis.z;
    dst->m[9] = yaxis.z;
    dst->m[10] = zaxis.z;
    dst->m[11] = 0.0f;
    
    dst->m[12] = -Vec3::dot(xaxis, eye);
    dst->m[13] = -Vec3::dot(yaxis, eye);
    dst->m[14] = -Vec3::dot(zaxis, eye);
    dst->m[15] = 1.0f;

    return dst;
}

/**
 * Creates a view matrix based on the specified input vectors, putting it in dst.
 *
 * @param eye       The eye position.
 * @param target    The target's center position.
 * @param up        The up vector.
 * @param dst       A matrix to store the result in.
 *
 * @return A reference to dst for chaining
 */
Mat4* Mat4::createLookAt(const Vec3 eye, const Vec3 target, const Vec3 up, Mat4* dst) {
    return Mat4::createLookAt(eye.x, eye.y, eye.z,
                              target.x, target.y, target.z,
                              up.x, up.y, up.z,
                              dst);
}

/**
 * Builds a perspective projection matrix based on a field of view.
 *
 * Projection space refers to the space after applying projection
 * transformation from view space. After the projection transformation,
 * visible content has x- and y-coordinates ranging from -1 to 1, and a
 * z-coordinate ranging from 0 to 1. To obtain the viewable area
 * (in world space) of a scene, create a bounding frustum and pass the
 * combined view and projection matrix to the constructor.
 *
 * @param fieldOfView   The field of view in the y direction (in degrees).
 * @param aspectRatio   The aspect ratio, defined as view space width divided by height.
 * @param zNearPlane    The distance to the near view plane.
 * @param zFarPlane     The distance to the far view plane.
 * @param dst           A matrix to store the result in.
 *
 * @return A reference to dst for chaining
 */
Mat4* Mat4::createPerspective(float fieldOfView, float aspectRatio,
                              float zNearPlane, float zFarPlane, Mat4* dst) {
    CUAssertLog(dst, "Assignment matrix is null");
    CUAssertLog(zFarPlane != zNearPlane, "Z-axis range is degenerate");
    CUAssertLog(aspectRatio, "Aspect ratio is degenerate");
    
    float f_n = 1.0f / (zFarPlane - zNearPlane);
    float theta = CU_MATH_DEG_TO_RAD(fieldOfView) * 0.5f;
    if (fabs(fmod(theta, M_PI_2)) < CU_MATH_EPSILON) {
        CULogError("Invalid field of view value (%f) attempted calculation tan(%f), which is undefined.", fieldOfView, theta);
        return nullptr;
    }
    
    float divisor = tan(theta);
    CUAssertLog(divisor, "Field of view factor is degenerate");
    float factor = 1.0f / divisor;
    
    std::memset(dst, 0, MATRIX_SIZE);
    dst->m[0] = (1.0f / aspectRatio) * factor;
    dst->m[5] = factor;
    dst->m[10] = (-(zFarPlane + zNearPlane)) * f_n;
    dst->m[11] = -1.0f;
    dst->m[14] = -2.0f * zFarPlane * zNearPlane * f_n;
    return dst;
}

/**
 * Creates an orthographic projection matrix.
 *
 * Projection space refers to the space after applying projection
 * transformation from view space. After the projection transformation,
 * visible content has x and y coordinates ranging from -1 to 1, and z
 * coordinates ranging from 0 to 1. Unlike perspective projection, there is
 * no perspective foreshortening in orthographic projection.
 *
 * The viewable area of this orthographic projection extends from left to
 * right on the x-axis and bottom to top on the y-axis. The z-axis is bound
 * between zNearPlane and zFarPlane. These values are relative to the
 * position and x, y, and z-axes of the view.
 *
 * To obtain the viewable area (in world space) of a scene, create a
 * bounding frustum and pass the combined view and projection matrix to
 * the constructor.
 *
 * @param left The minimum x-value of the view volume.
 * @param right The maximum x-value of the view volume.
 * @param bottom The minimum y-value of the view volume.
 * @param top The maximum y-value of the view volume.
 * @param zNearPlane The minimum z-value of the view volume.
 * @param zFarPlane The maximum z-value of the view volume.
 * @param dst A matrix to store the result in.
 *
 * @return A reference to dst for chaining
 */
Mat4* Mat4::createOrthographicOffCenter(float left, float right, float bottom, float top,
                                        float zNearPlane, float zFarPlane, Mat4* dst) {
    CUAssertLog(dst,"Assignment matrix is null");
    CUAssertLog(right != left, "Width is degenerate");
    CUAssertLog(top != bottom, "Height is degenerate");
    CUAssertLog(zFarPlane != zNearPlane, "Z-axis range is degenerate");
    
    std::memset(&(dst->m[0]), 0, MATRIX_SIZE);
    dst->m[0] = 2 / (right - left);
    dst->m[5] = 2 / (top - bottom);
    dst->m[10] = 2 / (zNearPlane - zFarPlane);
    
    dst->m[12] = (left + right) / (left - right);
    dst->m[13] = (top + bottom) / (bottom - top);
    dst->m[14] = (zNearPlane + zFarPlane) / (zNearPlane - zFarPlane);
    dst->m[15] = 1;
    return dst;
}

/**
 * Creates a uniform scale matrix.
 *
 * @param scale The amount to scale.
 * @param dst   A matrix to store the result in.
 *
 * @return A reference to dst for chaining
 */
Mat4* Mat4::createScale(float scale, Mat4* dst) {
    CUAssertLog(dst,"Assignment matrix is null");
    
    std::memcpy(&(dst->m[0]), &(IDENTITY.m[0]), MATRIX_SIZE);
    dst->m[0]  = scale;
    dst->m[5]  = scale;
    dst->m[10] = scale;
    return dst;
}

/**
 * Creates a nonuniform scale matrix.
 *
 * @param sx    The amount to scale along the x-axis.
 * @param sy    The amount to scale along the y-axis.
 * @param sz    The amount to scale along the z-axis.
 * @param dst   A matrix to store the result in.
 *
 * @return A reference to dst for chaining
 */
Mat4* Mat4::createScale(float sx, float sy, float sz, Mat4* dst) {
    CUAssertLog(dst,"Assignment matrix is null");
    
    std::memcpy(&(dst->m[0]), &(IDENTITY.m[0]), MATRIX_SIZE);
    dst->m[0]  = sx;
    dst->m[5]  = sy;
    dst->m[10] = sz;
    return dst;
}

/**
 * Creates a nonuniform scale matrix from the given vector.
 *
 * @param scale The nonuniform scale value.
 * @param dst   A matrix to store the result in.
 *
 * @return A reference to dst for chaining
 */
Mat4* Mat4::createScale(const Vec3 scale, Mat4* dst) {
    CUAssertLog(dst,"Assignment matrix is null");
    
    std::memcpy(&(dst->m[0]), &(IDENTITY.m[0]), MATRIX_SIZE);
    dst->m[0]  = scale.x;
    dst->m[5]  = scale.y;
    dst->m[10] = scale.z;
    return dst;
}

/**
 * Creates a rotation matrix from the specified quaternion.
 *
 * @param quat  A quaternion describing a 3D orientation.
 * @param dst   A matrix to store the result in.
 *
 * @return A reference to dst for chaining
 */
Mat4* Mat4::createRotation(const Quaternion& quat, Mat4* dst) {
    CUAssertLog(dst, "Assignment matrix is null");
    dst->set(quat);
    return dst;
}

/**
 * Creates a rotation matrix from the specified axis and angle.
 *
 * The angle measurement is in radians.  The rotation is counter
 * clockwise about the axis.
 *
 * @param axis  A vector describing the axis to rotate about.
 * @param angle The angle (in radians).
 * @param dst   A matrix to store the result in.
 *
 * @return A reference to dst for chaining
 */
Mat4* Mat4::createRotation(const Vec3 axis, float angle, Mat4* dst) {
    CUAssertLog(dst, "Assignment matrix is null");

    Vec3 n(axis);
    n.normalize();
    
    float c = cos(angle);
    float s = sin(angle);
    
    float t  = 1.0f - c;
    float tx = t * n.x;
    float ty = t * n.y;
    float tz = t * n.z;
    float txy = tx * n.y;
    float txz = tx * n.z;
    float tyz = ty * n.z;
    float sx  = s * n.x;
    float sy  = s * n.y;
    float sz  = s * n.z;

    dst->m[0] = c + tx*n.x;
    dst->m[1] = txy + sz;
    dst->m[2] = txz - sy;
    dst->m[3] = 0.0f;
    
    dst->m[4] = txy - sz;
    dst->m[5] = c + ty*n.y;
    dst->m[6] = tyz + sx;
    dst->m[7] = 0.0f;
    
    dst->m[8] = txz + sy;
    dst->m[9] = tyz - sx;
    dst->m[10] = c + tz*n.z;
    dst->m[11] = 0.0f;
    
    dst->m[12] = 0.0f;
    dst->m[13] = 0.0f;
    dst->m[14] = 0.0f;
    dst->m[15] = 1.0f;

    return dst;
}

/**
 * Creates a matrix specifying a rotation around the x-axis.
 *
 * The angle measurement is in radians.  The rotation is counter
 * clockwise about the axis.
 *
 * @param angle The angle of rotation (in radians).
 * @param dst   A matrix to store the result in.
 *
 * @return A reference to dst for chaining
 */
Mat4* Mat4::createRotationX(float angle, Mat4* dst) {
    CUAssertLog(dst, "Assignment matrix is null");
    
    float c = cos(angle);
    float s = sin(angle);
    
    std::memcpy(&(dst->m[0]), &(IDENTITY.m[0]), MATRIX_SIZE);
    dst->m[5]  = c;
    dst->m[6]  = s;
    dst->m[9]  = -s;
    dst->m[10] = c;
    return dst;
}

/**
 * Creates a matrix specifying a rotation around the y-axis.
 *
 * The angle measurement is in radians.  The rotation is counter
 * clockwise about the axis.
 *
 * @param angle The angle of rotation (in radians).
 * @param dst   A matrix to store the result in.
 *
 * @return A reference to dst for chaining
 */
Mat4* Mat4::createRotationY(float angle, Mat4* dst) {
    CUAssertLog(dst, "Assignment matrix is null");
    
    float c = cos(angle);
    float s = sin(angle);
    
    std::memcpy(&(dst->m[0]), &(IDENTITY.m[0]), MATRIX_SIZE);
    dst->m[0]  = c;
    dst->m[2]  = -s;
    dst->m[8]  = s;
    dst->m[10] = c;
    return dst;
}

/**
 * Creates a matrix specifying a rotation around the z-axis.
 *
 * The angle measurement is in radians.  The rotation is counter
 * clockwise about the axis.
 *
 * @param angle The angle of rotation (in radians).
 * @param dst   A matrix to store the result in.
 *
 * @return A reference to dst for chaining
 */
Mat4* Mat4::createRotationZ(float angle, Mat4* dst) {
    CUAssertLog(dst, "Assignment matrix is null");
    
    float c = cos(angle);
    float s = sin(angle);
    
    std::memcpy(&(dst->m[0]), &(IDENTITY.m[0]), MATRIX_SIZE);
    dst->m[0] = c;
    dst->m[1] = s;
    dst->m[4] = -s;
    dst->m[5] = c;
    return dst;
}

/**
 * Creates a translation matrix from the given offset.
 *
 * @param trans The translation offset.
 * @param dst   A matrix to store the result in.
 *
 * @return A reference to dst for chaining
 */
Mat4* Mat4::createTranslation(const Vec3 trans, Mat4* dst) {
    CUAssertLog(dst, "Assignment matrix is null");
    
    std::memcpy(&(dst->m[0]), &(IDENTITY.m[0]), MATRIX_SIZE);
    dst->m[12] = trans.x;
    dst->m[13] = trans.y;
    dst->m[14] = trans.z;

    return dst;
}

/**
 * Creates a translation matrix from the given parameters.
 *
 * @param tx    The translation on the x-axis.
 * @param ty    The translation on the y-axis.
 * @param tz    The translation on the z-axis.
 * @param dst   A matrix to store the result in.
 *
 * @return A reference to dst for chaining
 */
Mat4* Mat4::createTranslation(float tx, float ty, float tz, Mat4* dst) {
    CUAssertLog(dst, "Assignment matrix is null");
    
	std::memcpy(&(dst->m[0]), &(IDENTITY.m[0]), MATRIX_SIZE);
    dst->m[12] = tx;
    dst->m[13] = ty;
    dst->m[14] = tz;

    return dst;
}


#pragma mark -
#pragma mark Setters

/**
 * Sets the individal values of this matrix.
 *
 * @param m11 The first element of the first row.
 * @param m12 The second element of the first row.
 * @param m13 The third element of the first row.
 * @param m14 The fourth element of the first row.
 * @param m21 The first element of the second row.
 * @param m22 The second element of the second row.
 * @param m23 The third element of the second row.
 * @param m24 The fourth element of the second row.
 * @param m31 The first element of the third row.
 * @param m32 The second element of the third row.
 * @param m33 The third element of the third row.
 * @param m34 The fourth element of the third row.
 * @param m41 The first element of the fourth row.
 * @param m42 The second element of the fourth row.
 * @param m43 The third element of the fourth row.
 * @param m44 The fourth element of the fourth row.
 *
 * @return A reference to this (modified) Mat4 for chaining.
 */
Mat4& Mat4::set(float m11, float m12, float m13, float m14,
                float m21, float m22, float m23, float m24,
                float m31, float m32, float m33, float m34,
                float m41, float m42, float m43, float m44) {
    m[0]  = m11;
    m[1]  = m21;
    m[2]  = m31;
    m[3]  = m41;
    m[4]  = m12;
    m[5]  = m22;
    m[6]  = m32;
    m[7]  = m42;
    m[8]  = m13;
    m[9]  = m23;
    m[10] = m33;
    m[11] = m43;
    m[12] = m14;
    m[13] = m24;
    m[14] = m34;
    m[15] = m44;

    return *this;
}

/**
 * Sets the values of this matrix to those in the specified column-major array.
 *
 * The passed-in array is in column-major order, so the memory layout of
 * the array is as follows:
 *
 *     0   4   8   12
 *     1   5   9   13
 *     2   6   10  14
 *     3   7   11  15
 *
 * @param mat An array containing 16 elements in column-major order.
 *
 * @return A reference to this (modified) Mat4 for chaining.
 */
Mat4& Mat4::set(const float* mat) {
    CUAssertLog(mat, "Source array is null");
    std::memcpy(&(this->m[0]), mat, MATRIX_SIZE);
    return *this;
}

/**
 * Sets this matrix as a rotation matrix from the specified quaternion.
 *
 * @param quat  A quaternion describing a 3D orientation.
 *
 * @return A reference to this (modified) Mat4 for chaining.
 */
Mat4& Mat4::set(const Quaternion& quat) {
    float x2 = quat.x + quat.x;
    float y2 = quat.y + quat.y;
    float z2 = quat.z + quat.z;
    
    float xx2 = quat.x * x2;
    float yy2 = quat.y * y2;
    float zz2 = quat.z * z2;
    float xy2 = quat.x * y2;
    float xz2 = quat.x * z2;
    float yz2 = quat.y * z2;
    float wx2 = quat.w * x2;
    float wy2 = quat.w * y2;
    float wz2 = quat.w * z2;
    
    m[0] = 1.0f - yy2 - zz2;
    m[1] = xy2 + wz2;
    m[2] = xz2 - wy2;
    m[3] = 0.0f;
    
    m[4] = xy2 - wz2;
    m[5] = 1.0f - xx2 - zz2;
    m[6] = yz2 + wx2;
    m[7] = 0.0f;
    
    m[8] = xz2 + wy2;
    m[9] = yz2 - wx2;
    m[10] = 1.0f - xx2 - yy2;
    m[11] = 0.0f;
    
    m[12] = 0.0f;
    m[13] = 0.0f;
    m[14] = 0.0f;
    m[15] = 1.0f;

    return *this;
}

/**
 * Sets the elements of this matrix to those in the specified matrix.
 *
 * @param mat The matrix to copy.
 *
 * @return A reference to this (modified) Mat4 for chaining.
 */
Mat4& Mat4::set(const Mat4& mat) {
    memcpy(&(this->m[0]), &(mat.m[0]), MATRIX_SIZE);
    return *this;
}

/**
 * Sets this matrix to the identity matrix.
 *
 * @return A reference to this (modified) Mat4 for chaining.
 */
Mat4& Mat4::setIdentity() {
	std::memcpy(&(this->m[0]), &(IDENTITY.m[0]), MATRIX_SIZE);
	return *this;
}

/**
 * Sets all elements of the current matrix to zero.
 *
 * @return A reference to this (modified) Mat4 for chaining.
 */
Mat4& Mat4::setZero() {
    std::memset(&(this->m[0]), 0, MATRIX_SIZE);
    return *this;
}


#pragma mark -
#pragma mark Comparisons
/**
 * Returns true if the matrices are exactly equal to each other.
 *
 * This method may be unreliable given that the elements are floats.
 * It should only be used to compared matrices that have not undergone
 * a lot of transformations.
 *
 * @param mat       The matrix to compare against.
 *
 * @return true if the matrices are exactly equal to each other.
 */
bool Mat4::isExactly(const Mat4& mat) const {
    return std::memcmp(&m[0],&(mat.m[0]),MATRIX_SIZE) == 0;
}

/**
 * Returns true if the matrices are within tolerance of each other.
 *
 * The tolerance is applied to each element of the matrix individually.
 *
 * @param mat       The matrix to compare against.
 * @param epsilon   The comparison tolerance.
 *
 * @return true if the matrices are within tolerance of each other.
 */
bool Mat4::equals(const Mat4& mat, float epsilon) const {
    bool similar = true;
    for(int ii = 0; similar && ii < 16; ii++) {
        similar = (fabsf(m[ii]-mat.m[ii]) <= epsilon);
    }
    return similar;
}


#pragma mark -
#pragma mark Matrix Attributes
/**
 * Returns true if this matrix is equal to the identity matrix.
 *
 * The optional comparison tolerance takes into accout that elements
 * are floats and this may not be exact.  The tolerance is applied to
 * each element individually.  By default, the match must be exact.
 *
 * @param epsilon   The comparison tolerance
 *
 * @return true if this matrix is equal to the identity matrix.
 */
bool Mat4::isIdentity(float epsilon) const {
    return equals(IDENTITY,epsilon);
}

/**
 * Returns true if this matrix is orthogonal.
 *
 * The optional comparison tolerance takes into accout that elements
 * are floats and this may not be exact.  The tolerance is applied to
 * BOTH the normality test and the dot-product test for each pair.
 *
 * @param epsilon   The comparison tolerance
 *
 * @return true if this matrix is orthogonal.
 */
bool Mat4::isOrthogonal(float epsilon) const {
    Mat4 trans;
    Mat4::transpose(*this, &trans);
    Mat4::multiply(trans, *this, &trans);
    return trans.equals(Mat4::IDENTITY,epsilon);
}

/**
 * Returns the determinant of this matrix.
 *
 * @return the determinant of this matrix.
 */
float Mat4::getDeterminant() const {
    float a0 = m[0] * m[5] - m[1] * m[4];
    float a1 = m[0] * m[6] - m[2] * m[4];
    float a2 = m[0] * m[7] - m[3] * m[4];
    
    
    float a3 = m[1] * m[6] - m[2] * m[5];
    float a4 = -m[1] * m[7] + m[3] * m[5];
    float a5 = m[2] * m[7] - m[3] * m[6];
   
    float b0 = m[8] * m[13] - m[9] * m[12];
    float b1 = m[8] * m[14] - m[10] * m[12];
    float b2 = m[8] * m[15] - m[11] * m[12];
    
    float b3 = m[9] * m[14] - m[10] * m[13];
    float b4 = -m[9] * m[15] + m[11] * m[13];
    float b5 = m[10] * m[15] - m[11] * m[14];
    
    // Calculate the determinant.
    return (a0 * b5 + a1 * b4 + a2 * b3 + a3 * b2 + a4 * b1 + a5 * b0);
}

/**
 * Returns the scale component of this matrix.
 *
 * If the scale component of this matrix has negative parts,
 * it is not possible to always extract the exact scale component.
 * In that case, a scale vector that is mathematically equivalent to
 * the original scale vector is extracted and returned.
 *
 * To work properly, the matrix must have been constructed in the following
 * order: scale, then rotate, then translation. In any other order, the
 * scale is not guaranteed to be correct.
 *
 * @return the scale component of this matrix.
 */
Vec3 Mat4::getScale() const {
    Vec3 result;
    decompose(*this,&result,nullptr,nullptr);
    return result;
}

/**
 * Returns the rotational component of this matrix.
 *
 * If the scale component is too close to zero, we cannot extract the
 * rotation.  In that case, we return the zero quaternion.
 (
 * @return the rotational component of this matrix.
 */
Quaternion Mat4::getRotation() const {
    Quaternion result;
    decompose(*this,nullptr,&result,nullptr);
    return result;
}

/**
 * Returns the translational component of this matrix.
 *
 * To work properly, the matrix must have been constructed in the following
 * order: scale, then rotate, then translation. In any other order, the
 * translation is not guaranteed to be correct.
 *
 * @return the translational component of this matrix.
 */
Vec3 Mat4::getTranslation() const {
    Vec3 result;
    decompose(*this,nullptr,nullptr,&result);
    return result;
}

/**
 * Returns the up vector of this matrix, when treated as a camera.
 *
 * @return the up vector of this matrix, when treated as a camera.
 */
Vec3 Mat4::getUpVector() const {
    return Vec3(m[4],m[5],m[6]);
}

/**
 * Returns the down vector of this matrix, when treated as a camera.
 *
 * @return the down vector of this matrix, when treated as a camera.
 */
Vec3 Mat4::getDownVector() const {
    return Vec3(-m[4],-m[5],-m[6]);
}

/**
 * Returns the left vector of this matrix, when treated as a camera.
 *
 * @return the left vector of this matrix, when treated as a camera.
 */
Vec3 Mat4::getLeftVector() const {
    return Vec3(m[0],m[1],m[2]);
}

/**
 * Returns the right vector of this matrix, when treated as a camera.
 *
 * @return the right vector of this matrix, when treated as a camera.
 */
Vec3 Mat4::getRightVector() const {
    return Vec3(-m[0],-m[1],-m[2]);
}

/**
 * Returns the forward vector of this matrix, when treated as a camera.
 *
 * @return the forward vector of this matrix, when treated as a camera.
 */
Vec3 Mat4::getForwardVector() const {
    return Vec3(-m[8],-m[9],-m[10]);
}

/**
 * Returns the backward vector of this matrix, when treated as a camera.
 *
 * @return the backward vector of this matrix, when treated as a camera.
 */
Vec3 Mat4::getBackVector() const {
    return Vec3(m[8],m[9],m[10]);
}

#pragma mark -
#pragma mark Vector Operations
/**
 * Returns a copy of this point transformed by the matrix.
 *
 * The vector is treated as a point, which means that translation is
 * applied to the result.
 *
 * Note: This does not modify the original point. To transform a
 * point in place, use the static method (or the appropriate operator).
 *
 * @param point The point to transform.
 *
 * @return a copy of this point transformed by the matrix.
 */
Vec2 Mat4::transform(const Vec2 point) const {
    Vec4 result(point);
    transform(*this,result,&result);
    return Vec2(result.x,result.y);
}

/**
 * Returns a copy of this vector transformed by the matrix.
 *
 * The vector is treated as a direction, which means that translation is
 * not applied to the result.
 *
 * Note: This does not modify the original vector. To transform a
 * vector in place, use the static method (or the appropriate operator).
 *
 * @param vec The vector to transform.
 *
 * @return a copy of this point transformed by the matrix.
 */
Vec2 Mat4::transformVector(const Vec2 vec) const {
    Vec2 result;
    transformVector(*this,vec,&result);
    return Vec2(result.x,result.y);
}

/**
 * Returns a copy of this point transformed by the matrix.
 *
 * The vector is treated as a point, which means that translation is
 * applied to the result.
 *
 * Note: This does not modify the original point. To transform a
 * point in place, use the static method (or the appropriate operator).
 *
 * @param point The point to transform.
 *
 * @return a copy of this point transformed by the matrix.
 */
Vec3 Mat4::transform(const Vec3 point) const {
    Vec4 result(point);
    transform(*this,result,&result);
    return Vec3(result.x,result.y,result.z);
}

/**
 * Returns a copy of this vector transformed by the matrix.
 *
 * The vector is treated as a direction, which means that translation is
 * not applied to the result.
 *
 * Note: This does not modify the original vector. To transform a
 * vector in place, use the static method (or the appropriate operator).
 *
 * @param vec The vector to transform.
 *
 * @return a copy of this point transformed by the matrix.
 */
Vec3 Mat4::transformVector(const Vec3 vec) const {
    Vec4 result(vec.x,vec.y,vec.z,0.0f);
    transform(*this,result,&result);
    return Vec3(result.x,result.y,result.z);
}

/**
 * Returns a copy of this vector transformed by the matrix.
 *
 * The vector is treated as is.  Hence whether or not translation is applied
 * depends on the value of w.
 *
 * Note: This does not modify the original vector. To transform a
 * vector in place, use the static method (or the appropriate operator).
 *
 * @param vec   The vector to transform.
 *
 * @return a copy of this point transformed by the matrix.
 */
Vec4 Mat4::transform(const Vec4 vec) const {
    Vec4 result;
    transform(*this,vec,&result);
    return result;
}

/**
 * Returns a copy of the given rectangle transformed.
 *
 * This method transforms the four defining points of the rectangle.  It
 * then computes the minimal bounding box storing these four points
 *
 * Note: This does not modify the original rectangle. To transform a
 * point in place, use the static method.
 *
 * @param rect  The rect to transform.
 *
 * @return A reference to dst for chaining
 */
Rect Mat4::transform(const Rect rect) const {
    Rect result;
    return *(transform(*this,rect,&result));
}

#pragma mark -
#pragma mark Static Arithmetic

/**
 * Adds a scalar to each component of mat and stores the result in dst.
 *
 * @param mat       The matrix to add to.
 * @param scalar    The scalar value to add.
 * @param dst       A matrix to store the result in.
 *
 * @return A reference to dst for chaining
 */
Mat4* Mat4::add(const Mat4& mat, float scalar, Mat4* dst) {
    float* m = dst->m;
    for(size_t ii = 0; ii < 16; ii++) {
        m[ii] = mat.m[ii]  + scalar;
    }
    return dst;
}

/**
 * Adds a scalar to each component of mat and stores the result in dst.
 *
 * This method assumes the float arrays are in column major order.
 *
 * @param mat       The matrix to add to in column-major order
 * @param scalar    The scalar value to add.
 * @param dst       A matrix to store the result in column-major order
 *
 * @return A reference to dst for chaining
 */
float* Mat4::add(const float* mat, float scalar, float* dst) {
    for(size_t ii = 0; ii < 16; ii++) {
        dst[ii]  = mat[ii]  + scalar;
    }
    return dst;
}

/**
 * Adds the specified matrices and stores the result in dst.
 *
 * @param m1    The first matrix.
 * @param m2    The second matrix.
 * @param dst   The destination matrix to add to.
 *
 * @return A reference to dst for chaining
 */
Mat4* Mat4::add(const Mat4& m1, const Mat4& m2, Mat4* dst) {
    float* m = dst->m;
    for(size_t ii = 0; ii < 16; ii++) {
        m[ii]  = m1.m[ii]  + m2.m[ii];
    }
    return dst;
}

/**
 * Adds the specified matrices and stores the result in dst.
 *
 * This method assumes the float arrays are in column major order.
 *
 * @param m1    The first matrix in column-major order
 * @param m2    The second matrix in column-major order
 * @param dst   The destination matrix in column-major order
 *
 * @return A reference to dst for chaining
 */
float* Mat4::add(const float* m1, const float* m2, float* dst) {
    for(size_t ii = 0; ii < 16; ii++) {
        dst[ii]  = m1[ii]  + m2[ii];
    }
    return dst;
}

/**
 * Subtracts a scalar from each component of mat and stores the result in dst.
 *
 * @param mat       The matrix to subtract from.
 * @param scalar    The scalar value to subtract.
 * @param dst       A matrix to store the result in.
 *
 * @return A reference to dst for chaining
 */
Mat4* Mat4::subtract(const Mat4& mat, float scalar, Mat4* dst) {
    float* m = dst->m;
    for(size_t ii = 0; ii < 16; ii++) {
        m[ii]  = mat.m[ii] - scalar;
    }
    return dst;
}

/**
 * Subtracts a scalar from each component of mat and stores the result in dst.
 *
 * This method assumes the float arrays are in column major order.
 *
 * @param mat       The matrix to subtract from in column major order
 * @param scalar    The scalar value to subtract in column major order
 * @param dst       A matrix to store the result in column major order
 *
 * @return A reference to dst for chaining
 */
float* Mat4::subtract(const float* mat, float scalar, float* dst) {
    for(size_t ii = 0; ii < 16; ii++) {
        dst[ii]  = mat[ii] - scalar;
    }
    return dst;
}

/**
 * Subtracts the matrix m2 from m1 and stores the result in dst.
 *
 * @param m1    The first matrix.
 * @param m2    The second matrix.
 * @param dst   A matrix to store the result in.
 *
 * @return A reference to dst for chaining
 */
Mat4* Mat4::subtract(const Mat4& m1, const Mat4& m2, Mat4* dst) {
    float* m = dst->m;
    for(size_t ii = 0; ii < 16; ii++) {
        m[ii]  = m1.m[ii] - m2.m[ii];
    }
    return dst;
}

/**
 * Subtracts the matrix m2 from m1 and stores the result in dst.
 *
 * This method assumes the float arrays are in column major order.
 *
 * @param m1    The first matrix in column-major order
 * @param m2    The second matrix in column-major order
 * @param dst   The destination matrix in column-major order
 *
 * @return A reference to dst for chaining
 */
float* Mat4::subtract(const float* m1, const float* m2, float* dst) {
    for(size_t ii = 0; ii < 16; ii++) {
        dst[ii]  = m1[ii] - m2[ii];
    }
    return dst;
}

/**
 * Multiplies the specified matrix by a scalar and stores the result in dst.
 *
 * @param mat       The matrix.
 * @param scalar    The scalar value.
 * @param dst       A matrix to store the result in.
 *
 * @return A reference to dst for chaining
 */
Mat4* Mat4::multiply(const Mat4& mat, float scalar, Mat4* dst) {
    float* m = dst->m;
    for(size_t ii = 0; ii < 16; ii++) {
        m[ii]  = mat.m[ii] * scalar;
    }
    return dst;
}

/**
  * Multiplies the specified matrix by a scalar and stores the result in dst.
  *
  * This method assumes the float arrays are in column major order.
  *
  * @param mat       The matrix in column-major order
  * @param scalar    The scalar value.
  * @param dst       A matrix to store the result in column-major order
  *
  * @return A reference to dst for chaining
  */
float* Mat4::multiply(const float* mat, float scalar, float* dst) {
    for(size_t ii = 0; ii < 16; ii++) {
        dst[ii]  = mat[ii] * scalar;
    }
    return dst;
}

/**
 * Multiplies m1 by the matrix m2 and stores the result in dst.
 *
 * The matrix m2 is on the right.  This means that it corresponds to
 * an subsequent transform, when looking at a sequence of transforms.
 *
 * @param m1    The first matrix to multiply.
 * @param m2    The second matrix to multiply.
 * @param dst   A matrix to store the result in.
 *
 * @return A reference to dst for chaining
 */
Mat4* Mat4::multiply(const Mat4& m1, const Mat4& m2, Mat4* dst) {
    float product[16];
    product[0]  = m2.m[0] * m1.m[0]  + m2.m[4] * m1.m[1] + m2.m[8]   * m1.m[2]  + m2.m[12] * m1.m[3];
    product[1]  = m2.m[1] * m1.m[0]  + m2.m[5] * m1.m[1] + m2.m[9]   * m1.m[2]  + m2.m[13] * m1.m[3];
    product[2]  = m2.m[2] * m1.m[0]  + m2.m[6] * m1.m[1] + m2.m[10]  * m1.m[2]  + m2.m[14] * m1.m[3];
    product[3]  = m2.m[3] * m1.m[0]  + m2.m[7] * m1.m[1] + m2.m[11]  * m1.m[2]  + m2.m[15] * m1.m[3];
    
    product[4]  = m2.m[0] * m1.m[4]  + m2.m[4] * m1.m[5] + m2.m[8]   * m1.m[6]  + m2.m[12] * m1.m[7];
    product[5]  = m2.m[1] * m1.m[4]  + m2.m[5] * m1.m[5] + m2.m[9]   * m1.m[6]  + m2.m[13] * m1.m[7];
    product[6]  = m2.m[2] * m1.m[4]  + m2.m[6] * m1.m[5] + m2.m[10]  * m1.m[6]  + m2.m[14] * m1.m[7];
    product[7]  = m2.m[3] * m1.m[4]  + m2.m[7] * m1.m[5] + m2.m[11]  * m1.m[6]  + m2.m[15] * m1.m[7];
    
    product[8]  = m2.m[0] * m1.m[8]  + m2.m[4] * m1.m[9] + m2.m[8]   * m1.m[10] + m2.m[12] * m1.m[11];
    product[9]  = m2.m[1] * m1.m[8]  + m2.m[5] * m1.m[9] + m2.m[9]   * m1.m[10] + m2.m[13] * m1.m[11];
    product[10] = m2.m[2] * m1.m[8]  + m2.m[6] * m1.m[9] + m2.m[10]  * m1.m[10] + m2.m[14] * m1.m[11];
    product[11] = m2.m[3] * m1.m[8]  + m2.m[7] * m1.m[9] + m2.m[11]  * m1.m[10] + m2.m[15] * m1.m[11];
    
    product[12] = m2.m[0] * m1.m[12] + m2.m[4] * m1.m[13] + m2.m[8]  * m1.m[14] + m2.m[12] * m1.m[15];
    product[13] = m2.m[1] * m1.m[12] + m2.m[5] * m1.m[13] + m2.m[9]  * m1.m[14] + m2.m[13] * m1.m[15];
    product[14] = m2.m[2] * m1.m[12] + m2.m[6] * m1.m[13] + m2.m[10] * m1.m[14] + m2.m[14] * m1.m[15];
    product[15] = m2.m[3] * m1.m[12] + m2.m[7] * m1.m[13] + m2.m[11] * m1.m[14] + m2.m[15] * m1.m[15];
    
    std::memcpy(&(dst->m[0]), &(product[0]), MATRIX_SIZE);
    return dst;
}

/**
 * Multiplies m1 by the matrix m2 and stores the result in dst.
 *
 * The matrix m2 is on the right.  This means that it corresponds to
 * an subsequent transform, when looking at a sequence of transforms.
 *
 * This method assumes the float arrays are in column major order.
 *
 * @param m1    The first matrix to multiply in column-major order
 * @param m2    The second matrix to multiply in column-major order
 * @param dst   A matrix to store the result in column-major order
 *
 * @return A reference to dst for chaining
 */
float* Mat4::multiply(const float* m1, const float* m2, float* dst) {
    float product[16];
    product[0]  = m2[0] * m1[0]  + m2[4] * m1[1] + m2[8]   * m1[2]  + m2[12] * m1[3];
    product[1]  = m2[1] * m1[0]  + m2[5] * m1[1] + m2[9]   * m1[2]  + m2[13] * m1[3];
    product[2]  = m2[2] * m1[0]  + m2[6] * m1[1] + m2[10]  * m1[2]  + m2[14] * m1[3];
    product[3]  = m2[3] * m1[0]  + m2[7] * m1[1] + m2[11]  * m1[2]  + m2[15] * m1[3];
    
    product[4]  = m2[0] * m1[4]  + m2[4] * m1[5] + m2[8]   * m1[6]  + m2[12] * m1[7];
    product[5]  = m2[1] * m1[4]  + m2[5] * m1[5] + m2[9]   * m1[6]  + m2[13] * m1[7];
    product[6]  = m2[2] * m1[4]  + m2[6] * m1[5] + m2[10]  * m1[6]  + m2[14] * m1[7];
    product[7]  = m2[3] * m1[4]  + m2[7] * m1[5] + m2[11]  * m1[6]  + m2[15] * m1[7];
    
    product[8]  = m2[0] * m1[8]  + m2[4] * m1[9] + m2[8]   * m1[10] + m2[12] * m1[11];
    product[9]  = m2[1] * m1[8]  + m2[5] * m1[9] + m2[9]   * m1[10] + m2[13] * m1[11];
    product[10] = m2[2] * m1[8]  + m2[6] * m1[9] + m2[10]  * m1[10] + m2[14] * m1[11];
    product[11] = m2[3] * m1[8]  + m2[7] * m1[9] + m2[11]  * m1[10] + m2[15] * m1[11];
    
    product[12] = m2[0] * m1[12] + m2[4] * m1[13] + m2[8]  * m1[14] + m2[12] * m1[15];
    product[13] = m2[1] * m1[12] + m2[5] * m1[13] + m2[9]  * m1[14] + m2[13] * m1[15];
    product[14] = m2[2] * m1[12] + m2[6] * m1[13] + m2[10] * m1[14] + m2[14] * m1[15];
    product[15] = m2[3] * m1[12] + m2[7] * m1[13] + m2[11] * m1[14] + m2[15] * m1[15];
    
    std::memcpy(dst, &(product[0]), MATRIX_SIZE);
    return dst;
}

/**
 * Negates m1 and stores the result in dst.
 *
 * @param m1    The matrix to negate.
 * @param dst   A matrix to store the result in.
 *
 * @return A reference to dst for chaining
 */
Mat4* Mat4::negate(const Mat4& m1, Mat4* dst) {
    float* m = dst->m;
    for(size_t ii = 0; ii < 16; ii++) {
        m[ii]  = -m1.m[ii];
    }
    return dst;
}

/**
 * Negates m1 and stores the result in dst.
 *
 * This method assumes the float arrays are in column major order.
 *
 * @param m1    The matrix to negate in column major order
 * @param dst   A matrix to store the result in column major order
 *
 * @return A reference to dst for chaining
 */
float* Mat4::negate(const float* m1, float* dst) {
    for(size_t ii = 0; ii < 16; ii++) {
        dst[ii]  = -m1[ii];
    }
    return dst;
}

/**
 * Transposes m1 and stores the result in dst.
 *
 * Transposing a matrix swaps columns and rows. This allows to transform
 * a vector by multiplying it on the left. If the matrix is orthonormal,
 * this is also the inverse.
 *
 * @param m1    The matrix to negate.
 * @param dst   A matrix to store the result in.
 *
 * @return A reference to dst for chaining
 */
Mat4* Mat4::transpose(const Mat4& m1, Mat4* dst) {
    float t[16] = {
        m1.m[0], m1.m[4], m1.m[8],  m1.m[12],
        m1.m[1], m1.m[5], m1.m[9],  m1.m[13],
        m1.m[2], m1.m[6], m1.m[10], m1.m[14],
        m1.m[3], m1.m[7], m1.m[11], m1.m[15]
    };
    std::memcpy(&(dst->m[0]), &(t[0]), MATRIX_SIZE);
    return dst;
}

/**
 * Transposes m1 and stores the result in dst.
 *
 * Transposing a matrix swaps columns and rows. This allows to transform
 * a vector by multiplying it on the left. If the matrix is orthonormal,
 * this is also the inverse.
 *
 * This method assumes the float arrays are in column major order.
 *
 * @param m1    The matrix to transpose in column major order
 * @param dst   A matrix to store the result in column major order
 *
 * @return A reference to dst for chaining
 */
float* Mat4::transpose(const float* m1, float* dst) {
    float t[16] = {
        m1[0], m1[4], m1[8],  m1[12],
        m1[1], m1[5], m1[9],  m1[13],
        m1[2], m1[6], m1[10], m1[14],
        m1[3], m1[7], m1[11], m1[15]
    };
    std::memcpy(dst, t, MATRIX_SIZE);
    return dst;
}



#pragma mark -
#pragma mark Static Matrix Operations
/**
 * Inverts m1 and stores the result in dst.
 *
 * If the matrix cannot be inverted, this method stores the zero matrix
 * in dst.
 *
 * @param m1    The matrix to negate.
 * @param dst   A matrix to store the result in.
 *
 * @return A reference to dst for chaining
 */
Mat4* Mat4::invert(const Mat4& m1, Mat4* dst) {
    float a0 = m1.m[0]  * m1.m[5] -  m1.m[1]  * m1.m[4];
    float a1 = m1.m[0]  * m1.m[6] -  m1.m[2]  * m1.m[4];
    float a2 = m1.m[0]  * m1.m[7] -  m1.m[3]  * m1.m[4];
    float a3 = m1.m[1]  * m1.m[6] -  m1.m[2]  * m1.m[5];
    float a4 = m1.m[1]  * m1.m[7] -  m1.m[3]  * m1.m[5];
    float a5 = m1.m[2]  * m1.m[7] -  m1.m[3]  * m1.m[6];
    float b0 = m1.m[8]  * m1.m[13] - m1.m[9]  * m1.m[12];
    float b1 = m1.m[8]  * m1.m[14] - m1.m[10] * m1.m[12];
    float b2 = m1.m[8]  * m1.m[15] - m1.m[11] * m1.m[12];
    float b3 = m1.m[9]  * m1.m[14] - m1.m[10] * m1.m[13];
    float b4 = m1.m[9]  * m1.m[15] - m1.m[11] * m1.m[13];
    float b5 = m1.m[10] * m1.m[15] - m1.m[11] * m1.m[14];
    
    // Calculate the determinant.
    float det = a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;
    
    // Close to zero, can't invert.
    if (fabs(det) <= CU_MATH_FLOAT_SMALL) {
        dst->setZero();
        return dst;
    }
    
    // Support the case where m1 == dst.
    Mat4 inverse;
    inverse.m[0]  =  m1.m[5]  * b5 - m1.m[6]  * b4 + m1.m[7]  * b3;
    inverse.m[1]  = -m1.m[1]  * b5 + m1.m[2]  * b4 - m1.m[3]  * b3;
    inverse.m[2]  =  m1.m[13] * a5 - m1.m[14] * a4 + m1.m[15] * a3;
    inverse.m[3]  = -m1.m[9]  * a5 + m1.m[10] * a4 - m1.m[11] * a3;
    
    inverse.m[4]  = -m1.m[4]  * b5 + m1.m[6]  * b2 - m1.m[7]  * b1;
    inverse.m[5]  =  m1.m[0]  * b5 - m1.m[2]  * b2 + m1.m[3]  * b1;
    inverse.m[6]  = -m1.m[12] * a5 + m1.m[14] * a2 - m1.m[15] * a1;
    inverse.m[7]  =  m1.m[8]  * a5 - m1.m[10] * a2 + m1.m[11] * a1;
    
    inverse.m[8]  =  m1.m[4]  * b4 - m1.m[5]  * b2 + m1.m[7]  * b0;
    inverse.m[9]  = -m1.m[0]  * b4 + m1.m[1]  * b2 - m1.m[3]  * b0;
    inverse.m[10] =  m1.m[12] * a4 - m1.m[13] * a2 + m1.m[15] * a0;
    inverse.m[11] = -m1.m[8]  * a4 + m1.m[9]  * a2 - m1.m[11] * a0;
    
    inverse.m[12] = -m1.m[4]  * b3 + m1.m[5]  * b1 - m1.m[6]  * b0;
    inverse.m[13] =  m1.m[0]  * b3 - m1.m[1]  * b1 + m1.m[2]  * b0;
    inverse.m[14] = -m1.m[12] * a3 + m1.m[13] * a1 - m1.m[14] * a0;
    inverse.m[15] =  m1.m[8]  * a3 - m1.m[9]  * a1 + m1.m[10] * a0;
    
    multiply(inverse, 1.0f / det, dst);

    return dst;
}

/**
 * Inverts m1 and stores the result in dst.
 *
 * If the matrix cannot be inverted, this method stores the zero matrix
 * in dst.
 *
 * This method assumes the float arrays are in column major order.
 *
 * @param m1    The matrix to invert in column major order
 * @param dst   A matrix to store the result in column major order
 *
 * @return A reference to dst for chaining
 */
float* Mat4::invert(const float* m1, float* dst) {
    float a0 = m1[0]  * m1[5] -  m1[1]  * m1[4];
    float a1 = m1[0]  * m1[6] -  m1[2]  * m1[4];
    float a2 = m1[0]  * m1[7] -  m1[3]  * m1[4];
    float a3 = m1[1]  * m1[6] -  m1[2]  * m1[5];
    float a4 = m1[1]  * m1[7] -  m1[3]  * m1[5];
    float a5 = m1[2]  * m1[7] -  m1[3]  * m1[6];
    float b0 = m1[8]  * m1[13] - m1[9]  * m1[12];
    float b1 = m1[8]  * m1[14] - m1[10] * m1[12];
    float b2 = m1[8]  * m1[15] - m1[11] * m1[12];
    float b3 = m1[9]  * m1[14] - m1[10] * m1[13];
    float b4 = m1[9]  * m1[15] - m1[11] * m1[13];
    float b5 = m1[10] * m1[15] - m1[11] * m1[14];
    
    // Calculate the determinant.
    float det = a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;
    
    // Close to zero, can't invert.
    if (fabs(det) <= CU_MATH_FLOAT_SMALL) {
        std::memset(dst, 0, 16*sizeof(float));
        return dst;
    }
    
    // Support the case where m1 == dst.
    float inverse[16];
    inverse[0]  =  m1[5]  * b5 - m1[6]  * b4 + m1[7]  * b3;
    inverse[1]  = -m1[1]  * b5 + m1[2]  * b4 - m1[3]  * b3;
    inverse[2]  =  m1[13] * a5 - m1[14] * a4 + m1[15] * a3;
    inverse[3]  = -m1[9]  * a5 + m1[10] * a4 - m1[11] * a3;
    
    inverse[4]  = -m1[4]  * b5 + m1[6]  * b2 - m1[7]  * b1;
    inverse[5]  =  m1[0]  * b5 - m1[2]  * b2 + m1[3]  * b1;
    inverse[6]  = -m1[12] * a5 + m1[14] * a2 - m1[15] * a1;
    inverse[7]  =  m1[8]  * a5 - m1[10] * a2 + m1[11] * a1;
    
    inverse[8]  =  m1[4]  * b4 - m1[5]  * b2 + m1[7]  * b0;
    inverse[9]  = -m1[0]  * b4 + m1[1]  * b2 - m1[3]  * b0;
    inverse[10] =  m1[12] * a4 - m1[13] * a2 + m1[15] * a0;
    inverse[11] = -m1[8]  * a4 + m1[9]  * a2 - m1[11] * a0;
    
    inverse[12] = -m1[4]  * b3 + m1[5]  * b1 - m1[6]  * b0;
    inverse[13] =  m1[0]  * b3 - m1[1]  * b1 + m1[2]  * b0;
    inverse[14] = -m1[12] * a3 + m1[13] * a1 - m1[14] * a0;
    inverse[15] =  m1[8]  * a3 - m1[9]  * a1 + m1[10] * a0;
    
    multiply(inverse, 1.0f / det, dst);
    return dst;
}

/**
 * Transforms the rectangle by the given matrix, and stores the result in dst.
 *
 * This method transforms the four defining points of the rectangle.  It
 * then computes the minimal bounding box storing these four points.
 *
 * @param mat   The transform matrix.
 * @param rect  The rect to transform.
 * @param dst   A rect to store the transformed rectangle in.
 *
 * @return A reference to dst for chaining
 */
cugl::Rect* Mat4::transform(const Mat4& mat, const Rect rect, Rect* dst) {
    Vec2 input1(rect.getMinX(),rect.getMinY());
    Vec2 input2(rect.getMinX(),rect.getMaxY());
    Vec2 input3(rect.getMaxX(),rect.getMinY());
    Vec2 input4(rect.getMaxX(),rect.getMaxY());
    Mat4::transform(mat, input1, &input1);
    Mat4::transform(mat, input2, &input2);
    Mat4::transform(mat, input3, &input3);
    Mat4::transform(mat, input4, &input4);
    float minx = std::min(input1.x,std::min(input2.x,std::min(input3.x,input4.x)));
    float maxx = std::max(input1.x,std::max(input2.x,std::max(input3.x,input4.x)));
    float miny = std::min(input1.y,std::min(input2.y,std::min(input3.y,input4.y)));
    float maxy = std::max(input1.y,std::max(input2.y,std::max(input3.y,input4.y)));

    dst->origin.set(minx,miny);
    dst->size.set(maxx-minx,maxy-miny);
    return dst;
}

/**
 * Decomposes the scale, rotation and translation components of the given matrix.
 *
 * To work properly, the matrix must have been constructed in the following
 * order: scale, then rotate, then translation.  While the rotation matrix
 * will always be correct, the scale and translation are not guaranteed
 * to be correct if this is violated.
 *
 * If any pointer is null, the method simply does not assign that result.
 * However, it will still continue to compute the component with non-null
 * vectors to store the result.
 *
 * If the scale component is too small, then it may be impossible to
 * extract the rotation. In that case, if the rotation pointer is not
 * null, this method will return false.
 *
 * @param mat   The matrix to decompose.
 * @param scale The scale component.
 * @param rot   The rotation component.
 * @param trans The translation component.
 *
 * @return true if all requested components were properly extracted
 */
bool Mat4::decompose(const Mat4& mat, Vec3* scale, Quaternion* rot, Vec3* trans) {
    if (trans != nullptr) {
        // Extract the translation.
        trans->x = mat.m[12];
        trans->y = mat.m[13];
        trans->z = mat.m[14];
    }
    
    // Nothing left to do.
    if (scale == nullptr && rot == nullptr) {
        return true;
    }
    
    // Extract the scale.  Using Vec4 guarantees optimization
    // This is simply the length of each axis (row/column) in the matrix.
    Vec4 xaxis(mat.m[0], mat.m[1], mat.m[2],0.0f);
    float scaleX = xaxis.length();
    
    Vec4 yaxis(mat.m[4], mat.m[5], mat.m[6],0.0f);
    float scaleY = yaxis.length();
    
    Vec4 zaxis(mat.m[8], mat.m[9], mat.m[10],0.0f);
    float scaleZ = zaxis.length();
    
    // Determine if we have a negative scale (true if determinant is less than zero).
    // In this case, we simply negate a single axis of the scale.
    float det = mat.getDeterminant();
    if (det < 0) {
        scaleZ = -scaleZ;
    }
    
    if (scale != nullptr) {
        scale->x = scaleX;
        scale->y = scaleY;
        scale->z = scaleZ;
    }
    
    // Nothing left to do.
    if (rot == nullptr) {
        return true;
    }
    
    // Scale too close to zero, can't decompose rotation.
    if (scaleX < CU_MATH_EPSILON || scaleY < CU_MATH_EPSILON || fabs(scaleZ) < CU_MATH_EPSILON) {
        return false;
    }

    float rn;
    
    // Factor the scale out of the matrix axes.
    rn = 1.0f / scaleX;
    xaxis *= rn;
    
    rn = 1.0f / scaleY;
    yaxis *= rn;
    
    rn = 1.0f / scaleZ;
    zaxis *= rn;

    // Now calculate the rotation from the resulting matrix (axes).
    float trace = xaxis.x + yaxis.y + zaxis.z + 1.0f;
    
    if (trace > CU_MATH_EPSILON) {
        float s = 0.5f / sqrt(trace);
        rot->w = 0.25f / s;
        rot->x = (yaxis.z - zaxis.y) * s;
        rot->y = (zaxis.x - xaxis.z) * s;
        rot->z = (xaxis.y - yaxis.x) * s;
    } else {
        // Note: since xaxis, yaxis, and zaxis are normalized,
        // we will never divide by zero in the code below.
        if (xaxis.x > yaxis.y && xaxis.x > zaxis.z) {
            float s = 0.5f / sqrt(1.0f + xaxis.x - yaxis.y - zaxis.z);
            rot->w = (yaxis.z - zaxis.y) * s;
            rot->x = 0.25f / s;
            rot->y = (yaxis.x + xaxis.y) * s;
            rot->z = (zaxis.x + xaxis.z) * s;
        } else if (yaxis.y > zaxis.z) {
            float s = 0.5f / sqrt(1.0f + yaxis.y - xaxis.x - zaxis.z);
            rot->w = (zaxis.x - xaxis.z) * s;
            rot->x = (yaxis.x + xaxis.y) * s;
            rot->y = 0.25f / s;
            rot->z = (zaxis.y + yaxis.z) * s;
        } else {
            float s = 0.5f / sqrt(1.0f + zaxis.z - xaxis.x - yaxis.y );
            rot->w = (xaxis.y - yaxis.x ) * s;
            rot->x = (zaxis.x + xaxis.z ) * s;
            rot->y = (zaxis.y + yaxis.z ) * s;
            rot->z = 0.25f / s;
        }
    }

    return true;
}

#pragma mark -
#pragma mark Static Vector Operations
/**
 * Transforms the point by the given matrix, and stores the result in dst.
 *
 * The vector is treated as a point, which means that translation is
 * applied to the result.
 *
 * @param mat   The transform matrix.
 * @param point The point to transform.
 * @param dst   A vector to store the transformed point in.
 *
 * @return A reference to dst for chaining
 */
Vec2* Mat4::transform(const Mat4& mat, const Vec2 point, Vec2* dst) {
    CUAssertLog(dst, "Destination vector is null");
    Vec4 temp(point);
    transform(mat,temp,&temp);
    dst->x = temp.x; dst->y = temp.y;
    return dst;
}

/**
 * Transforms the point by the given matrix, and stores the result in dst.
 *
 * The vector is treated as a point, which means that translation is
 * applied to the result.  The destination array will be assigned only
 * 2 elements.
 *
 * @param mat   The transform matrix.
 * @param point The point to transform.
 * @param dst   An array to store the transformed point.
 *
 * @return A reference to dst for chaining
 */
float* Mat4::transform(const Mat4& mat, const Vec2 point, float* dst) {
    CUAssertLog(dst, "Destination vector is null");
    Vec4 temp(point);
    transform(mat,temp,&temp);
    dst[0] = temp.x; dst[1] = temp.y;
    return dst;
}
/**
 * Transforms the vector by the given matrix, and stores the result in dst.
 *
 * The vector is treated as a direction, which means that translation is
 * not applied to the result.
 *
 * @param mat   The transform matrix.
 * @param vec   The vector to transform.
 * @param dst   A vector to store the transformed point in.
 *
 * @return A reference to dst for chaining
 */
Vec2* Mat4::transformVector(const Mat4& mat, const Vec2 vec, Vec2* dst) {
    CUAssertLog(dst, "Destination vector is null");
    Vec4 temp(vec.x,vec.y,0.0f,0.0f);
    transform(mat,temp,&temp);
    dst->x = temp.x; dst->y = temp.y;
    return dst;
}

/**
 * Transforms the point by the given matrix, and stores the result in dst.
 *
 * The vector is treated as a point, which means that translation is
 * applied to the result.
 *
 * @param mat   The transform matrix.
 * @param point The point to transform.
 * @param dst   A vector to store the transformed point in.
 *
 * @return A reference to dst for chaining
 */
Vec3* Mat4::transform(const Mat4& mat, const Vec3 point, Vec3* dst) {
    CUAssertLog(dst, "Destination vector is null");
    Vec4 temp(point);
    transform(mat,temp,&temp);
    dst->x = temp.x; dst->y = temp.y; dst->z = temp.z;
    return dst;
}

/**
 * Transforms the vector by the given matrix, and stores the result in dst.
 *
 * The vector is treated as a direction, which means that translation is
 * not applied to the result.
 *
 * @param mat   The transform matrix.
 * @param vec   The vector to transform.
 * @param dst   A vector to store the transformed point in.
 *
 * @return A reference to dst for chaining
 */
Vec3* Mat4::transformVector(const Mat4& mat, const Vec3 vec, Vec3* dst) {
    CUAssertLog(dst, "Destination vector is null");
    Vec4 temp(vec.x,vec.y,vec.z,0.0f);
    transform(mat,temp,&temp);
    dst->x = temp.x; dst->y = temp.y; dst->z = temp.z;
    return dst;
}

/**
 * Transforms the vector by the given matrix, and stores the result in dst.
 *
 * The vector is treated as is.  Hence whether or not translation is applied
 * depends on the value of w.
 *
 * @param mat   The transform matrix.
 * @param vec   The vector to transform.
 * @param dst   A vector to store the transformed point in.
 *
 * @return A reference to dst for chaining
 */
Vec4* Mat4::transform(const Mat4& mat, const Vec4 vec, Vec4* dst) {
    CUAssertLog(dst, "Destination vector is null");
    // Handle case where v == dst.
    float x = vec.x * mat.m[0] + vec.y * mat.m[4] + vec.z * mat.m[8]  + vec.w * mat.m[12];
    float y = vec.x * mat.m[1] + vec.y * mat.m[5] + vec.z * mat.m[9]  + vec.w * mat.m[13];
    float z = vec.x * mat.m[2] + vec.y * mat.m[6] + vec.z * mat.m[10] + vec.w * mat.m[14];
    float w = vec.x * mat.m[3] + vec.y * mat.m[7] + vec.z * mat.m[11] + vec.w * mat.m[15];
    
    dst->x = x;
    dst->y = y;
    dst->z = z;
    dst->w = w;

    return dst;
}

/**
 * Transforms the vector array by the given matrix, and stores the result in dst.
 *
 * The vector is array is treated as a list of 4 element vectors (@see Vec4).
 * The transform is applied in order and written to the output array.
 *
 * @param mat       The transform matrix.
 * @param input     The array of vectors to transform.
 * @param output    The array to store the transformed vectors.
 * @param size      The size of the two arrays.
 *
 * @return A reference to dst for chaining
 */
float* Mat4::transform(const Mat4& mat, float const* input, float* output, size_t size) {
    CUAssertLog(output, "Destination vector is null");
    for(size_t ii = 0; ii < size; ii++) {
        // Handle case where v == dst.
        float x = input[ii*4] * mat.m[0] + input[ii*4+1] * mat.m[4] + input[ii*4+2] * mat.m[8]  + input[ii*4+3] * mat.m[12];
        float y = input[ii*4] * mat.m[1] + input[ii*4+1] * mat.m[5] + input[ii*4+2] * mat.m[9]  + input[ii*4+3] * mat.m[13];
        float z = input[ii*4] * mat.m[2] + input[ii*4+1] * mat.m[6] + input[ii*4+2] * mat.m[10] + input[ii*4+3] * mat.m[14];
        float w = input[ii*4] * mat.m[3] + input[ii*4+1] * mat.m[7] + input[ii*4+2] * mat.m[11] + input[ii*4+3] * mat.m[15];
        
        output[ii*4  ] = x;
        output[ii*4+1] = y;
        output[ii*4+2] = z;
        output[ii*4+3] = w;
    }

    return output;
}

/**
 * Transforms the vector array by the given matrix, and stores the result in dst.
 *
 * The vector is array is treated as a list of 4 element vectors (@see Vec4).
 * The transform is applied in order and written to the output array.
 *
 * @param mat       The transform matrix in column major order
 * @param input     The array of vectors to transform.
 * @param output    The array to store the transformed vectors.
 * @param size      The size of the two arrays.
 *
 * @return A reference to dst for chaining
 */
float* Mat4::transform(const float* mat, float const* input, float* output, size_t size) {
    CUAssertLog(output, "Destination vector is null");
    for(size_t ii = 0; ii < size; ii++) {
        // Handle case where v == dst.
        float x = input[ii*4] * mat[0] + input[ii*4+1] * mat[4] + input[ii*4+2] * mat[8]  + input[ii*4+3] * mat[12];
        float y = input[ii*4] * mat[1] + input[ii*4+1] * mat[5] + input[ii*4+2] * mat[9]  + input[ii*4+3] * mat[13];
        float z = input[ii*4] * mat[2] + input[ii*4+1] * mat[6] + input[ii*4+2] * mat[10] + input[ii*4+3] * mat[14];
        float w = input[ii*4] * mat[3] + input[ii*4+1] * mat[7] + input[ii*4+2] * mat[11] + input[ii*4+3] * mat[15];
        
        output[ii*4  ] = x;
        output[ii*4+1] = y;
        output[ii*4+2] = z;
        output[ii*4+3] = w;
    }
    return output;
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
std::string Mat4::toString(bool verbose) const {
    std::stringstream ss;
    if (verbose) {
        ss << "cugl::Mat4";
    }
    const int PRECISION = 8;
    for(int ii = 0; ii < 4; ii++) {
        ss << "\n";
        ss << "|  ";
        ss << strtool::to_string(m[ii   ]).substr(0,PRECISION);
        ss << ", ";
        ss << strtool::to_string(m[ii+4 ]).substr(0,PRECISION);
        ss << ", ";
        ss << strtool::to_string(m[ii+8 ]).substr(0,PRECISION);
        ss << ", ";
        ss << strtool::to_string(m[ii+12]).substr(0,PRECISION);
        ss << "  |";
    }
    return ss.str();
}

/**
 * Cast from Mat4 to a Affine2.
 *
 * The z values are all uniformly ignored.  However, it the final element
 * of the matrix is not 1 (e.g. the translation has a w value of 1), then
 * it divides the entire matrix before creating the affine transform.
 *
 */
Mat4::operator Affine2() const {
    float v = 1.0f;
    if (m[15] != 1.0f && fabsf(m[15]) > CU_MATH_EPSILON) {
        v = 1.0f/m[15];
    }
    Affine2 result(m[0]*v,m[4]*v,m[1]*v,m[5]*v,m[12]*v,m[13]*v);
    return result;
}

/**
 * Creates a matrix from the given affine transform.
 *
 * The z values are set to the identity.
 *
 * @param aff The transform to convert
 */
Mat4::Mat4(const Affine2& aff) {
    set(aff);
}

/**
 * Sets the elements of this matrix to those of the given transform.
 *
 * The z values are set to the identity.
 *
 * @param aff The transform to convert
 *
 * @return A reference to this (modified) Mat4 for chaining.
 */
Mat4& Mat4::operator= (const Affine2& aff) {
    return set(aff);
}

/**
 * Sets the elements of this matrix to those of the given transform.
 *
 * The z values are set to the identity.
 *
 * @param aff The transform to convert
 *
 * @return A reference to this (modified) Mat4 for chaining.
 */
Mat4& Mat4::set(const Affine2& aff) {
    aff.get4x4(m);
    m[10] = 1.0f; // Trivial z
    return *this;
}

#pragma mark -
#pragma mark Constants

/** The identity matrix (ones on the diagonal) */
const Mat4 Mat4::IDENTITY(1.0f, 0.0f, 0.0f, 0.0f,
                          0.0f, 1.0f, 0.0f, 0.0f,
                          0.0f, 0.0f, 1.0f, 0.0f,
                          0.0f, 0.0f, 0.0f, 1.0f);

/** The matrix with all zeroes */
const Mat4 Mat4::ZERO(0.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 0.0f,
                      0.0f, 0.0f, 0.0f, 0.0f );

/** The matrix with all ones */
const Mat4 Mat4::ONE(1.0f, 1.0f, 1.0f, 1.0f,
                     1.0f, 1.0f, 1.0f, 1.0f,
                     1.0f, 1.0f, 1.0f, 1.0f,
                     1.0f, 1.0f, 1.0f, 1.0f );



