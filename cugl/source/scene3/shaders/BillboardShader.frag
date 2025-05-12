R"(////////// SHADER BEGIN /////////
//  BillboardShader.frag
//  Cornell University Game Library (CUGL)
//
//  This is a shader designed for rendering 2d billboards in a 3d space. It is
//  based off the billboard demo found at opengl-tutorial.org
//
//     http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/billboards/
//
//  This shader is a stripped down version of the SpriteBatch shader. It does
//  not allow scissoring or blurring. That is because it is designed to draw
//  a single sprite with no compositing. Minimal animation is supported via
//  a texture offset uniform. Adjusting that uniform will adjust the texture
//  coordinate of a mesh to move about a sprite sheet.
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
precision highp float;
#endif

// The output color
out vec4 frag_color;

// Color result from vertex shader
in vec4 outColor;

// Texture result from vertex shader
in vec2 outTexCoord;

// Gradient result from vertex shader
in vec2 outGradCoord;

// GRADIENT UNIFORM
// The gradient matrix
uniform mat3 uGradientMatrix;
// The gradient inner color
uniform vec4 uGradientInner;
// The gradient outer color
uniform vec4 uGradientOuter;
// The gradient extent
uniform vec2 uGradientExtent;
// The gradient radius
uniform float uGradientRadius;
// The gradient feather
uniform float uGradientFeathr;

// What to output
uniform int uType;

// Texture map
uniform sampler2D uTexture;


/**
 * Returns an interpolation value for a box gradient
 *
 * The value returned is the mix parameter for the inner and
 * outer color.
 *
 * Adapted from nanovg by Mikko Mononen (memon@inside.org)
 *
 * pt:      The (transformed) point to test
 * ext:     The gradient extent
 * radius:  The gradient radius
 * feather: The gradient feather amount
 */
float boxgradient(vec2 pt, vec2 ext, float radius, float feather) {
    vec2 ext2 = ext - vec2(radius,radius);
    vec2 dst = abs(pt) - ext2;
    float m = min(max(dst.x,dst.y),0.0) + length(max(dst,0.0)) - radius;
    return clamp((m + feather*0.5) / feather, 0.0, 1.0);
}

/**
 * Performs the main fragment shading.
 */
void main(void) {
    vec4 result;
    
    float fType = float(uType);

    if (mod(fType, 4.0) >= 2.0) {
        // Apply a gradient color
        mat3  cmatrix = uGradientMatrix;
        vec2  cextent = uGradientExtent;
        float cfeathr = uGradientFeathr;
        vec2 pt = (cmatrix * vec3(outGradCoord,1.0)).xy;
        float d = boxgradient(pt,cextent,uGradientRadius,cfeathr);
        result = mix(uGradientInner,uGradientOuter,d)*outColor;
    } else {
        // Use a solid color
        result = outColor;
    }
    
    if (mod(fType, 2.0) == 1.0) {
        // Include texture (tinted by color and/or gradient)
        result *= texture(uTexture, outTexCoord);
    }
    
    frag_color = result;
}
/////////// SHADER END //////////)"
