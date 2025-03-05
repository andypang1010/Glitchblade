#ifndef __CU_BTREE_PARSER_H__
#define __CU_BTREE_PARSER_H__

#include <string>
#include <memory>
#include <iostream>

#include "CUBehaviorTree.h"

namespace cugl
{
  namespace btree
  {
    /**
     * @class BTreeParser
     * @brief A helper class for constructing a BehaviorTree from JSON data.
     *
     * This class reads a JsonValue of a behavior tree and constructs
     * the corresponding @c BehaviorTree object. It supports predefined node types,
     * user-defined logic, and user-defined leaf actions.
     */
    class BTreeParser
    {

      int treeIndex;

    public:
      BTreeParser() : treeIndex(0) {}
      ~BTreeParser() {};

      /**
       * Parses a BehaviorTree from a JsonValue object.
       *
       * This method reads the JSON representation of a behavior tree and constructs
       * a corresponding BehaviorTree object.The method supports predefined actions,
       * user-defined logic, and user-defined leaf nodes.
       *
       * The JSON structure must include a "root" node as the entry point.
       *
       * This method is the primary entry point for constructing a BehaviorTree from JSON data.
       *
       * @param json                The root JSON object containing the behavior tree definition.
       * @param userDefinedLogic    A mapping of user-defined logic keys to their functions.
       *
       * @return A shared pointer to the constructed BehaviorTree, or nullptr if the JSON is invalid.
       */
      std::shared_ptr<BehaviorTree> parseFromJson(const std::shared_ptr<JsonValue>& json,
                                                  Node::LogicMap userDefinedLogic);

      /**
       * Recursively parses a node from a JsonValue object.
       *
       * This method is used internally to parse individual nodes within the behavior tree.
       * It handles various node types (predefined actions, user-defined logic, and leaf nodes),
       * parses node parameters, and constructs the node hierarchy by linking child nodes.
       *
       * Each node is expected to have:
       * - A unique key that maps to predefined actions or user-defined logic.
       * - Optional parameters stored as numbers or strings.
       * - An optional array of child nodes.
       *
       * @param nodeJson            The JSON object representing the node.
       * @param userDefinedLogic    A mapping of user-defined logic keys to their functions.
       *
       * @return A shared pointer to the constructed Node, or nullptr if the JSON is invalid.
       */
      std::shared_ptr<Node> parseNode(const std::shared_ptr<JsonValue>& nodeJson,
                                      Node::LogicMap userDefinedLogic);
    };
  }
}

#endif /* __CU_BTREE_PARSER_H__ */
