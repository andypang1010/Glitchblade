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
#include <cugl/scene3/CUObjParser.h>
#include <cugl/core/io/CUTextReader.h>
#include <cugl/core/util/CUStringTools.h>
#include <cugl/core/util/CUFiletools.h>
#include <cugl/core/assets/CUJsonValue.h>

using namespace cugl;
using namespace cugl::scene3;
using namespace cugl::graphics;
using namespace std;

/**
 * Returns true if c is skippable whitespace
 *
 * While OBJ files are not really UTF8 compliant, this function is the safest
 * way to
 */
static bool isSkippable(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c <= 0;
}

#pragma mark Parsing

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
std::shared_ptr<ModelInfo> ObjParser::parseJson(const std::shared_ptr<JsonValue>& json) {
    if (json->isString()) {
        return parseObj(json->asString(),true);
    } else if (!json->has("file")) {
        return nullptr;
    }
    
    std::string key = json->key();
    std::string source = json->getString("file");
    std::string prefix = cugl::filetool::split_path(source).first;
    prefix.push_back(cugl::filetool::path_sep);



    std::shared_ptr<ModelInfo> result = parseObj(key,source,false);
    if (result == nullptr) {
        return nullptr;
    }
    
    JsonValue* mtljson = nullptr;
    if (json->has("mtls")) {
        mtljson = json->get("mtls").get();
    }
    
    // Get the materials.
    for(auto it = result->libraries.begin(); it != result->libraries.end(); ++it) {
        JsonValue* child = nullptr;
        std::string path = prefix+it->first;
        if (mtljson != nullptr && mtljson->has(it->first)) {
            child = mtljson->get(it->first).get();
            path = child->getString("file",it->first);
        }
        
        auto lib = parseMtl(it->first,path);
        it->second = lib;
        JsonValue* sub;
        
        // Now get the textures
        if (lib != nullptr && child != nullptr && child->has("textures")) {
            child = child->get("textures").get();
            for(auto jt = lib->matinfos.begin(); jt != lib->matinfos.end(); ++jt) {
                auto mtlinfo = jt->second;
                if (mtlinfo->map_Ka != nullptr && child->has(mtlinfo->map_Ka->name)) {
                    sub = child->get(mtlinfo->map_Ka->name).get();
                    mtlinfo->map_Ka->path = sub->isString() ? sub->asString() : sub->getString("file",mtlinfo->map_Ka->path);
                }
                if (mtlinfo->map_Kd != nullptr && child->has(mtlinfo->map_Kd->name)) {
                    sub = child->get(mtlinfo->map_Kd->name).get();
                    mtlinfo->map_Kd->path = sub->isString() ? sub->asString() : sub->getString("file",mtlinfo->map_Kd->path);
                }
                if (mtlinfo->map_Ks != nullptr && child->has(mtlinfo->map_Ks->name)) {
                    sub = child->get(mtlinfo->map_Ks->name).get();
                    mtlinfo->map_Ks->path = sub->isString() ? sub->asString() : sub->getString("file",mtlinfo->map_Ks->path);
                }
                if (mtlinfo->map_Kn != nullptr && child->has(mtlinfo->map_Kn->name)) {
                    sub = child->get(mtlinfo->map_Kn->name).get();
                    mtlinfo->map_Kn->path = sub->isString() ? sub->asString() : sub->getString("file",mtlinfo->map_Kn->path);
                }
            }
        }
    }
    
    return result;
}

/**
 * Returns the information for the given OBJ file.
 *
 * This method blocks until the OBJ file is read. However, it does not
 * read any imported files (like the MTL files). Instead, it creates
 * entries in the {@link ModelInfo} object for the imported libraries with
 * no values. It is the responsibility of the user to parse these libraries
 * with the {@link #parseMTL} command.
 *
 * This is a stateful parser. Once an OBJ file has been parsed, its data
 * can be retreived at any time with {@link #getOBJ}. This method uses the
 * specified key for retrieval.
 *
 * @param key       The retrieval key
 * @param source    The path to the OBJ file
 *
 * @return the information for the given OBJ file.
 */
