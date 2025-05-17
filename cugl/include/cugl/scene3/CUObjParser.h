//
//  OBJParser.h
//  Cornell University Game Library (CUGL)
//
//  This module provides support for parsing Wavefront .obj files (and their
//  associated .mtl files). It does not actually construct meshes from these
//  files. Instead, it produces an abstract syntax tree that can be used to
//  create meshes. We abstract out this partion of .obj parsing when it became
//  clear that the files had a lot of back-and-forth in them that make inline
//  parsing not so straight forward.
//
//  Most users will never use these classes directly. Instead they are used
//  internally by other classes in the cugl::obj package.
//
//  These classes use our standard shared-pointer architecture.
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
#ifndef __OBJ_PARSER_H__
#define __OBJ_PARSER_H__
#include <memory>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <cugl/core/math/cu_math.h>
#include <cugl/core/util/CUHashtools.h>
#include <cugl/graphics/CUGraphicsBase.h>
#include <cugl/scene3/CUMaterial.h>
#include <cugl/scene3/CUObjModel.h>


namespace cugl {

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

/**
 * This class generates an AST for an OBJ file (and its associated MTL files).
 *
 * This parse only provides limited support for OBJ and MTL files. We only
 * support polygonal objects, with no freeform drawing. We also only support
 * basic illumination (illum values 0-2) with bump mapping.
 *
 * This class does not actually produce render data, as that is potentially
 * time consuming. That step should be left up to an asset loader. Instead,
 * this parser simply collates the information about the OBJ model into a
 * single AST, as its data can potentially be spread over multiple files
 * (including MTL and texture files).
 *
 * Because OBJ data is spread over mutliple files, this parser is stateful.
 * That means it can expand the current {@link ModelInfo} data by reading
 * other files.
 */
class ObjParser {
public:
    /** Whether to emit debugging information */
    bool debug;
    /** The information for the referenced textures */
    std::unordered_map<std::string,std::shared_ptr<TextureInfo>> textures;
    /** The information for previously parsed MTL files */
    std::unordered_map<std::string,std::shared_ptr<MaterialLib>> materials;
    /** The information for previously parsed OBJ files */
    std::unordered_map<std::string,std::shared_ptr<ModelInfo>> models;
    
    /**
     * Creates a new OBJ parser.
     *
     * This is a fairly lightweight object. Therefore it is safe to use this
     * constructor with new (though std::make_shared is prefered).
     */
    ObjParser() : debug(false) {}
    
    /**
     * Deletes this OBJ parser, disposing all resource.
     */
    ~ObjParser() { clear(); }
    
    /**
     * Releases the data from all previously parsed files.
     *
     * This method has the same affect as {@link #clear}.
     */
    void dispose() { clear(); }
    
    /**
     * Returns the information for the given OBJ file.
     *
     * This method blocks until the OBJ file is read. If recurse is true, it
     * will also read any imported MTL files (assuming that they are in the
     * same directory as the OBJ files). If recurse ifs false, it will create
     * entries in the {@link ModelInfo} object for the imported libraries with
     * no values. The user can add the information for these libraries later.
     *
     * This is a stateful parser. Once an OBJ file has been parsed, its data
     * can be retreived at any time with {@link #getObj}. This method uses the
     * path to the OBJ file as the key.
     *
     * @param source    The path to the OBJ file
     * @param recurse   Whether to load any files referenced by source
     *
     * @return the information for the given OBJ file.
     */
    std::shared_ptr<ModelInfo> parseObj(const std::string source, bool recurse) {
        return parseObj(source,source,recurse);
    }
    
    /**
     * Returns the information for the given OBJ model.
     *
     * This method allows you to specify the location of the OBJ file, its MTL
     * file, and any associated textures as a single JSON entry. An OBJ Json
     * entry has the following values
     *
     *      "file":         The path to the OBJ file
     *      "mtls":         An object of key:value pairs defining MTL libraries
     *
     * The "mtls" entry is optional. For each MTL library, the key should match
     * the name of the MTL file referenced in the obj file. If there are any
     * missing MTL libraries (or the "mtls" entry is missing entirely), then
     * the loader will attempt to use the same directory as the OBJ file.
     *
     * An MTL entry is either a string (which is a reference to the path to the
     * MTL file) or a JSON object. Such a JSON object would have the following
     * values:
     *
     *      "file":         The path to the MTL file
     *      "textures":     An object of key:value pairs defining textures
     *
     * The "textures" entry is optional. For each texture, the key should match
     * the name of the texture in the MTL file. Any missing textures will
     * attempt to be loaded if the parsing depth is correct.
     *
     * The values for the texture entries should be strings or JSONs. If they
     * are string, they should be either be a key referencing a previously
     * loaded texture, or a path to the texture file (the loader interprets it
     * as a path only if there is no key with that name). If it is a JSON, then
     * the JSON should follow the same rules as {@link graphics::Texture}.
     *
     * @param json      The JSON value specifying the OBJ model
     *
     * @return the information for the given OBJ model.
     */
    std::shared_ptr<ModelInfo> parseJson(const std::shared_ptr<JsonValue>& json);
    
