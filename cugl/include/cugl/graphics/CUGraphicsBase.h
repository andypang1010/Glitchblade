//
//  CUGraphicsBase.h
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
#ifndef __CU_GRAPHICS_BASE_H__
#define __CU_GRAPHICS_BASE_H__
#include <cugl/core/CUBase.h>
#include <cugl/core/math/CUColor4.h>

// OpenGL support
/** Support for standard OpenGL   */
#define CU_GL_OPENGL   0
/** Support for standard OpenGLES */
#define CU_GL_OPENGLES 1

// Load the libraries and define the platform
#if defined (__IPHONEOS__)
    #include <OpenGLES/ES3/gl.h>
    #include <OpenGLES/ES3/glext.h>
    /** The current OpenGL platform */
    #define CU_GL_PLATFORM   CU_GL_OPENGLES
#elif defined (__ANDROID__)
    #include <GLES3/gl3platform.h>
    #include <GLES3/gl3.h>
    #include <GLES3/gl3ext.h>
    /** The current OpenGL platform */
    #define CU_GL_PLATFORM   CU_GL_OPENGLES
#elif defined (__MACOSX__)
    #include <OpenGL/OpenGL.h>
    #include <OpenGL/gl3.h>
    #include <OpenGL/gl3ext.h>
    /** The current OpenGL platform */
    #define CU_GL_PLATFORM   CU_GL_OPENGL
#elif defined (__WINDOWS__)
    #include <GL/glew.h>
    #include <SDL_opengl.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
    /** The current OpenGL platform */
    #define CU_GL_PLATFORM   CU_GL_OPENGL
#elif defined (__LINUX__)
    #include <GL/glew.h>
    #include <SDL_opengl.h>
    #include <GL/gl.h>
    #include <GL/glu.h>
    #include <GL/glut.h>
    /** The current OpenGL platform */
    #define CU_GL_PLATFORM   CU_GL_OPENGL
#endif

namespace cugl {

// Forward declaration
class JsonValue;

    /**
     * The classes and functions needed to construct a graphics pipeline.
     *
     * Initially these were part of the core CUGL module (everyone wants graphics,
     * right). However, after student demand for a headless option that did not
     * have so many OpenGL dependencies, this was factored out.
     */
    namespace graphics {
/**
 * Returns a string description of an OpenGL error type
 *
 * @param error The OpenGL error type
 *
 * @return a string description of an OpenGL error type
 */
std::string gl_error_name(GLenum error);

/**
 * Returns a string description of an OpenGL data type
 *
 * @param error The OpenGL error type
 *
 * @return a string description of an OpenGL data type
 */
std::string gl_type_name(GLenum error);
    
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
GLuint gl_filter(const std::string name);

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
GLuint gl_wrap(const std::string name);

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
GLenum gl_blend_eq(std::string name);

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
GLenum gl_blend_func(std::string name);
    
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
Color4 json_color(JsonValue* entry, std::string backup);
    
    }
}
#endif /* __CU_GRAPHICS_BASE_H__ */
