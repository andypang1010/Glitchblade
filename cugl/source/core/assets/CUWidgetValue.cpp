//
//  CUWidgetValue.h
//  Cornell University Game Library (CUGL)
//
//  This module represents a JSON value that has substitution semantics. This
//  allows us to define "widgets", which are JSON subtrees that can be injected
//  into another JSON. We use this to manage heavily nested JSON structures,
//  such as those specifying Figma scene graphs.
//
//  This class uses our standard shared-pointer architecture.
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
//  Author: Walker Whitte and Graham Rutledge
//  Version: 7/3/24 (CUGL 3.0 reorganization)
//
#include <cugl/core/assets/CUWidgetValue.h>
#include <cugl/core/util/CUDebug.h>
#include <algorithm>
#include <sstream>

using namespace cugl;

/**
 * Returns the list of variables in this widget.
 *
 * The variables are JSON locations in the tree that can be substituted
 * with new JSON values. Variables names are used in conjuction with
 * {@link #substitute} to produce a new JSON value
 *
 * @return the list of variables in this widget.
 */
std::vector<std::string> WidgetValue::getVariables() const {
    if (!_json->has("variables")) {
        return std::vector<std::string>();
    }
    
    std::vector<std::string> result;
    std::shared_ptr<JsonValue> variables = _json->get("variables");
    for(int ii = 0; ii < variables->size(); ii++) {
        auto child = variables->get(ii);
        result.push_back(child->key());
    }

    return result;
}

/**
 * Returns the JSON value resulting from substituting the specified values.
 *
 * The values should be a JSON object whose entries are collection are some
 * subset of {@link #getVariables}. For each entry, this function will
 * find the internal node for that variable, and replace the subtree with
 * the value for that variable.
 *
 * This function creates a new JsonValue, and does not modify the widget.
 * Therefore, the widget can be safely reused for other substitutions.
 *
 * @param values    The variable substitutions
 *
 * @return the JSON value resulting from substituting the specified values.
 */
std::shared_ptr<JsonValue> WidgetValue::substitute(const std::shared_ptr<JsonValue>& values) const {

    std::shared_ptr<JsonValue> variables = _json->get("variables");
    std::shared_ptr<JsonValue> contents = _json->get("contents");
    std::string contentString = contents->toString();
    std::shared_ptr<JsonValue> contentCopy = JsonValue::allocWithJson(contentString);
    if (values) {
        for (int ii = 0; ii < values->size(); ii++) {
            auto child = values->get(ii);
            if (variables->has(child->key())) {
                bool found = true;
                std::shared_ptr<JsonValue> address = variables->get(child->key());
                std::shared_ptr<JsonValue> spotToChange = contentCopy;
                std::vector<std::string> sAry = address->asStringArray();
                for (std::string s : sAry) {
                    if (spotToChange->has(s)) {
                        spotToChange = spotToChange->get(s);
                    } else {
                        found = false;
                    }
                }
                if (found) {
                    spotToChange->merge(child);
                } else {
                    std::string err = "No variable found matching name " + child->key();
                    CULogError("%s",err.c_str());
                }
            }
        }
    }
    
    return contentCopy;
}

/**
 * Returns true if this widget value is valid.
 *
 * A valid widget value is a JSON in the correct format, and which does
 * not have any variables that are prefixes of another.
 *
 * @return true if this widget value is valid.
 */
bool WidgetValue::isValid() const {
    std::shared_ptr<JsonValue> variables = _json->get("variables");
    std::shared_ptr<JsonValue> contents = _json->get("contents");
    if (contents == nullptr) {
        return false;
    } else if (variables == nullptr) {
        return true;
    }
    
    // Stringify paths
    std::vector<std::string> paths;
    for(int ii = 0; ii < variables->size(); ii++) {
        JsonValue* child = variables->get(ii).get();
        if (child->isString()) {
            paths.push_back(child->asString());
        } else if (child->isArray()) {
            std::stringstream ss;
            for(int jj = 0; jj < child->size(); jj++) {
                ss << "/" << child->get(jj)->asString();
            }
            paths.push_back(ss.str());
        } else {
            return false;
        }
    }
    
    // Sort by string length
    std::sort(paths.begin(),paths.end(),
              [](const std::string& a, const std::string& b) { return a.size() < b.size(); });
    
    // Look for prefixes
    for(int ii = 0; ii < paths.size()-1; ii++) {
        std::string* a = &paths[ii];
        std::string* b = &paths[ii+1];
        auto test = std::mismatch(a->begin(), a->end(), b->begin());
        if (test.first == a->end()) {
            return false;
        }
    }
    return true;
}
