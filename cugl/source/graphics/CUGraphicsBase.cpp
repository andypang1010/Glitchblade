//
//  CUGraphicsBase.cpp
//  Cornell University Game Library (CUGL)
//
//  This file contains some simple OpenGL debugging tools, together with
//  functions for extracting OpenGL settings from JSON strings.
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
#include <cugl/graphics/CUGraphicsBase.h>
#include <cugl/core/assets/CUJsonValue.h>


#pragma mark Debugging Functions
/**
 * Returns a string description of an OpenGL error type
 *
 * @param type  The OpenGL error type
 *
 * @return a string description of an OpenGL error type
 */
std::string cugl::graphics::gl_error_name(GLenum type) {
    std::string error = "UNKNOWN";
    
    switch(type) {
        case 0:
            error="NO ERROR";
            break;
        case GL_INVALID_OPERATION:
            error="INVALID_OPERATION";
            break;
        case GL_INVALID_ENUM:
            error="INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error="INVALID_VALUE";
            break;
        case GL_OUT_OF_MEMORY:
            error="OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error="INVALID_FRAMEBUFFER_OPERATION";
            break;
        case GL_FRAMEBUFFER_UNDEFINED:
            error="FRAMEBUFFER_UNDEFINED";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            error="FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            error="FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
            break;
#if (CU_GL_PLATFORM == CU_GL_OPENGL)
        case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
            error="FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
            error="FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
            break;
#endif
        case GL_FRAMEBUFFER_UNSUPPORTED:
            error="FRAMEBUFFER_UNSUPPORTED";
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            error="FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
            break;
#if (CU_GL_PLATFORM == CU_GL_OPENGL)
        case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
            error="FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
            break;
#endif
    }
    return error;
}

/**
 * Returns a string description of an OpenGL data type
 *
 * @param type  The OpenGL error type
 *
 * @return a string description of an OpenGL data type
 */
std::string cugl::graphics::gl_type_name(GLenum type) {
    switch(type) {
        case GL_FLOAT:
            return "GL_FLOAT";
        case GL_FLOAT_VEC2:
            return "GL_FLOAT";
        case GL_FLOAT_VEC3:
            return "GL_FLOAT_VEC2";
        case GL_FLOAT_VEC4:
            return "GL_FLOAT_VEC4";
        case GL_FLOAT_MAT2:
            return "GL_FLOAT_MAT2";
        case GL_FLOAT_MAT3:
            return "GL_FLOAT_MAT3";
        case GL_FLOAT_MAT4:
            return "GL_FLOAT_MAT4";
        case GL_FLOAT_MAT2x3:
            return "GL_FLOAT_MAT2x3";
        case GL_FLOAT_MAT2x4:
            return "GL_FLOAT_MAT2x4";
        case GL_FLOAT_MAT3x2:
            return "GL_FLOAT_MAT3x2";
        case GL_FLOAT_MAT3x4:
            return "GL_FLOAT_MAT3x4";
        case GL_FLOAT_MAT4x2:
            return "GL_FLOAT_MAT4x2";
        case GL_FLOAT_MAT4x3:
            return "GL_FLOAT_MAT4x3";
        case GL_INT:
            return "GL_INT";
        case GL_INT_VEC2:
            return "GL_INT_VEC2";
        case GL_INT_VEC3:
            return "GL_INT_VEC3";
        case GL_INT_VEC4:
            return "GL_INT_VEC4";
        case GL_UNSIGNED_INT:
            return "GL_UNSIGNED_INT";
        case GL_UNSIGNED_INT_VEC2:
            return "GL_UNSIGNED_INT_VEC2";
        case GL_UNSIGNED_INT_VEC3:
            return "GL_UNSIGNED_INT_VEC3";
        case GL_UNSIGNED_INT_VEC4:
            return "GL_UNSIGNED_INT_VEC4";
#if (CU_GL_PLATFORM == CU_GL_OPENGL)
        case GL_DOUBLE:
            return "GL_GL_DOUBLE";
        case GL_DOUBLE_VEC2:
            return "GL_DOUBLE_VEC2";
        case GL_DOUBLE_VEC3:
            return "GL_DOUBLE_VEC3";
        case GL_DOUBLE_VEC4:
            return "GL_DOUBLE_VEC4";
        case GL_DOUBLE_MAT2:
            return "GL_DOUBLE_MAT2";
        case GL_DOUBLE_MAT3:
            return "GL_DOUBLE_MAT3";
        case GL_DOUBLE_MAT4:
            return "GL_DOUBLE_MAT4";
        case GL_DOUBLE_MAT2x3:
            return "GL_DOUBLE_MAT2x3";
        case GL_DOUBLE_MAT2x4:
            return "GL_DOUBLE_MAT2x4";
        case GL_DOUBLE_MAT3x2:
            return "GL_DOUBLE_MAT3x2";
        case GL_DOUBLE_MAT3x4:
            return "GL_DOUBLE_MAT3x4";
        case GL_DOUBLE_MAT4x2:
            return "GL_DOUBLE_MAT4x2";
        case GL_DOUBLE_MAT4x3:
            return "GL_DOUBLE_MAT4x3";
#endif
        case GL_SAMPLER_2D:
            return "GL_SAMPLER_2D";
        case GL_SAMPLER_3D:
            return "GL_SAMPLER_3D";
        case GL_SAMPLER_CUBE:
            return "GL_SAMPLER_CUBE";
        case GL_SAMPLER_2D_SHADOW:
            return "GL_SAMPLER_2D_SHADOW";
        case GL_UNIFORM_BUFFER:
            return "GL_UNIFORM_BUFFER";
    }
    return "GL_UNKNOWN";
}


