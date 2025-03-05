#include "CUBTreeParser.h"
using namespace cugl::btree;

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
std::shared_ptr<BehaviorTree> BTreeParser::parseFromJson(const std::shared_ptr<JsonValue>& json,
                                                         Node::LogicMap userDefinedLogic)
{
  // Check if JSON root exists and is a valid object
  if (json->isNull() || !json->isObject())
    return nullptr;

  // Create the root BehaviorTree object
  treeIndex = 0;
  std::shared_ptr<BehaviorTree> tree = BehaviorTree::alloc();
  tree->rootNode = parseNode(json->get("root"), userDefinedLogic); // Parse starting from "root" node
  return tree;
}

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
std::shared_ptr<Node> BTreeParser::parseNode(const std::shared_ptr<JsonValue>& nodeJson,
                                             Node::LogicMap userDefinedLogic)
{
  // Check if JSON node exists and is a valid object
  if (nodeJson.get()->isNull() || !nodeJson.get()->isObject())
  {
    return nullptr;
  }
  std::string executeKey = nodeJson->key();
  // Create the Node object
  std::shared_ptr<Node> node = std::make_shared<Node>();
  node->init(treeIndex, executeKey);
  treeIndex++;

  // Parse the node type, do w- inlude the enum in the json?
  if (node->predefined.count(executeKey) > 0)
  {
    node->type = Node::Type::Predefined;
    node->executeFunc = node->predefined[executeKey];
  }
  else if (userDefinedLogic.count(executeKey) > 0)
  {
    node->type = Node::Type::UserDefinedLogic;
    node->executeFunc = userDefinedLogic[executeKey](node);
  }
  else
  {
    node->type = Node::Type::UserDefinedLeaf;
    node->executeFunc = node->executeUserLeaf;
  }
  // Iterate the maps to parse the node data
  std::shared_ptr<JsonValue> parametersJson = nodeJson->get("parameters");
  if (parametersJson != nullptr && parametersJson->isObject())
  {
    // Iterate over all key-value pairs in the parameters object
    const auto &parameterChildren = parametersJson->children();
    for (const auto &paramChild : parameterChildren)
    {
      std::string paramKey = paramChild->key();
      if (paramChild->isNumber())
      {
        float paramValue = paramChild->asFloat();
        node->initialFloatParameters[paramKey] = paramValue;
      }
      else if (paramChild->isString())
      {
        std::string paramValue = paramChild->asString();
        node->initialStringParameters[paramKey] = paramValue;
      }
      else
      {
        // Handle other types if necessary, or ignore
      }
    }
  }

    // Parse the node children
    std::shared_ptr<JsonValue> childrenJson = nodeJson->get("children");
    if (childrenJson != nullptr) {
        node->maxChildren = (int)childrenJson->size();
        for (int i = 0; i < childrenJson->size(); i++) {
            std::shared_ptr<Node> child = parseNode(childrenJson->get(i), userDefinedLogic);
            node->addChildToNode(child);
        }
    }
    return node;
}