    /**
     * Returns the information for the given OBJ file.
     *
     * This method blocks until the OBJ file is read. If recurse is true, it
     * will also read any imported MTL files (assuming that they are in the
     * same directory as the OBJ files). If recurse ifs false, it will create
     * entries in the {@link ModelInfo} object for the imported libraries with
     * no values. The user can add the information for these libraries later.
     *
     * This is a stateful parser. Once an OBJ file has been parsed, its data
     * can be retreived at any time with {@link #getObj}. This method uses the
     * specified key for retrieval.
     *
     * @param key       The retrieval key
     * @param source    The path to the OBJ file
     * @param recurse   Whether to load any files referenced by source
     *
     * @return the information for the given OBJ file.
     */
    std::shared_ptr<ModelInfo> parseObj(const std::string key, const std::string source,
                                        bool recurse);
    
    /**
     * Returns the information for a previously parsed OBJ file.
     *
     * This method returns nullptr if the OBJ file has not been yet parsed.
     * For models that have been parsed, the retrieval key was specified at the
     * time of parsing.
     *
     * @param key       The retrieval key
     *
     * @return the information for a previously parsed OBJ file.
     */
    std::shared_ptr<ModelInfo> getObj(const std::string key) const;
    
    /**
     * Returns the information for the given MTL file.
     *
     * This method blocks until the MTL file is read. However, it does not
     * read any imported files (like the Texture files). Instead, it assigns
     * the {@link TextureInfo} attributes according the settings in the MTL
     * file.
     *
     * This method does not link a {@link MaterialInfo} to a {@link ModelInfo}
     * object. That is the responsibility of the user.
     *
     * This is a stateful parser. Once a MTL file has been parsed, its data
     * can be retreived at any time with {@link #getMtl}. This method uses the
     * path to the MTL file as the key.
     *
     * @param source    The path to the MTL file
     *
     * @return the information for the given MTL file.
     */
    std::shared_ptr<MaterialLib> parseMtl(const std::string source) {
        return parseMtl(source,source);
    }
    
    /**
     * Returns the information for the given MTL file.
     *
     * This method blocks until the MTL file is read. However, it does not
     * read any imported files (like the Texture files). Instead, it assigns
     * the {@link TextureInfo} attributes according the settings in the MTL
     * file.
     *
     * This method does not link a {@link MaterialInfo} to a {@link ModelInfo}
     * object. That is the responsibility of the user.
     *
     * This is a stateful parser. Once a MTL file has been parsed, its data
     * can be retreived at any time with {@link #getMtl}. This method uses the
     * specified key for retrieval.
     *
     * @param key       The retrieval key
     * @param source    The path to the MTL file
     *
     * @return the information for the given MTL file.
     */
    std::shared_ptr<MaterialLib> parseMtl(const std::string key, const std::string source);
    
    /**
     * Returns the information for a previously parsed MTL file.
     *
     * This method returns nullptr if the MTL file has not been yet parsed.
     * For models that have been parsed, the retrieval key was specified at the
     * time of parsing.
     *
     * @param key       The retrieval key
     *
     * @return the information for a previously parsed MTL file.
     */
    std::shared_ptr<MaterialLib> getMtl(const std::string key) const;
    
    /**
     * Clears all internal caches.
     *
     * This is a stateful parser. Once an OBJ or MTL file has been parsed, its
     * data can be retrieved at any time with {@link #getObj} or
     * {@link #getMtl}, respectively. This method clears all such state so that
     * those methods return nullptr until a new file is parsed.
     */
    void clear();
    
private:
    /**
     * Processes a line representing an "o" command in an OBJ file.
     *
     * @param begin The start of the line
     * @param end   The end of the line
     * @param obj   The current ModelInfo results
     */
    void processObject(const char* begin, const char* end, const std::shared_ptr<ModelInfo>& obj);
    
    /**
     * Processes a line representing an "mtllib" command in an OBJ file.
     *
     * @param begin The start of the line
     * @param end   The end of the line
     * @param obj   The current ModelInfo results
     */
    void processImport(const char* begin, const char* end, const std::shared_ptr<ModelInfo>& obj);
    
