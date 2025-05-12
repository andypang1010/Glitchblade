R"(////////// SHADER BEGIN /////////
//  ParticleShader.frag
//  Cornell University Game Library (CUGL)
//
//  This is a shader designed for rendering particle systems in OpenGL ES. It
//  is compatible with both 2d and 3d rendering. It is based off of the particle
//  system tutorial found as opengl-tutorial.org
//
//     http://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/particles-instancing/
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

// Interpolated values from the vertex shaders
in vec2 outTexCoord;
in vec4 outColor;

// The output color
out vec4 frag_color;

// The particle texture
uniform sampler2D uTexture;

// Perform simple texturing and coloring
void main(){
    frag_color = texture( uTexture, outTexCoord ) * outColor;
}

/////////// SHADER END //////////)"
