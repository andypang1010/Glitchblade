R"(////////// SHADER BEGIN /////////
//  BillboardShader.vert
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

// Positions
in vec4 aPosition;
// Position data not interpolated

// Colors
in  vec4 aColor;
out vec4 outColor;

// Texture coordinates
in  vec2 aTexCoord;
out vec2 outTexCoord;

// Gradient coordinates
in  vec2 aGradCoord;
out vec2 outGradCoord;

// The camera right vector
uniform vec3 uCameraRight;
// The camera up vector
uniform vec3 uCameraUp;

// The (combined) camera matrix
uniform mat4 uPerspective;
// The model matrix of this billboard
uniform mat4 uModelMatrix;
// The offset of the tex coords (for animation)
uniform vec2 uTexOffset;

// Transform and pass through
void main() {
    // Compute the position of the billboard vertex
    vec3 position = (uCameraRight * aPosition.x + uCameraUp * aPosition.y);
    
    // Transform this with the camera
    gl_Position = uPerspective * uModelMatrix * vec4(position, 1.0f);

    // Output the interpolated values
    outColor = aColor;
    outTexCoord  = aTexCoord+uTexOffset;
    outGradCoord = aGradCoord;
}

/////////// SHADER END //////////)"
