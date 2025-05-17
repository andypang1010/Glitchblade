//
//  CUMaterial.cpp
//  Cornell University Game Library (CUGL)
//
//  This module represents a WaveFront Material as defined by a MTL file. It
//  includes classes both for the Material object itself, as well as classes
//  for the AST built during parsing.
//
//  Our Material objects currently include support for illums 0-2 (using
//  standard phong shading). We allow texture maps for ambient, diffuse, and
//  specular color. However, the shininess value is only a scalar; we do not
//  support texture maps for the specular exponent. We also support simple
//  bump mapping.
//
//  Because the AST classes are essentially structs with no strong invariants,
//  all attributes are public and we do not provide any initialization or
//  allocation methods for them. However, the Material class itself has proper
//  encapsulation combined with our standard shared-pointer architecture.
//
//  1. The constructor does not perform any initialization; it just sets all
//     attributes to their defaults.
//
//  2. All initialization takes place via init methods, which can fail if an
//     object is initialized more than once.
//
//  3. All allocation takes place via static constructors which return a shared
//     pointer.
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
#include <cugl/graphics/CUTexture.h>
#include <cugl/scene3/CUMaterial.h>
#include <cugl/scene3/CUObjShader.h>

using namespace cugl;
using namespace cugl::graphics;
using namespace cugl::scene3;
using namespace std;

#pragma mark MaterialLib

#define DIFFUSE_BIND  0
#define AMBIENT_BIND  1
#define SPECULAR_BIND 2
#define NORMAL_BIND   3
/**
 * Returns a new MaterialInfo allocated for this library and key
 *
 * This method is used during parsing to add new material data
 *
 * @param key   The material key
 *
 * @return a new MaterialInfo allocated for this library and key
 */
std::shared_ptr<MaterialInfo> MaterialLib::acquireMaterial(const std::string key) {
    auto it = matinfos.find(key);
    if (it != matinfos.end()) {
        return it->second;
    }
    
    auto result = std::make_shared<MaterialInfo>();
    matinfos[key] = result;
    active = result;
    return result;
}

#pragma mark -
#pragma mark Material

/**
 * Disposes all of the resources used by this material.
 *
 * A disposed Material can be safely reinitialized. Any textures owned by
 * this material will be released. They will be deleted if no other object
 * owns them.
 */
void Material::dispose() {
    _normalMap = nullptr;
    _ambientMap = nullptr;
    _diffuseMap = nullptr;
    _specularMap = nullptr;
    _ambient.set(0,0,0,0);
    _diffuse.set(0,0,0,0);
    _specular.set(0,0,0,0);
    _shininess = 1.0f;
    _illum = 0;
    _name = "";
}

/**
 * Initializes a material with the given AST information
 *
 * This method will fill in the attributes using the information provided.
 * If load is true, it will also load and allocate any of the specified
 * textures. This value is false by default, as it is assumed the textures
 * will be loaded separately and manually attached to the material. Textures
 * must be in the same direct as the MTL file to be loaded automatically.
 *
 * @param info  The material information
 * @param load  Whether to load and allocate the textures
 *
 * @return true if initialization was successful.
 */
bool Material::initWithInfo(const std::shared_ptr<MaterialInfo>& info, bool texture) {
    if (info == nullptr) {
        return false;
    }
    
    _name = info->name;
    _illum = info->illum;
    _shininess = info->Ns;
    _ambient = info->Ka;
    _diffuse = info->Kd;
    _specular = info->Ks;
    
    if (texture) {
        if (info->map_Ka != nullptr) {
            _ambientMap = Texture::allocWithFile(info->map_Ka->path);
            _ambientMap->setBindPoint(AMBIENT_BIND);
            if (_ambientMap != nullptr) {
                _ambientMap->setWrapS(info->map_Ka->wrapS);
                _ambientMap->setWrapT(info->map_Ka->wrapT);
            }
        }
        if (info->map_Kd != nullptr) {
            _diffuseMap = Texture::allocWithFile(info->map_Kd->path);
            _diffuseMap->setBindPoint(DIFFUSE_BIND);
            if (_diffuseMap != nullptr) {
                _diffuseMap->setWrapS(info->map_Kd->wrapS);
                _diffuseMap->setWrapT(info->map_Kd->wrapT);
            }
        }
        if (info->map_Ks != nullptr) {
            _specularMap = Texture::allocWithFile(info->map_Ks->path);
            _specularMap->setBindPoint(SPECULAR_BIND);
            if (_specularMap != nullptr) {
                _specularMap->setWrapS(info->map_Ks->wrapS);
                _specularMap->setWrapT(info->map_Ks->wrapT);
            }
        }
        if (info->map_Kn != nullptr) {
            _normalMap = Texture::allocWithFile(info->map_Kn->path);
            _normalMap->setBindPoint(NORMAL_BIND);
            if (_normalMap != nullptr) {
                _normalMap->setWrapS(info->map_Kn->wrapS);
                _normalMap->setWrapT(info->map_Kn->wrapT);
            }
        }
    }
    return true;
}

/**
 * Initializes a material with a single texture.
 *
 * This creates a trivial material that uses the given texture as the
 * diffuse map (with a white light color). There will be no textures for
 * the other maps, and all other lights will be clear.
 *
 * @param texture   The diffuse texture for this material
 *
 * @return true if initialization was successful.
 */
