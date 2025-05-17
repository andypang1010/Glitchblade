R"(////////// SHADER BEGIN /////////
//  OBJShader.frag
//  Cornell University Game Library (CUGL)
//
//  This is a basic MTL shader for OBJ objects. It supports illum values 0-2,
//  and also supports basic bump mapping. This shader is adapted by one written
//  by Robert Kooima for the Wavefront OBJ OpenGL renderer:
//
//      https://github.com/rlk/obj
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
//  Version: 6/24/24
#ifdef CUGLES
// This one line is all the difference
precision mediump float;
#endif

// The output color
out vec4 frag_color;

// Interpolated normal from vertex shader
in vec3 outNormal;

// Texture result from vertex shader
in vec2 outTexCoord;

// Interpolated view from vertex shader
in vec3 outView;

// Interpolated light vector
in vec3 outLight;

// The draw settings
uniform int uIllum;
uniform int uHasKa;
uniform int uHasKd;
uniform int uHasKs;
uniform int uHasKn;

// The colors
uniform vec4 uKa;
uniform vec4 uKd;
uniform vec4 uKs;
uniform float uNs;

// The textures
uniform sampler2D uMapKd;
uniform sampler2D uMapKa;
uniform sampler2D uMapKs;
uniform sampler2D uMapKn;

/**
 * Performs the main fragment shading.
 */
void main(void) {
    // Get the normal (using bump map if available)
    vec3 N = outNormal;
    if (uHasKn > 0) {
        vec4 tN = texture(uMapKn, outTexCoord);
        N = normalize(2.0 * tN.xyz - 1.0);
    }
    
    // Now do some actual lighting
    if (uIllum == 0) {
        frag_color = uKd;
        if (uHasKd > 0) {
            frag_color *= texture(uMapKd, outTexCoord);
        }
    } else {
        // Determine the per-fragment lighting value
        vec3 L = normalize(outLight);
        float lambertian = max(dot(L, N), 0.0);
        
        // Most solutions fall back on illum 0 if lambertian is 0
        frag_color = uKa;
        if (uHasKa > 0) {
            frag_color *= texture(uMapKa, outTexCoord);
        } else if (uHasKd > 0 && lambertian == 0.0) {
            frag_color = uKd*texture(uMapKd, outTexCoord);
        }

        // Now do some actual lighting
        if (lambertian > 0.0) {
            if (uHasKd > 0) {
                frag_color += uKd*lambertian*texture(uMapKd, outTexCoord);
            } else {
                frag_color += uKd*lambertian;
            }
            
            if (uIllum > 1) {
                vec3 R = reflect(L, N);
                vec3 V = normalize(outView);
                float angle = max(dot(V, R), 0.0);
                float specular = pow(angle, uNs);

                if (uHasKs > 0) {
                    frag_color += uKs*specular*texture(uMapKs, outTexCoord);
                } else {
                    frag_color += uKs*specular;
                }
            }
        }
    }
}

/////////// SHADER END //////////)"