#pragma mark Decoding Functions
/**
 * Returns the OpenGL enum for the given texture filter name
 *
 * This function converts JSON directory entries into OpenGL values. If the
 * name is invalid, it returns GL_LINEAR.
 *
 * @param name  The JSON name for the texture filter
 *
 * @return the OpenGL enum for the given texture filter name
 */
GLuint cugl::graphics::gl_filter(const std::string name) {
    if (name == "nearest") {
        return GL_NEAREST;
    } else if (name == "linear") {
        return GL_LINEAR;
    } else if (name == "nearest-nearest") {
        return GL_NEAREST_MIPMAP_NEAREST;
    } else if (name == "linear-nearest") {
        return GL_LINEAR_MIPMAP_NEAREST;
    } else if (name == "nearest-linear") {
        return GL_NEAREST_MIPMAP_LINEAR;
    } else if (name == "linear-linear") {
        return GL_LINEAR_MIPMAP_LINEAR;
    }
    return GL_LINEAR;
}

/**
 * Returns the OpenGL enum for the given texture wrap name
 *
 * This function converts JSON directory entries into OpenGL values. If the
 * name is invalid, it returns GL_CLAMP_TO_EDGE.
 *
 * @param name  The JSON name for the texture wrap
 *
 * @return the OpenGL enum for the given texture wrap name
 */
GLuint cugl::graphics::gl_wrap(const std::string name) {
    if (name == "clamp") {
        return GL_CLAMP_TO_EDGE;
    } else if (name == "repeat") {
        return GL_REPEAT;
    } else if (name == "mirrored") {
        return GL_MIRRORED_REPEAT;
    }
    return GL_CLAMP_TO_EDGE;
}

/**
 * Returns the OpenGL blend equation for the given name
 *
 * This function converts JSON directory entries into OpenGL values. If the
 * name is invalid, it returns GL_FUNC_ADD.
 *
 * @param name  The JSON name for the blend equation
 *
 * @return the OpenGL blend equation for the given name
 */
GLenum cugl::graphics::gl_blend_eq(std::string value) {
    if (value == "GL_FUNC_SUBTRACT") {
        return GL_FUNC_SUBTRACT;
    } else if (value == "GL_FUNC_REVERSE_SUBTRACT") {
        return GL_FUNC_REVERSE_SUBTRACT;
    } else if (value == "GL_MAX") {
        return GL_MAX;
    } else if (value == "GL_MIN") {
        return GL_MIN;
    }
    return GL_FUNC_ADD;
}

/**
 * Returns the OpenGL blend function for the given name
 *
 * This function converts JSON directory entries into OpenGL values. If the
 * name is invalid, it returns GL_ZERO.
 *
 * @param name  The JSON name for the blend function
 *
 * @return the OpenGL blend function for the given name
 */
GLenum cugl::graphics::gl_blend_func(std::string value) {
    if (value == "GL_SRC_COLOR") {
        return GL_SRC_COLOR;
    } else if (value == "GL_ONE_MINUS_SRC_COLOR") {
        return GL_ONE_MINUS_SRC_COLOR;
    } else if (value == "GL_ONE_MINUS_SRC_COLOR") {
        return GL_ONE_MINUS_SRC_COLOR;
    } else if (value == "GL_DST_COLOR") {
        return GL_DST_COLOR;
    } else if (value == "GL_ONE_MINUS_DST_COLOR") {
        return GL_ONE_MINUS_DST_COLOR;
    } else if (value == "GL_SRC_ALPHA") {
        return GL_SRC_ALPHA;
    } else if (value == "GL_ONE_MINUS_SRC_ALPHA") {
        return GL_ONE_MINUS_SRC_ALPHA;
    } else if (value == "GL_DST_ALPHA") {
        return GL_DST_ALPHA;
    } else if (value == "GL_ONE_MINUS_DST_ALPHA") {
        return GL_ONE_MINUS_DST_ALPHA;
    } else if (value == "GL_ONE") {
        return GL_ONE;
    } else if (value == "GL_CONSTANT_COLOR") {
        return GL_CONSTANT_COLOR;
    } else if (value == "GL_ONE_MINUS_CONSTANT_COLOR") {
        return GL_ONE_MINUS_CONSTANT_COLOR;
    } else if (value == "GL_CONSTANT_ALPHA") {
        return GL_CONSTANT_ALPHA;
    } else if (value == "GL_ONE_MINUS_CONSTANT_ALPHA") {
        return GL_ONE_MINUS_CONSTANT_ALPHA;
    }
    return GL_ZERO;
}


/**
 * Returns the color value for the given JSON entry
 *
 * A color entry is either a four-element integer array (values 0..255) or a
 * string. Any string should be a web color or a Tkinter color name.
 *
 * @param entry     The JSON entry for the color
 * @param backup    Default color to use on failure
 *
 * @return the color value for the given JSON entry
 */
cugl::Color4 cugl::graphics::json_color(JsonValue* entry, std::string backup) {
    Color4 result;
    result.set(backup);
    if (entry == nullptr) {
        return result;
    }
    
    if (entry->isString()) {
        result.set(entry->asString(backup));
    } else {
        CUAssertLog(entry->size() >= 4, "'color' must be a four element number array");
        result.r = std::max(std::min(entry->get(0)->asInt(0),255),0);
        result.g = std::max(std::min(entry->get(1)->asInt(0),255),0);
        result.b = std::max(std::min(entry->get(2)->asInt(0),255),0);
        result.a = std::max(std::min(entry->get(3)->asInt(0),255),0);
    }
    return result;
}