bool Material::initWithTexture(const std::shared_ptr<graphics::Texture>& texture) {
    if (texture == nullptr) {
        return false;
    }
    
    _name = texture->getName();
    _diffuseMap = texture;
    _diffuse.set(1,1,1,1);
    return true;    
}
    
#pragma mark Attributes
/**
 * Sets the illumination setting of this material.
 *
 * We support illum values 0-2. In the MTL specification, these are defined
 * as follows:
 *
 *     0: Color with no reflection (ambient only)
 *     1: Diffuse reflection
 *     2: Specular reflection
 *
 * Colors are determined by multiplying the color coefficient with the
 * appropriate texture. If a texture is missing, a solid color is used
 * instead. The exception to this rule is ambient color. If there is no
 * ambient texture, but there is a diffuse texture, then the ambient color
 * is the ambient coefficient times the diffuse texture.
 *
 * @param value The illumination setting of this material
 */
void Material::setIllum(GLuint value) {
    CUAssertLog(value < 3, "Unsupported illum value: %d",value);
    _illum = value;
}
    
/**
 * Sets the shininess coefficient.
 *
 * This value defines the focus of the specular highlight as an exponent.
 * A high exponent results in a tight, concentrated highlight. These values
 * normally range from 0 to 1000.
 *
 * @param value The shininess coefficient.
 */
void Material::setShininess(float value) {
    CUAssertLog(value >= 0, "Unsupported shininess value: %f",value);
    _shininess = value;
}
    
/**
 * Sets the texture map of the ambient light.
 *
 * The ambient light is determined by multiplying this texture (if it
 * exists) times the ambient color. No ambient texture produces a solid
 * color. However, if the diffuse texture exists while the ambient texture
 * does not, the ambient light will multiply this coefficient times that
 * texture instead.
 *
 * @param value The texture map of the ambient light.
 */
void Material::setAmbientMap(const std::shared_ptr<graphics::Texture>& value) {
    _ambientMap = value;
    if (_ambientMap != nullptr) {
        _ambientMap->setBindPoint(AMBIENT_BIND);
    }
}

/**
 * Sets the texture map of the diffuse light.
 *
 * The diffuse light is determined by multiplying this texture (if it
 * exists) times the diffuse color. No diffuse texture produces a solid
 * color. This texture will also determine the ambient light if there is
 * no separate texture for the ambient light.
 *
 * @param value The texture map of the diffuse light.
 */
void Material::setDiffuseMap(const std::shared_ptr<graphics::Texture>& value) {
    _diffuseMap = value;
    if (_diffuseMap != nullptr) {
        _diffuseMap->setBindPoint(DIFFUSE_BIND);
    }
}

/**
 * Sets the texture map of the specular light.
 *
 * The specular light is determined by multiplying this texture (if it
 * exists) times the specular color. No specular texture produces a solid
 * color. The specular light is also affected by the direction of the
 * light source, as well as the shininess exponent.
 *
 * @param value The texture map of the specular light.
 */
void Material::setSpecularMap(const std::shared_ptr<graphics::Texture>& value) {
    _specularMap = value;
    if (_specularMap != nullptr) {
        _specularMap->setBindPoint(SPECULAR_BIND);
    }
}
 
/**
 * Sets the texture map of the fragment normals.
 *
 * This texture specifies the per fragment normals for bump mapping. These
 * normals are encoded as RGB values in tangent space (meaning the images
 * typically appear blue). If this texture is not specified, the lighting
 * normal will be interpolated from the vertex normals instead.
 *
 * @param value The texture map of the fragment normals.
 */
void Material::setBumpMap(const std::shared_ptr<graphics::Texture>& value) {
    _normalMap = value;
    if (_normalMap != nullptr) {
        _normalMap->setBindPoint(NORMAL_BIND);
    }
}


#pragma mark Usage
/**
 * Binds this material to the given shader, activating it.
 *
 * This call is reentrant. If can be safely called multiple times.
 */
void Material::bind(const std::shared_ptr<ObjShader>& shader) {
    CUAssertLog(shader != nullptr,"Undefined shader");
    CUAssertLog(shader->isBound(),"Shader is not bound for drawing");

    shader->setIllum(_illum);
    shader->setSpecularExponent(_shininess);
    shader->setAmbientColor(_ambient);
    shader->setDiffuseColor(_diffuse);
    shader->setSpecularColor(_specular);
    
    // For missing textures
    Texture::getBlank()->bind();
    
    shader->setAmbientTexture(_ambientMap);
    if (_ambientMap) {
        _ambientMap->bind();
    }

    shader->setDiffuseTexture(_diffuseMap);
    if (_diffuseMap) {
        _diffuseMap->bind();
    }

    shader->setSpecularTexture(_specularMap);
    if (_specularMap) {
        _specularMap->bind();
    }

    shader->setNormalTexture(_normalMap);
    if (_normalMap) {
        _normalMap->bind();
    }

    glActiveTexture(GL_TEXTURE0);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        CULogError("Material: %s", gl_error_name(error).c_str());
        return;
    }
}

/**
 * Unbinds the material, making it inactive.
 *
 * This call is reentrant. If can be safely called multiple times.
 */
void Material::unbind() {
    if (_ambientMap) {
        _ambientMap->unbind();
    }
    if (_diffuseMap) {
        _diffuseMap->unbind();
    }
    if (_specularMap) {
        _specularMap->unbind();
    }
    if (_normalMap) {
        _normalMap->unbind();
    }
}
    