std::shared_ptr<ModelInfo> ObjParser::parseObj(const std::string key,
                                               const std::string source,
                                               bool recurse) {
    auto reader = TextReader::alloc(source);
    if (reader == nullptr) {
        CUAssertLog(false, "Could not read file %s", source.c_str());
        return nullptr;
    }
     
    // Create a single model
    std::shared_ptr<ModelInfo> model = std::make_shared<ModelInfo>();
    model->name = key;
    model->path = source;
     
    while (reader->ready()) {
        string line = reader->readLine();
        const char* begin = line.c_str();
        const char* end = begin+line.size();
         
        while(begin != end && isSkippable(*begin)) {
            begin++;
        }
         
        if (begin != end) {
            char c = *begin;
             
            switch (c) {
            case 'o':
                processObject(begin, end, model);
                break;
            case 'm':
                processImport(begin, end, model);
                break;
            case 'g':
                processGroup(begin, end, model);
                break;
            case 's':
                processSmooth(begin, end, model);
                break;
            case 'v':
                processVertex(begin, end, model);
                break;
            case 'f':
                processFace(begin, end, model);
                break;
            case 'l':
                processLine(begin, end, model);
                break;
            case 'p':
                processPoints(begin, end, model);
                break;
            case 'u':
                processUsage(begin, end, model);
                break;
            case '#':
                // Comment.  Abort
                break;
            default:
                if (debug) {
                    CULogError("Unsupported OBJ command: %s",begin);
                }
                break;
            }
         }
     }
     
    reader = nullptr;

    if (recurse) {
        std::string root = cugl::filetool::split_path(source).first;
        for(auto it = model->libraries.begin(); it != model->libraries.end(); ++it) {
            if (it->second == nullptr) {
                std::string path = cugl::filetool::join_path({root,it->first});
                it->second = parseMtl(it->first,path);
            }
        }
    }

    return model;
}

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
std::shared_ptr<ModelInfo> ObjParser::getObj(const std::string key) const {
    auto it = models.find(key);
    if (it != models.end()) {
        return it->second;
    }
    return nullptr;
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
 * can be retreived at any time with {@link #getMTL}. This method uses the
 * specified key for retrieval.
 *
 * @param key       The retrieval key
 * @param source    The path to the MTL file
 *
 * @return the information for the given MTL file.
 */
std::shared_ptr<MaterialLib> ObjParser::parseMtl(const std::string key, const std::string source) {
    auto reader = TextReader::alloc(source);
    if (reader == nullptr) {
        CUAssertLog(false, "Could not read file %s", source.c_str());
        return nullptr;
    }
     
    // Create a single model
    std::shared_ptr<MaterialLib> lib = std::make_shared<MaterialLib>();
    lib->name = key;
    lib->path = source;
    
    std::string root = cugl::filetool::split_path(source).first;
     
    while (reader->ready()) {
        string line = reader->readLine();
        const char* begin = line.c_str();
        const char* end = begin+line.size();
         
        while(begin != end && isSkippable(*begin)) {
            begin++;
        }
         
        if (begin != end) {
            char c = *begin;
             
            switch (c) {
            case 'n':
                processMaterial(begin, end, lib);
                break;
            case 'i':
                processIllum(begin, end, lib);
                break;
            case 'N':
                processShininess(begin, end, lib);
                break;
            case 'K':
                processColor(begin, end, lib);
                break;
            case 'm':
            case 'b':
                {
                    auto texture = processTexture(begin, end, lib);
                    if (texture != nullptr) {
                        texture->path = cugl::filetool::join_path({root,texture->name});
                    }
                }
                break;
            case '#':
                // Comment.  Abort
                break;
            default:
                if (debug) {
                    CULogError("Unsupported MTL command: %s",begin);
                }
                break;
            }
         }
     }
     
    reader = nullptr;
    return lib;
}

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
std::shared_ptr<MaterialLib> ObjParser::getMtl(const std::string key) const {
    auto it = materials.find(key);
    if (it != materials.end()) {
        return it->second;
    }
    return nullptr;

}

/**
 * Clears all internal caches.
 *
 * This is a stateful parser. Once an OBJ or MTL file has been parsed, its
 * data can be retrieved at any time with {@link #getOBJ} or
 * {@link #getMTL}, respectively. This method clears all such state so that
 * those methods return nullptr until a new file is parsed.
 */
void ObjParser::clear() {
    textures.clear();
    materials.clear();
    models.clear();
}

#pragma mark -
#pragma mark OBJ Data
/**
 * Processes a line representing an "o" command in an OBJ file.
 *
 * @param begin The start of the line
 * @param end   The end of the line
 * @param obj   The current ModelInfo results
 */
void ObjParser::processObject(const char* begin, const char* end, const std::shared_ptr<ModelInfo>& obj) {
    if (begin+1 == end || !isSkippable(*(begin+1))) {
        if (debug) CULogError("Unrecognized OBJ command: %s",begin);
        return;
    }
     
    const char* left = begin+1;
    while (left != end && isSkippable(*left)) {
        left++;
    }
    const char* right = left;
    while (right != end && !isSkippable(*right) && *right != '#') {
        right++;
    }
    if (left == right) {
        if (debug) CULogError("Invalid object name: %s",begin);
        return;
    }

    std::string name(left,right);
    auto group = obj->acquireGroup();
    group->object = name;
}

/**
 * Processes a line representing an "mtllib" command in an OBJ file.
 *
 * @param begin The start of the line
 * @param end   The end of the line
 * @param obj   The current ModelInfo results
 */
void ObjParser::processImport(const char* begin, const char* end, const std::shared_ptr<ModelInfo>& obj)  {
    const char* key = "mtllib";
    size_t len = strlen(key);
    
    if (begin+len+1 >= end) {
        if (debug) CULogError("Unrecognized OBJ command: %s",begin);
        return;
    }
    
    char command[7]; // Cannot use variables on stack, so magic number required
    memcpy(command, begin, len);
    command[len] = 0;
    if (strcmp(command, key) != 0) {
        if (debug) CULogError("Unrecognized OBJ command: %s",begin);
    }
    
    const char* left = begin+len;
    while (left != end && isSkippable(*left)) {
        left++;
    }
    const char* right = left;
    while (right != end && !isSkippable(*right) && *right != '#') {
        right++;
    }
    if (left == right) {
        if (debug) CULogError("Invalid library name: %s",begin);
        return;
    }

    // Create a placeholder for future parsing
    std::string lib(left,right);
    obj->libraries[lib] = nullptr;
}


/**
 * Processes a line representing a "v" command in an OBJ file.
 *
 * @param begin The start of the line
 * @param end   The end of the line
 * @param obj   The current ModelInfo results
 */
void ObjParser::processVertex(const char* begin, const char* end, const std::shared_ptr<ModelInfo>& obj) {
    if (begin+1 == end) {
        if (debug) CULogError("Unrecognized vertex command: %s",begin);
        return;
    }
    
    char c = *(begin+1);
    if (!isSkippable(c)) {
        switch (c) {
            case 'n':
                processNormal(begin, end, obj);
                break;
            case 't':
                processTexCoord(begin, end, obj);
                break;
            default:
                if (debug) CULogError("Unsupported vertex command: %s",begin);
                break;
        }
        return;
    }
    
    float data[3];
    int index = 0;
    
    const char* curr = begin+1;
    while(index < 3) {
        while (curr != end && isSkippable(*curr) && *curr != '#') {
            curr++;
        }
        if (curr == end || *curr == '#') {
            index = 4;
        }
        char* right;
        data[index++] = std::strtof(curr,&right);
        curr = right;
    }
    
    if (index == 4) {
        if (debug) CULogError("Could not parse command: %s",begin);
        return;
    }

    Vec3 result;
    result.x = data[0];
    result.y = data[1];
    result.z = data[2];
    obj->positions.push_back(result);
}

/**
 * Processes a line representing a "vt" command in an OBJ file.
 *
 * @param begin The start of the line
 * @param end   The end of the line
 * @param obj   The current ModelInfo results
 */
void ObjParser::processTexCoord(const char* begin, const char* end, const std::shared_ptr<ModelInfo>& obj) {
    if (begin+2 >= end || !isSkippable(*(begin+2))) {
        if (debug) CULogError("Unrecognized tex coord command: %s",begin);
        return;
    }
    
    float data[2];
    int index = 0;
    
    const char* curr = begin+2;
    while(index < 2) {
        while (curr != end && isSkippable(*curr) && *curr != '#') {
            curr++;
        }
        if (curr == end || *curr == '#') {
            index = 3;
        }
        char* right;
        data[index++] = std::strtof(curr,&right);
        curr = right;
    }
    
    if (index == 3) {
        if (debug) CULogError("Could not parse command: %s",begin);
        return;
    }

    Vec2 result;
    result.x = data[0];
    result.y = 1-data[1];
    obj->texcoords.push_back(result);
}

/**
 * Processes a line representing a "vn" command in an OBJ file.
 *
 * @param begin The start of the line
 * @param end   The end of the line
 * @param obj   The current ModelInfo results
 */
void ObjParser::processNormal(const char* begin, const char* end, const std::shared_ptr<ModelInfo>& obj) {
    if (begin+2 >= end || !isSkippable(*(begin+2))) {
        if (debug) CULogError("Unrecognized normal command: %s",begin);
        return;
    }
    
    float data[3];
    int index = 0;
    
    const char* curr = begin+2;
    while(index < 3) {
        while (curr != end && isSkippable(*curr) && *curr != '#') {
            curr++;
        }
        if (curr == end || *curr == '#') {
            index = 4;
        }
        char* right;
        data[index++] = std::strtof(curr,&right);
        curr = right;
    }
    
    if (index == 4) {
        if (debug) CULogError("Could not parse command: %s",begin);
        return;
    }
    
    Vec3 result;
    result.x = data[0];
    result.y = data[1];
    result.z = data[2];
    obj->normals.push_back(result);
}

/**
 * Processes a line representing a "usemtl" command in an OBJ file.
 *
 * @param begin The start of the line
 * @param end   The end of the line
 * @param obj   The current ModelInfo results
 */
void ObjParser::processUsage(const char* begin, const char* end, const std::shared_ptr<ModelInfo>& obj) {
    const char* key = "usemtl";
    size_t len = strlen(key);
    
    if (begin+len+1 >= end) {
        if (debug) CULogError("Unrecognized OBJ command: %s",begin);
        return;
    }
    
    char command[7]; // Cannot use variables on stack, so magic number required
    memcpy(command, begin, len);
    command[len] = 0;
    if (strcmp(command, key) != 0) {
        if (debug) CULogError("Unrecognized OBJ command: %s",begin);
    }
    
    // Get the name
    const char* left = begin+len;
    while (left != end && isSkippable(*left)) {
        left++;
    }
    const char* right = left;
    while (right != end && !isSkippable(*right) && *right != '#') {
        right++;
    }
    if (left == right) {
        if (debug) CULogError("Invalid material name: %s",begin);
        return;
    }

    std::string name(left,right);
    auto group = obj->currentGroup();
    if (group == nullptr || group->touched || group->material != "") {
        group = obj->acquireGroup();
    }
    obj->material = name;
    group->material = name;
}

/**
 * Processes a line representing a "g" command in an OBJ file.
 *
 * @param begin The start of the line
 * @param end   The end of the line
 * @param obj   The current ModelInfo results
 */
void ObjParser::processGroup(const char* begin, const char* end, const std::shared_ptr<ModelInfo>& obj) {
    if (begin+1 == end || !isSkippable(*(begin+1))) {
        if (debug) CULogError("Unrecognized OBJ command: %s",begin);
        return;
    }
    
    std::shared_ptr<GroupInfo> group = obj->currentGroup();

    const char* curr = begin+1;
    while (curr < end) {
        const char* left = curr;
        while (left != end && isSkippable(*left)) {
            left++;
        }
        const char* right = left;
        while (right != end && !isSkippable(*right) && *right != '#') {
            right++;
        }
        if (left != right) {
            if (group == nullptr || group->touched) {
                group = obj->acquireGroup();
            }
            group->tags.emplace(left,right);
            curr = right+1;
        } else {
            curr = end;
        }
    }
}

/**
 * Processes a line representing an "s" command in an OBJ file.
 *
 * @param begin The start of the line
 * @param end   The end of the line
 * @param obj   The current ModelInfo results
 */
void ObjParser::processSmooth(const char* begin, const char* end, const std::shared_ptr<ModelInfo>& obj)  {
    if (begin+1 == end || !isSkippable(*(begin+1))) {
        if (debug) CULogError("Unrecognized OBJ command: %s",begin);
        return;
    }
    
    const char* curr = begin+2;
    char* right;
    unsigned index = (unsigned)std::strtoul(curr,&right,10);
    if (curr == right) {
        if (debug) CULogError("Unrecognized index: %s",begin);
        return;
    }
    
    auto group = obj->acquireGroup();
    group->index = index;
}

/**
 * Processes a line representing an "f" command in an OBJ file.
 *
 * @param begin The start of the line
 * @param end   The end of the line
 * @param obj   The current ModelInfo results
 */
void ObjParser::processFace(const char* begin, const char* end, const std::shared_ptr<ModelInfo>& obj) {
    if (begin+1 == end || !isSkippable(*(begin+1))) {
        if (debug) CULogError("Unrecognized OBJ command: %s",begin);
        return;
    }
    
    // Get the current group
    std::shared_ptr<GroupInfo> group = obj->currentGroup();
    if (group == nullptr || (group->command != GL_FALSE && group->command != GL_TRIANGLES)) {
        group = obj->acquireGroup();
    }
    
    const char* curr = begin+2;
    GLuint count = (GLuint)group->vertCache.size();
    std::vector<GLuint> indices;
    while (curr < end) {
        VertexInfo vert;
        curr = parseVertex(curr, end, vert);
        if (vert.pindex != -1) {
            // Search for it in the index
            auto find = group->vertCache.find(vert);
            if (find == group->vertCache.end()) {
                indices.push_back(count);
                group->vertices.push_back(vert);
                group->vertCache[vert] = count++;
            } else {
                indices.push_back(find->second);
            }
        }
    }
    
    // Faces are added as triangle fans
    group->command = GL_TRIANGLES;
    group->touched = false;
    GLuint base = indices[0];
    GLuint left = indices[1];
    for(auto it = indices.begin()+2; it != indices.end(); ++it) {
        GLuint right = *it;
        group->indices.push_back(base);
        group->indices.push_back(left);
        group->indices.push_back(right);
        left = right;
    }
}

/**
 * Processes a line representing an "l" command in an OBJ file.
 *
 * @param begin The start of the line
 * @param end   The end of the line
 * @param obj   The current ModelInfo results
 */
void ObjParser::processLine(const char* begin, const char* end, const std::shared_ptr<ModelInfo>& obj) {
    if (begin+1 == end || !isSkippable(*(begin+1))) {
        if (debug) CULogError("Unrecognized OBJ command: %s",begin);
        return;
    }
    
    // Get the current group
    std::shared_ptr<GroupInfo> group = obj->currentGroup();
    if (group == nullptr || (group->command != GL_FALSE && group->command != GL_LINES)) {
        group = obj->acquireGroup();
    }
    
    const char* curr = begin+2;
    GLuint count = (GLuint)group->vertCache.size();
    std::vector<GLuint> indices;
    while (curr < end) {
        VertexInfo vert;
        curr = parseVertex(curr, end, vert);
        if (vert.pindex != -1) {
            // Search for it in the index
            auto find = group->vertCache.find(vert);
            if (find == group->vertCache.end()) {
                indices.push_back(count);
                group->vertices.push_back(vert);
                group->vertCache[vert] = count++;
            } else {
                indices.push_back(find->second);
            }
        }
    }
    
    // Lines are added as pairs
    group->command = GL_LINES;
    group->touched = false;
    GLuint left = indices[0];
    for(auto it = indices.begin()+1; it != indices.end(); ++it) {
        GLuint right = *it;
        group->indices.push_back(left);
        group->indices.push_back(right);
        left = right;
    }
}

/**
 * Processes a line representing a "p" command in an OBJ file.
 *
 * @param begin The start of the line
 * @param end   The end of the line
 * @param obj   The current ModelInfo results
 */
void ObjParser::processPoints(const char* begin, const char* end, const std::shared_ptr<ModelInfo>& obj) {
    if (begin+1 == end || !isSkippable(*(begin+1))) {
        if (debug) CULogError("Unrecognized OBJ command: %s",begin);
        return;
    }
    
    // Get the current group
    std::shared_ptr<GroupInfo> group = obj->currentGroup();
    if (group == nullptr || (group->command != GL_FALSE && group->command != GL_POINTS)) {
        group = obj->acquireGroup();
    }
    
    const char* curr = begin+2;
    GLuint count = (GLuint)group->vertCache.size();
    std::vector<GLuint> indices;
    while (curr < end) {
        VertexInfo vert;
        curr = parseVertex(curr, end, vert);
        if (vert.pindex != -1) {
            // Search for it in the index
            auto find = group->vertCache.find(vert);
            if (find == group->vertCache.end()) {
                indices.push_back(count);
                group->vertices.push_back(vert);
                group->vertCache[vert] = count++;
            } else {
                indices.push_back(find->second);
            }
        }
    }
    
    // Points are added individually
    group->command = GL_POINTS;
    group->touched = false;
    group->indices.insert(group->indices.end(),indices.begin(),indices.end());
}

#pragma mark -
#pragma mark MTL Data

/**
 * Processes a line representing a "material" command in a MTL file.
 *
 * @param begin The start of the line
 * @param end   The end of the line
 * @param obj   The current ModelInfo results
 */
void ObjParser::processMaterial(const char* begin, const char* end, const std::shared_ptr<MaterialLib>& mtl) {
    const char* key = "newmtl";
    size_t len = strlen(key);
    
    if (begin+len+1 >= end) {
        if (debug) CULogError("Unrecognized MTL command: %s",begin);
        return;
    }
    
    char command[7]; // Cannot use variables on stack, so magic number required
    memcpy(command, begin, len);
    command[len] = 0;
    if (strcmp(command, key) != 0) {
        if (debug) CULogError("Unrecognized MTL command: %s",begin);
    }
    
    // Get the name
    const char* left = begin+len;
    while (left != end && isSkippable(*left)) {
        left++;
    }
    const char* right = left;
    while (right != end && !isSkippable(*right) && *right != '#') {
        right++;
    }
    if (left == right) {
        if (debug) CULogError("Invalid material name: %s",begin);
        return;
    }

    std::string name(left,right);
    auto material = mtl->acquireMaterial(name);
    material->name = name;
}

/**
 * Processes a line representing an "illum" command in a MTL file.
 *
 * @param begin The start of the line
 * @param end   The end of the line
 * @param obj   The current ModelInfo results
 */
void ObjParser::processIllum(const char* begin, const char* end, const std::shared_ptr<MaterialLib>& mtl) {
    const char* key = "illum";
    size_t len = strlen(key);
    
    if (begin+len+1 >= end) {
        if (debug) CULogError("Unrecognized MTL command: %s",begin);
        return;
    }
    
    char command[6]; // Cannot use variables on stack, so magic number required
    memcpy(command, begin, len);
    command[len] = 0;
    if (strcmp(command, key) != 0) {
        if (debug) CULogError("Unrecognized MTL command: %s",begin);
    }
    
    const char* curr = begin+len;
    char* right;
    unsigned illum = (GLuint)std::strtoul(curr,&right,10);
    if (curr == right) {
        if (debug) CULogError("Unrecognized illum: %s",begin);
        return;
    }
    
    auto material = mtl->currentMaterial();
    if (material == nullptr) {
        if (debug) CULogError("MTL command on undefined material");
        return;
    }
    
    material->illum = illum;
}

/**
 * Processes a line representing an "Ns" command in a MTL file.
 *
 * @param begin The start of the line
 * @param end   The end of the line
 * @param obj   The current ModelInfo results
 */
void ObjParser::processShininess(const char* begin, const char* end, const std::shared_ptr<MaterialLib>& mtl) {
    const char* key = "Ns";
    size_t len = strlen(key);
    
    if (begin+len+1 >= end) {
        if (debug) CULogError("Unrecognized MTL command: %s",begin);
        return;
    }
    
    char command[3]; // Cannot use variables on stack, so magic number required
    memcpy(command, begin, len);
    command[len] = 0;
    if (strcmp(command, key) != 0) {
        if (debug) CULogError("Unrecognized MTL command: %s",begin);
    }
    
    const char* curr = begin+len;
    char* right;
    float ns = std::strtof(curr,&right);
    if (curr == right) {
        if (debug) CULogError("Unrecognized shininess: %s",begin);
        return;
    }
    
    auto material = mtl->currentMaterial();
    if (material == nullptr) {
        if (debug) CULogError("MTL command on undefined material");
        return;
    }
    
    material->Ns = ns;
}

/**
 * Processes a line representing a "K*" command in a MTL file.
 *
 * @param begin The start of the line
 * @param end   The end of the line
 * @param obj   The current ModelInfo results
 */
void ObjParser::processColor(const char* begin, const char* end, const std::shared_ptr<MaterialLib>& mtl) {
    char suff = *(begin+1);
    Color4f* color;
    
    auto material = mtl->currentMaterial();
    if (material == nullptr) {
        if (debug) CULogError("MTL command on undefined material");
        return;
    }
    
    switch (suff) {
        case 'a':
            color = &(material->Ka);
            break;
        case 'd':
            color = &(material->Kd);
            break;
        case 's':
            color = &(material->Ks);
            break;
        default:
            if (debug) CULogError("Unrecognized MTL command: %s",begin);
            return;
    }
    
    suff = *(begin+2);
    if (!isSkippable(suff)) {
        if (debug) CULogError("Unrecognized MTL command: %s",begin);
        return;
    }
    
    float data[3];
    int index = 0;
    
    const char* curr = begin+2;
    while(index < 3) {
        while (curr != end && isSkippable(*curr) && *curr != '#') {
            curr++;
        }
        if (curr == end || *curr == '#') {
            index = 4;
        }
        char* right;
        data[index++] = std::strtof(curr,&right);
        curr = right;
    }
    
    if (index == 4) {
        if (debug) CULogError("Could not parse command: %s",begin);
        return;
    }

    color->r = data[0];
    color->g = data[1];
    color->b = data[2];
    color->a = 1.0f;
}

/**
 * Processes a line representing a "map_*" command in a MTL file.
 *
 * @param begin The start of the line
 * @param end   The end of the line
 * @param obj   The current ModelInfo results
 */
std::shared_ptr<TextureInfo> ObjParser::processTexture(const char* begin, const char* end, const std::shared_ptr<MaterialLib>& mtl) {
    // This one actually needs to be exploded
    std::vector<std::string> tokens;
    
    // Get rid of initial space
    const char* left  = begin;
    while (left != end) {
        // Skip initial whitespace
        while (left != end && isSkippable(*left)) {
            left++;
        }
        
        const char* right = left;
        while (right != end && !isSkippable(*right)) {
            right++;
        }
        if (left != right) {
            tokens.emplace_back(left,right);
        }
        left = right;
    }
    
    if (tokens.size() < 2) {
        return nullptr;
    }
    
    auto material = mtl->currentMaterial();
    if (material == nullptr) {
        if (debug) CULogError("MTL command on undefined material");
        return nullptr;
    }
    
    std::shared_ptr<TextureInfo> texture = std::make_shared<TextureInfo>();
    if (tokens.front() == "map_Ka") {
        material->map_Ka = texture;
    } else if (tokens.front() == "map_Kd") {
        material->map_Kd = texture;
    } else if (tokens.front() == "map_Ks") {
        material->map_Ks = texture;
    } else if (tokens.front() == "map_Kn" || tokens.front() == "bump") {
        material->map_Kn = texture;
    } else {
        return nullptr;
    }
    
    // Last is the file.
    // Because textures may be relocated, do not check for path.
    texture->name = tokens.back();
    
    // Now process options
    if (tokens.size() > 2) {
        bool stop = false;
        for(auto it = tokens.begin()+1; !stop && it != tokens.end()-1; ++it) {
            if (it->c_str()[0] != '-') {
                if (debug) CULogError("Unrecognized texture command: %s",it->c_str());
                if (++it >= tokens.end()-1) { stop = true; }
            } else if (*it == "-blendu" || *it == "-blendv" || *it == "-cc" ||
                       *it == "-texres" || *it == "-imfchan") {
                if (debug) CULogError("Unsupported texture option: %s",it->c_str());
                if (++it >= tokens.end()-1) { stop = true; }
            } else if (*it == "-mm") {
                if (debug) CULogError("Unsupported texture option: %s",it->c_str());
                it += 2;
                if (it >= tokens.end()-1) { stop = true; }
            } else if (*it == "-o" || *it == "-s" || *it == "-t") {
                if (debug) CULogError("Unsupported texture option: %s",it->c_str());
                it += 3;
                if (it >= tokens.end()-1) { stop = true; }
            } else if (*it == "-clamp") {
                it++;
                if (it == tokens.end()) {
                    stop = true;
                } else if (*it == "on") {
                    texture->wrapS = GL_CLAMP_TO_EDGE;
                    texture->wrapT = GL_CLAMP_TO_EDGE;
                } else if (*it == "off") {
                    texture->wrapS = GL_REPEAT;
                    texture->wrapT = GL_REPEAT;
                } else {
                    if (debug) CULogError("Unrecognized clamp option: %s",it->c_str());
                    stop = true;
                }
            }
        }
    }
    
    return texture;
}

#pragma mark -
#pragma mark VertexInfo
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
const char* ObjParser::parseVertex(const char* begin, const char* end, VertexInfo& info) {
    const char* left = begin;
    while (left != end && isSkippable(*left)) {
        left++;
    }
    char* right;
    
    info.pindex = (int)std::strtoul(left,&right,10)-1;
    if (left == right) {
        info.pindex = -1;
    }

    left = right;
    if (left != end && *left == '/') {
        left++;
        info.tindex = (int)std::strtoul(left,&right,10)-1;
        if (left == right) {
            info.tindex = -1;
        }
    }

    left = right;
    if (left != end && *left == '/') {
        left++;
        info.nindex = (int)std::strtoul(left,&right,10)-1;
        if (left == right) {
            info.nindex = -1;
        }
    }
    
    //CULog("%d/%d/%d",pindex,tindex,nindex);
    return right;
}