    /**
     * Processes a line representing a "v" command in an OBJ file.
     *
     * @param begin The start of the line
     * @param end   The end of the line
     * @param obj   The current ModelInfo results
     */
    void processVertex(const char* begin, const char* end, const std::shared_ptr<ModelInfo>& obj);
    
    /**
     * Processes a line representing a "vt" command in an OBJ file.
     *
     * @param begin The start of the line
     * @param end   The end of the line
     * @param obj   The current ModelInfo results
     */
    void processTexCoord(const char* begin, const char* end, const std::shared_ptr<ModelInfo>& obj);
    
    /**
     * Processes a line representing a "vn" command in an OBJ file.
     *
     * @param begin The start of the line
     * @param end   The end of the line
     * @param obj   The current ModelInfo results
     */
    void processNormal(const char* begin, const char* end, const std::shared_ptr<ModelInfo>& obj);
    
    /**
     * Processes a line representing a "usemtl" command in an OBJ file.
     *
     * @param begin The start of the line
     * @param end   The end of the line
     * @param obj   The current ModelInfo results
     */
    void processUsage(const char* begin, const char* end, const std::shared_ptr<ModelInfo>& obj);
    
    /**
     * Processes a line representing a "g" command in an OBJ file.
     *
     * @param begin The start of the line
     * @param end   The end of the line
     * @param obj   The current ModelInfo results
     */
    void processGroup(const char* begin, const char* end, const std::shared_ptr<ModelInfo>& obj);
    
    /**
     * Processes a line representing an "s" command in an OBJ file.
     *
     * @param begin The start of the line
     * @param end   The end of the line
     * @param obj   The current ModelInfo results
     */
    void processSmooth(const char* begin, const char* end, const std::shared_ptr<ModelInfo>& obj);
    
    /**
     * Processes a line representing an "f" command in an OBJ file.
     *
     * @param begin The start of the line
     * @param end   The end of the line
     * @param obj   The current ModelInfo results
     */
    void processFace(const char* begin, const char* end, const std::shared_ptr<ModelInfo>& obj);
    
    /**
     * Processes a line representing an "l" command in an OBJ file.
     *
     * @param begin The start of the line
     * @param end   The end of the line
     * @param obj   The current ModelInfo results
     */
    void processLine(const char* begin, const char* end, const std::shared_ptr<ModelInfo>& obj);
    
    /**
     * Processes a line representing a "p" command in an OBJ file.
     *
     * @param begin The start of the line
     * @param end   The end of the line
     * @param obj   The current ModelInfo results
     */
    void processPoints(const char* begin, const char* end, const std::shared_ptr<ModelInfo>& obj);
    
    /**
     * Processes a line representing a "material" command in a MTL file.
     *
     * @param begin The start of the line
     * @param end   The end of the line
     * @param obj   The current ModelInfo results
     */
    void processMaterial(const char* begin, const char* end, const std::shared_ptr<MaterialLib>& mtl);
    
    /**
     * Processes a line representing an "illum" command in a MTL file.
     *
     * @param begin The start of the line
     * @param end   The end of the line
     * @param obj   The current ModelInfo results
     */
    void processIllum(const char* begin, const char* end, const std::shared_ptr<MaterialLib>& mtl);
    
    /**
     * Processes a line representing an "Ns" command in a MTL file.
     *
     * @param begin The start of the line
     * @param end   The end of the line
     * @param obj   The current ModelInfo results
     */
    void processShininess(const char* begin, const char* end, const std::shared_ptr<MaterialLib>& mtl);
    
    /**
     * Processes a line representing a "K*" command in a MTL file.
     *
     * @param begin The start of the line
     * @param end   The end of the line
     * @param obj   The current ModelInfo results
     */
    void processColor(const char* begin, const char* end, const std::shared_ptr<MaterialLib>& mtl);
    
    /**
     * Processes a line representing a "map_*" command in a MTL file.
     *
     * @param begin The start of the line
     * @param end   The end of the line
     * @param obj   The current ModelInfo results
     */
    std::shared_ptr<TextureInfo> processTexture(const char* begin, const char* end, const std::shared_ptr<MaterialLib>& mtl);
    
    /**
     * Parses a VertexInfo from a string.
     *
     * A VertexInfo is specified by 1-3 integers separated by /, per the OBJ
     * specification. This function will scan the string range for the first
     * available instance of this information and store it in this object.
     * When done, it will return the start of the unparsed portion of the string
     * (whihch may be end, if the string was fully parsed).
     *
     * @param begin The start of the string fragment to parse
     * @param end   The end of the string fragment to parse
     * @param info  The vertex to modify
     *
     * @return the start of the unparsed portion of the string
     */
    const char*  parseVertex(const char* begin, const char* end, VertexInfo& info);
};

    }
}

#endif /* __OBJ_PARSER_H__ */
