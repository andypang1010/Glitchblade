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

// Positions (which are particle vertices)
in vec3 aPosition;
// Position information need not be interpolated

// Texture coordinates
in vec2 aTexCoord;
out vec2 outTexCoord;

// The position of the particle center + the size (set in the w value)
in vec4 aCenter;
// Position information need not be interpolated

// The particle color
in vec4 aColor;
out vec4 outColor;

// The texture offset of an instance (for animation)
in vec2 aOffset;
// This value is output in outTextCoord

// The camera right vector
uniform vec3 uCameraRight;
// The camera up vector
uniform vec3 uCameraUp;

// The (combined) camera matrix
uniform mat4 uPerspective;
// The model matrix of this particle system
uniform mat4 uModelMatrix;

// Transform and pass through
void main() {
    // Extract the particle size and center
    float size  = aCenter.w;
	vec3 center = aCenter.xyz;
	
    // Compute the position of the particle vertex
	vec3 position = (center + uCameraRight * aPosition.x * size
                            + uCameraUp * aPosition.y * size);

	// Transform this with the camera
	gl_Position = uPerspective * uModelMatrix * vec4(position, 1.0f);

	// Output the interpolated values
    outTexCoord = aTexCoord+aOffset;
    outColor = aColor;
}

/////////// SHADER END //////////)"
