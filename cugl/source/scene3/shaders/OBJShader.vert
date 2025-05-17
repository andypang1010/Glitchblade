R"(////////// SHADER BEGIN /////////
//  OBJShader.vert
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

// Attributes
in vec3 aPosition;
in vec3 aTangent;
in vec3 aNormal;
in vec2 aTexCoord;

// Outputs
out vec2 outTexCoord;
out vec3 outNormal;
out vec3 outView;
out vec3 outLight;

// Matrices
uniform mat4 uPerspective;
uniform mat4 uModelMatrix;
uniform mat4 uNormalMatrix;

// Lighting (only one light supported)
uniform vec3 uLightPos;

// Transform and pass through
void main(void) {
    // Tangent space vectors give the columns of the eye-to-tangent transform.
    vec3 N = vec3(uNormalMatrix * vec4(aNormal,1.0));
    vec3 T = vec3(uNormalMatrix * vec4(aTangent,1.0));
    mat3 M = transpose(mat3(T, cross(N, T), N));

	vec4 vertPos = uModelMatrix * vec4(aPosition,1.0);
    outNormal =  N;
    outView   =  M * vec3(vertPos);
    outLight  =  M * vec3(uModelMatrix * vec4(uLightPos,1.0));
	outTexCoord  = aTexCoord;
    gl_Position  = uPerspective*vertPos;
}

/////////// SHADER END //////////)"
