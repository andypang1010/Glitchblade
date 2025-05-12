//
//  CUMaterial.h
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
#ifndef __CU_MATERIAL_H__
#define __CU_MATERIAL_H__
#include <cugl/core/math/cu_math.h>
#include <cugl/graphics/CUGraphicsBase.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace cugl {

// Forward reference
namespace graphics {
    class Texture;
}

    /**
     * The classes to construct a 3-d scene graph.
     *
     * Unlike the scene2 package, the 3-d scene graph classes are quite limited.
     * We only have support for OBJ/MTL files, as well as simple billboarding.
     * There is no support for bone animation or physically based rendering.
     *
     * The reason for this limitation is because this is a student training
     * engine, and we often like to task students at adding those features.
     * In addition, unlike our 2-d scene graph with Figma support, there are
     * a lot of third party libraries out there that handle rendering better
     * for 3-d scenes.
     */
    namespace scene3 {

// Forward reference
class ObjShader;

#pragma mark AST Classes
/**
 * This class is a reference to a (potentially loaded) texture.
 *
 * This class contains enough information to load a texture from a file (either
 * via {@link graphics::TextureLoader} or directly via the {@link graphics::Texture}
 * interface). It is used to support textures referenced in an MTL file that
 * may or may not have been loaded previously by an {@link AssetManager}.
 */
class TextureInfo {
public:
    /** The key for this TextureInfo in an {@link AssetManager} */
    std::string name;
    /** The path to the texture file */
    std::string path;
    /** The minimization algorithm */
    GLuint minflt;
    /** The maximization algorithm */
    GLuint magflt;
    /** The wrap-style for the horizontal texture coordinate */
    GLuint wrapS;
    /** The wrap-style for the vertical texture coordinate */
    GLuint wrapT;
    /** Whether or not the texture has mip maps */
    bool mipmaps;
    
    /**
     * Creates a default TextureInfo.
     *
     * By default, material textures are repeated and use linear scaling.
     * Mipmaps are not supported by default.
     */
    TextureInfo() :
    minflt(GL_LINEAR),
    magflt(GL_LINEAR),
    wrapS(GL_REPEAT),
    wrapT(GL_REPEAT),
    mipmaps(false) {
        // Keys and paths are undefined
        name = "";
        path = "";
    }
    
};

/**
 * This class represents the attributes of a MTL material.
 *
 * Our Material objects currently include support for illums 0-2 (using
 * standard phong shading). We allow texture maps for ambient, diffuse, and
 * specular color. However, the shininess value is only a scalar; we do not
 * support texture maps for the specular exponent.
 *
 * We do not support reflection, transparency, or emission. We also do not
 * support dissolve effects or decals. If you need more complicated MTL support,
 * you should use a third party rendering library.
 *
 * With that said, this class does provide support for simple bump mapping. The
 * bump texture should be an RGB image encoding the normals in tangent space.
 */
class MaterialInfo {
public:
    /** The material name */
    std::string name;
    /** The illumination setting */
    GLuint illum;
    /** The specular exponent (shininess) */
    float Ns;
    /** The color of the ambient reflectivity */
    Color4f Ka;
    /** The color of the diffuse reflectivity */
    Color4f Kd;
    /** The color of the specular reflectivity */
    Color4f Ks;
    /** The texture map for the ambient colors */
    std::shared_ptr<TextureInfo> map_Ka;
    /** The texture map for the diffuse colors */
    std::shared_ptr<TextureInfo> map_Kd;
    /** The texture map for the specular colors */
    std::shared_ptr<TextureInfo> map_Ks;
    /** The texture map for the per pixel normals */
    std::shared_ptr<TextureInfo> map_Kn;
    
    /**
     * Creates a default MaterialInfo.
     *
     * By default, materials support diffuse only shading (no ambient or
     * specular).
     */
    MaterialInfo() : illum(0), Ns(1.0f) {
        name = "";
        Kd.set(1,1,1);
    }
};

#pragma mark -
#pragma mark Material
/**
 * This class represents a WaveFront material
 *
 * The MTL file itself is read into an {@link MaterialLib} object using an
 * {@link ObjParser}. From that object, you can access the {@link MaterialInfo}
 * objects and use them to construct actual materials.
 *
 * Our Material objects currently include support for illums 0-2 (using
 * standard phong shading). We allow texture maps for ambient, diffuse, and
 * specular color. However, the shininess value is only a scalar; we do not
 * support texture maps for the specular exponent.
 *
 * Colors are determined by multiplying the color coefficient with the
 * appropriate texture. If a texture is missing, a solid color is used
 * instead. The exception to this rule is ambient color. If there is no
 * ambient texture, but there is a diffuse texture, then the ambient color
 * is the ambient coefficient times the diffuse texture.
 *
 * We do not support reflection, transparency, or emission. We also do not
 * support dissolve effects or decals. If you need more complicated MTL
 * support, you should use a third party rendering library.
 *
 * With that said, this class does provide support for simple bump mapping. The
 * bump texture should be an RGB image encoding the normals in tangent space.
 */
class Material {
private:
    /** The name of this material */
    std::string _name;
    /** The illumination setting */
    GLuint _illum;
    /** The specular exponent (shininess) */
    float _shininess;
    /** The color of the ambient reflectivity */
    cugl::Color4f _ambient;
    /** The color of the diffuse reflectivity */
    cugl::Color4f _diffuse;
    /** The color of the specular reflectivity */
    cugl::Color4f _specular;
    /** Texture representing the ambient light */
    std::shared_ptr<graphics::Texture> _ambientMap;
    /** Texture representing the diffuse surface color */
    std::shared_ptr<graphics::Texture> _diffuseMap;
    /** Texture representing the specular surface color */
    std::shared_ptr<graphics::Texture> _specularMap;
    /** Texture representing the fragment normals */
    std::shared_ptr<graphics::Texture> _normalMap;
    
public:
    /**
     * Creates a degenerate material
     *
     * Binding this material is guaranteed to create a white, featureless
     * surface. For a more interesting material, you must initialize it using
     * a {@link MaterialInfo} object.
     */
    Material() : _illum(0), _shininess(1.0) {
        _name = "";
        _ambient.set(1,1,1,1); // White by default
    }
    
    /**
     * Deletes this material, disposing all resources
     */
    ~Material() { dispose(); }
    
    /**
     * Disposes all of the resources used by this material.
     *
     * A disposed Material can be safely reinitialized. Any textures owned by
     * this material will be released. They will be deleted if no other object
     * owns them.
     */
    void dispose();
    
    /**
     * Initializes a simple material.
     *
     * Binding this material is guaranteed to create a white, featureless
     * surface. This initializer does not do much more than the constructor.
     *
     * @return true if initialization was successful.
     */
    bool init() { _diffuse.set(1,1,1,1); return true; }
    
    /**
     * Initializes a material with the given AST information
     *
     * This method will fill in the attributes using the information provided.
     * If load is true, it will also load and allocate any of the specified
     * textures. This value is false by default, as it is assumed the textures
     * will be loaded separately and manually attached to the material. Textures
     * must be in the same direct as the MTL file to be loaded automatically.
     *
     * @param info      The material information
     * @param texture   Whether to load and allocate the textures
     *
     * @return true if initialization was successful.
     */
    bool initWithInfo(const std::shared_ptr<MaterialInfo>& info, bool texture=false);

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
    bool initWithTexture(const std::shared_ptr<graphics::Texture>& texture);

    /**
     * Returns a newly allocated simple material.
     *
     * Binding this material is guaranteed to create a white, featureless
     * surface. This initializer does not do much more than the constructor.
     *
     * @return a newly allocated simple material.
     */
    static std::shared_ptr<Material> alloc() {
        std::shared_ptr<Material> result = std::make_shared<Material>();
        return (result->init() ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated material with the given AST information
     *
     * This method will fill in the attributes using the information provided.
     * If load is true, it will also load and allocate any of the specified
     * textures. This value is false by default, as it is assumed the textures
     * will be loaded separately and manually attached to the material. Textures
     * must be in the same direct as the MTL file to be loaded automatically.
     *
     * @param info      The material information
     * @param texture   Whether to load and allocate the textures
     *
     * @return a newly allocated material with the given AST information
     */
    static std::shared_ptr<Material> allocWithInfo(const std::shared_ptr<MaterialInfo>& info,
                                                   bool texture=false) {
        std::shared_ptr<Material> result = std::make_shared<Material>();
        return (result->initWithInfo(info,texture) ? result : nullptr);
    }
    
    /**
     * Returns a newly allocated material with a single texture.
     *
     * This creates a trivial material that uses the given texture as the
     * diffuse map (with a white light color). There will be no textures for
     * the other maps, and all other lights will be clear.
     *
     * @param texture   The diffuse texture for this material
     *
     * @return a newly allocated material with a single texture.
     */
    static std::shared_ptr<Material> allocWithTexture(const std::shared_ptr<graphics::Texture>& texture) {
        std::shared_ptr<Material> result = std::make_shared<Material>();
        return (result->initWithTexture(texture) ? result : nullptr);
    }

#pragma mark Attributes
    /**
     * Returns the name of this material.
     *
     * @return the name of this material.
     */
    std::string getName() const { return _name; }

    /**
     * Sets the name of this material.
     *
     * @param name	the name of this material.
     */
    void setName(std::string name) { _name = name; }

    /**
     * Returns the illumination setting of this material.
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
     * @return the illumination setting of this material.
     */
    GLuint getIllum() const { return _illum; }
    
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
    void setIllum(GLuint value);
    
    /**
     * Returns the shininess coefficient.
     *
     * This value defines the focus of the specular highlight as an exponent.
     * A high exponent results in a tight, concentrated highlight. These values
     * normally range from 0 to 1000.
     *
     * @return the shininess coefficient.
     */
    float getShininess() const { return _shininess; }
    
    /**
     * Sets the shininess coefficient.
     *
     * This value defines the focus of the specular highlight as an exponent.
     * A high exponent results in a tight, concentrated highlight. These values
     * normally range from 0 to 1000.
     *
     * @param value The shininess coefficient.
     */
    void setShininess(float value);
    
    /**
     * Returns the color coefficient of the ambient light.
     *
     * The ambient light is determined by multiplying this color times the
     * ambient texture (if it exists). No ambient texture produces a solid
     * color. However, if the diffuse texture exists while the ambient texture
     * does not, the ambient light will multiply this coefficient times that
     * texture instead.
     *
     * @return the color coefficient of the ambient light.
     */
    cugl::Color4f getAmbientTint() const { return _ambient; }
    
    /**
     * Sets the color coefficient of the ambient light.
     *
     * The ambient light is determined by multiplying this color times the
     * ambient texture (if it exists). No ambient texture produces a solid
     * color. However, if the diffuse texture exists while the ambient texture
     * does not, the ambient light will multiply this coefficient times that
     * texture instead.
     *
     * @param value The color coefficient of the ambient light.
     */
    void setAmbientTint(const cugl::Color4f value) { _ambient = value; }
    
    /**
     * Returns the color coefficient of the diffuse light.
     *
     * The diffuse light is determined by multiplying this color times the
     * diffuse texture (if it exists). No diffuse texture produces a solid
     * color. The diffuse light is also affected by the direction of the light
     * source.
     *
     * @return the color coefficient of the diffuse light.
     */
    cugl::Color4f getDiffuseTint() const { return _diffuse; }
    
    /**
     * Sets the color coefficient of the diffuse light.
     *
     * The diffuse light is determined by multiplying this color times the
     * diffuse texture (if it exists). No diffuse texture produces a solid
     * color. The diffuse light is also affected by the direction of the light
     * source.
     *
     * @param value The color coefficient of the diffuse light.
     */
    void setDiffuseTint(const cugl::Color4f value) { _diffuse = value; }
    
    /**
     * Returns the color coefficient of the specular light.
     *
     * The specular light is determined by multiplying this color times the
     * specular texture (if it exists). No specular texture produces a solid
     * color. The specular light is also affected by the direction of the
     * light source, as well as the shininess exponent.
     *
     * @return the color coefficient of the specular light.
     */
    cugl::Color4f getSpecularTint() const { return _specular; }
    
    /**
     * Sets the color coefficient of the specular light.
     *
     * The specular light is determined by multiplying this color times the
     * specular texture (if it exists). No specular texture produces a solid
     * color. The specular light is also affected by the direction of the
     * light source, as well as the shininess exponent.
     *
     * @param value The color coefficient of the specular light.
     */
    void setSpecularTint(const cugl::Color4f value) { _specular = value; }
    
    /**
     * Returns the texture map of the ambient light.
     *
     * The ambient light is determined by multiplying this texture (if it
     * exists) times the ambient color. No ambient texture produces a solid
     * color. However, if the diffuse texture exists while the ambient texture
     * does not, the ambient light will multiply this coefficient times that
     * texture instead.
     *
     * @return the texture map of the ambient light.
     */
    std::shared_ptr<graphics::Texture> getAmbientMap() const { return _ambientMap; }
    
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
    void setAmbientMap(const std::shared_ptr<graphics::Texture>& value);
    
    /**
     * Returns the texture map of the diffuse light.
     *
     * The diffuse light is determined by multiplying this texture (if it
     * exists) times the diffuse color. No diffuse texture produces a solid
     * color. This texture will also determine the ambient light if there is
     * no separate texture for the ambient light.
     *
     * @return the texture map of the diffuse light.
     */
    std::shared_ptr<graphics::Texture> getDiffuseMap() const { return _diffuseMap; }
    
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
    void setDiffuseMap(const std::shared_ptr<graphics::Texture>& value);
    
    /**
     * Returns the texture map of the specular light.
     *
     * The specular light is determined by multiplying this texture (if it
     * exists) times the specular color. No specular texture produces a solid
     * color. The specular light is also affected by the direction of the
     * light source, as well as the shininess exponent.
     *
     * @return the texture map of the specular light.
     */
    std::shared_ptr<graphics::Texture> getSpecularMap() const { return _specularMap; }
    
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
    void setSpecularMap(const std::shared_ptr<graphics::Texture>& value);
    
    /**
     * Returns the texture map of the fragment normals.
     *
     * This texture specifies the per fragment normals for bump mapping. These
     * normals are encoded as RGB values in tangent space (meaning the images
     * typically appear blue). If this texture is not specified, the lighting
     * normal will be interpolated from the vertex normals instead.
     *
     * @return the texture map of the fragment normals.
     */
    std::shared_ptr<graphics::Texture> getBumpMap() const { return _normalMap; }
    
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
    void setBumpMap(const std::shared_ptr<graphics::Texture>& value);
    
    
#pragma mark Usage
    /**
     * Binds this material to the given shader, activating it.
     *
     * This call is reentrant. If can be safely called multiple times.
     */
    void bind(const std::shared_ptr<ObjShader>& shader);
    
    /**
     * Unbinds the material, making it inactive.
     *
     * This call is reentrant. If can be safely called multiple times.
     */
    void unbind();
    
};
    
#pragma mark -
#pragma mark Library
/**
 * This class represents an MTL file
 *
 * An MTL is a collection of one or more materials. An OBJ file can reference
 * more than one MTL file.
 *
 * As MaterialLib objects are processed using a state machine, they will have
 * an active material at all times during parsing. However, this active texture
 * will be set to nullptr once the library is fully parsed.
 */
class MaterialLib {
public:
    /** The key for this MaterialLib in an {@link AssetManager} */
    std::string name;
    /** The path to the texture file */
    std::string path;
    /** The material information for this library. */
    std::unordered_map<std::string,std::shared_ptr<MaterialInfo>> matinfos;
    /** The completed materials for this library. */
    std::unordered_map<std::string,std::shared_ptr<Material>> materials;
    /** Whether the library has completed loading */
    bool complete;
    // TODO: Put this in the parser
    /** The current material for parsing (nullptr when parsing complete) */
    std::shared_ptr<MaterialInfo> active;
    /**
     * Creates an empty MaterialLib.
     */
    MaterialLib() : complete(false) {
        name = "";
        path = "";
    }
    
    /**
     * Returns a new MaterialInfo allocated for this library and key
     *
     * This method is used during parsing to add new material data
     *
     * @param key   The material key
     *
     * @return a new MaterialInfo allocated for this library and key
     */
    std::shared_ptr<MaterialInfo> acquireMaterial(const std::string key);
    
    // TODO: Put this in the parser
    /**
     * Returns the currently active MaterialInfo object.
     *
     * This method is used during parsing to update the current material.
     *
     * @return the currently active MaterialInfo object.
     */
    std::shared_ptr<MaterialInfo> currentMaterial() const {
        return active;
    }
};

    }
}

#endif /* __CU_MATERIAL_H__ */
