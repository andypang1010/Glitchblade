#include "CUTreeInstance.h"
#include <vector>

using namespace cugl;
using namespace cugl::btree;

/**
 * Performs one step of this tree instance by executing the root node.
 */
void TreeInstance::step()
{
  Node::State rootState = (*nodeDataVector)[this->tree->rootNode->treeIndex].state;
  if (rootState != Node::State::Running)
  {
    reset();
  }

  this->tree->rootNode->execute(nodeDataVector);
}

/**
 * Dispose the TreeInstance object.
 */
void TreeInstance::dispose()
{
  tree = nullptr;
  nodeDataVector = nullptr;
}

/**
 * Resets the dynamic state of this tree by reseting the node data vector.
 */
void TreeInstance::reset()
{
  tree->rootNode->reset(nodeDataVector);
};

/**
 * Assigns this instance's execute functions to their associated NodeData objects.
 *
 * @param actionMap The map of user-defined leaf executes for this instance.
 *
 * @throws exception if a user-defined leaf node has no matching function in the map.
 */
void TreeInstance::assignLeafExecutes(Node::ActionMap actionMap)
{
  if (actionMap.empty())
  {
    return;
  }

  for (std::shared_ptr<Node> node : tree->asVector())
  {
    if (node->type == Node::Type::UserDefinedLeaf)
    {
      if (!actionMap[node->executeKey])
      {
          throw std::runtime_error("User-defined leaf node has no matching key in action map.");
      }
      (*nodeDataVector)[node->treeIndex].userDefinedExecute = actionMap[node->executeKey];
    }
  }
}

/**
 * Initializes the node data vector for the tree rooted at the given node.
 *
 * @param node        The root node of the tree represented by the node data vector.
 * @param dataVector  The vector to populate with node data.
 */
void TreeInstance::createNodeDataVector(std::shared_ptr<Node> node, std::shared_ptr<Node::NodeDataVector> dataVector)
{
  auto nodeData = std::make_shared<Node::NodeData>();

  nodeData->floatParameters = node->initialFloatParameters;
  nodeData->stringParameters = node->initialStringParameters;

  (*dataVector).push_back(*nodeData);

  for (auto child : node->children)
  {
    createNodeDataVector(child, dataVector);
  }
}
