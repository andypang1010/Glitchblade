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
#ifndef __CU_WIDGET_VALUE_H__
#define __CU_WIDGET_VALUE_H__
#include <cugl/core/CUBase.h>
#include <cugl/core/assets/CUJsonValue.h>
#include <memory>
#include <vector>

namespace cugl {

/**
 * This class represents an externally defined widget in JSON form.
 *
 * A widget is a JSON with substitution semantics. That is, it has variables
 * that refer to nodes in the JSON tree, and allows these nodes to be replaced
 * with other JSON trees. The purpose of this is to manage heavily nested JSON 
 * structures, such as those specifying Figma scene graphs.
 *
 * More specifically, a widget value is a JSON object with two keys: "variables"
 * and "contents".  The former is JSON object with (string) keys mapping to
 * paths in "contents".  For example:
 *
 *
 *     {
 *         "variables" : {
 *             "first" :  ["outer", "inner", "one"],
 *             "second" : ["outer" , "middle"]
 *         },
 *             "contents : {
 *                 "outer" : {
 *                     "inner" : {
 *                         "one" : 1,
 *                         "two" : 2,
 *                 },
 *                 "middle": 3
 *             }
 *         }
 *     }
 *
 *
 * A call to {@link #substitute} on the JSON `{ "first": 4}` will return the
 * JSON object
 *
 *
 *     {
 *         "inner" : {
 *             "outer" : {
 *                 "one" : 4,
 *                 "two" : 2,
 *             },
 *             "middle": 3
 *         }
 *     }
 * 
 *
 * The substitution semantics is rather simple. That means it is undefined if
 * any of the variables are prefixes of one another. The method {@link #isValid}
 * is used to check for valid widgets.
 */
class WidgetValue {
protected:
	/** The JSON entry representing this widget */
	std::shared_ptr<JsonValue> _json;

public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a null WidgetValue.
     *
     * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
     * the heap, use one of the static constructors instead.
     */
    WidgetValue() { };
    
    /**
     * Deletes this WidgetValue and all of its resources.
     *
     * If no other references own the descendants of this node, they will all
     * be recursively deleted as well.
     */
	~WidgetValue() {};
    
    /**
     * Initializes a new WidgetValue to wrap the give JsonValue.
     *
     * This initializer simply wraps the provided JSON
     *
     * @param json  A shared pointer to JsonValue that defines this widget.
     *
     * @return  true if the JsonValue is not a nullptr, false otherwise.
     */
    bool init(const std::shared_ptr<JsonValue>& json) {
		if (json == nullptr) {
			return false;
		}

        _json = json;
		return true;
    }
    
    /**
     * Returns a newly allocated WidgetValue to wrap the given JsonValue.
     *
     * @param json  The JSON definition of this widget.
     *
     * @return a newly allocated WidgetValue wrapping the provided JsonValue.
     */
    static std::shared_ptr<WidgetValue> alloc(std::shared_ptr<JsonValue> json) {
        std::shared_ptr<WidgetValue> result = std::make_shared<WidgetValue>();
        return (result->init(json) ? result : nullptr);
    }

#pragma mark -
#pragma mark Access and Substitution

	/**
	 * Returns the JsonValue representation of this widget.
	 *
	 * @return a shared pointer to the JsonValue representation of this widget.
	 */
	const std::shared_ptr<JsonValue> getJson() const {
		return _json;
	}
    
    /**
     * Returns the list of variables in this widget.
     *
     * The variables are JSON locations in the tree that can be substituted
     * with new JSON values. Variables names are used in conjuction with
     * {@link #substitute} to produce a new JSON value
     *
     * @return the list of variables in this widget.
     */
    std::vector<std::string> getVariables() const;
    
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
    std::shared_ptr<JsonValue> substitute(const std::shared_ptr<JsonValue>& values) const;
    
    /**
     * Returns true if this widget value is valid.
     *
     * A valid widget value is a JSON in the correct format, and which does
     * not have any variables that are prefixes of another.
     *
     * @return true if this widget value is valid.
     */
    bool isValid() const;
};

}
#endif /* __CU_WIDGET_VALUE_H__ */
