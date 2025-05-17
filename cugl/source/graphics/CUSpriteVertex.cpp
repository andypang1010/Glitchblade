//
//  CUSpriteVertex.h
//  Cornell University Game Library (CUGL)
//
//  This module provides the basic struct for the sprite batch pipeline.
//  These this is is meant to be passed by value, so we have no methods for
//  shared pointers.
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
#include <cugl/graphics/CUSpriteVertex.h>
#include <cugl/core/util/CUDebug.h>
#include <cugl/core/assets/CUJsonValue.h>
#include <cugl/core/math/CUColor4.h>

using namespace cugl;
using namespace cugl::graphics;

/**
 * Creates a new SpriteVertex from the given JSON value.
 *
 * A sprite vertex can be described as an array of floats or a JSON object.
 * If it is a JSON object, then it supports the following attributes.
 *
 *     "position":  An array of float arrays of length two
 *     "color":     Either a four-element integer array (values 0..255) or a string.
 *                  Any string should be a web color or a Tkinter color name.
 *     "texcoord":  An array of float arrays of length two
 *     "gradcoord": An array of float arrays of length two
 *
 * Again, all attributes are optional. The default color is 'white' and all
 * other values resolve to the origin.
 *
 * If the sprite vertex is represented as an array, then it should be an
 * array of length no more than 10. These float are assigned to the
 * attributes position (2), color (4), texCoord (2) and gradCoord (2) in
 * order. Missing values are replaced with a 0 (or 255 in the case of the
 * color attributes).
 *
 * @param json  The JSON object specifying the vertex
 */
SpriteVertex::SpriteVertex(const std::shared_ptr<JsonValue>& json) {
    set(json);
}

/**
 * Sets this SpriteVertex to have the data in the given JSON value.
 *
 * A sprite vertex can be described as an array of floats or a JSON object.
 * If it is a JSON object, then it supports the following attributes.
 *
 *     "position":  An array of float arrays of length two
 *     "color":     Either a four-element integer array (values 0..255) or a string.
 *                  Any string should be a web color or a Tkinter color name.
 *     "texcoord":  An array of float arrays of length two
 *     "gradcoord": An array of float arrays of length two
 *
 * Again, all attributes are optional. The default color is 'white' and all
 * other values resolve to the origin.
 *
 * If the sprite vertex is represented as an array, then it should be an
 * array of length no more than 10. Color attributes in this case should be
 * specified as values 0..1 instead of 0..255. These float are assigned to
 * the attributes position (2), color (4), texCoord (2) and gradCoord (2) in
 * that order. Missing values are replaced with a 0 (or 1 in the case of the
 * color attributes).
 *
 * @param json  The JSON object specifying the vertex
 *
 * @return a reference to this sprite vertex for chaining
 */
SpriteVertex& SpriteVertex::set(const std::shared_ptr<JsonValue>& json) {
    CUAssertLog(json->isArray() || json->isObject(), "Vertex %s is not valid",
                json->toString().c_str());
    if (json->isArray()) {
        float values[10];
        int pos = 0;
        for(int ii = 0; ii < json->size() && ii < 10; ii++) {
            values[pos++] = json->get(ii)->asFloat(0.0f);
        }
        for(int ii = pos; ii < 10; ii++) {
            if (ii >= 2 && ii <= 6) {
                values[ii] = 1.0f;
            } else {
                values[ii] = 0;
            }
        }
        position.set(values[0],values[1]);
        color = Color4f(values[2],values[3],values[4],values[5]).getPacked();
        texcoord.set(values[6],values[7]);
        gradcoord.set(values[8],values[9]);
    } else if (json->isObject()) {
        if (json->has("position")) {
            JsonValue* child = json->get("position").get();
            CUAssertLog(child->isArray(), "Vertex %s is not an array", child->toString().c_str());
            CUAssertLog(child->size() == 2, "Vertex %s does not have 2 elements", child->toString().c_str());
            position.x = child->get(0)->asFloat(0.0f);
            position.y = child->get(1)->asFloat(0.0f);
        }
        if (json->has("color")) {
            JsonValue* col = json->get("color").get();
            Color4 value(255,255,255,255);
             if (col->isString()) {
                 value.set(col->asString("#ffffff"));
             } else {
                 CUAssertLog(col->size() >= 4, "'color' must be a four element number array");
                 value.r = std::max(std::min(col->get(0)->asInt(0),255),0);
                 value.g = std::max(std::min(col->get(1)->asInt(0),255),0);
                 value.b = std::max(std::min(col->get(2)->asInt(0),255),0);
                 value.a = std::max(std::min(col->get(3)->asInt(0),255),0);
             }
            color = value.getPacked();
        } else {
            color = Color4::WHITE.getPacked();
        }
        if (json->has("texcoord")) {
            JsonValue* child = json->get("texcoord").get();
            CUAssertLog(child->isArray(), "Texture Coord %s is not an array",
                        child->toString().c_str());
            CUAssertLog(child->size() == 2, "Texture Coord %s does not have 2 elements",
                        child->toString().c_str());
            texcoord.x = child->get(0)->asFloat(0.0f);
            texcoord.y = child->get(1)->asFloat(0.0f);
        }
        if (json->has("gradcoord")) {
            JsonValue* child = json->get("gradcoord").get();
            CUAssertLog(child->isArray(), "Gradient Coord %s is not an array",
                        child->toString().c_str());
            CUAssertLog(child->size() == 2, "Gradient Coord %s does not have 2 elements",
                        child->toString().c_str());
            gradcoord.x = child->get(0)->asFloat(0.0f);
            gradcoord.y = child->get(1)->asFloat(0.0f);
        }
    }
    
    return *this;
}
