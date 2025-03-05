#include "CUNode.h"
#include <iostream>

using namespace cugl;
using namespace cugl::btree;

/**
 * Adds a node as a child of this node.
 *
 * @param child The node to be added as a child to this node.
 *
 * @return true if the child was successfully added.
 */
bool Node::addChildToNode(std::shared_ptr<Node> child)
{
    if (children.size() >= maxChildren)
    {
        return false;
    }

    children.push_back(child);
    return true;
}

/**
 * Dispose this node and its children.
 */
void Node::dispose()
{
    initialFloatParameters.clear();
    initialStringParameters.clear();
    initialIntParameters.clear();
    for (auto it = children.begin(); it != children.end(); it++)
    {
        *it = nullptr;
    }
    children.clear();
}

/**
 * Reset this node's parameter values to default values.
 *
 * @param data The NodeDataVector of the current tree.
 */
void Node::reset(std::shared_ptr<NodeDataVector> data)
{
    (*data)[treeIndex].state = State::Fresh;
    (*data)[treeIndex].currentChildIndex = 0;
    (*data)[treeIndex].counter = 0;
    (*data)[treeIndex].shuffledChildren.clear();
    (*data)[treeIndex].floatParameters = initialFloatParameters;
    (*data)[treeIndex].stringParameters = initialStringParameters;
    (*data)[treeIndex].intParameters = initialIntParameters;
    unsigned int oldSeed = (*data)[0].seed;
    if (treeIndex == 0 && oldSeed != 0)
    {
        std::mt19937 engine(oldSeed);
        (*data)[0].seed = engine();
    }
    for (std::shared_ptr<Node> child : children)
    {
        child->reset(data);
    }
}

/**
 * Returns the child node at the given index.
 *
 * @throws out_of_range if childIndex is greater than the number of children or less than zero.
 *
 * @param childIndex The index of the child node.
 *
 * @return the child node at the given index.
 */
std::shared_ptr<Node> Node::getChild(int childIndex)
{
    if (childIndex >= children.size() || childIndex < 0)
    {
        throw std::out_of_range("Child index out of range");
    }

    else
    {
        return this->children[childIndex];
    }
}

/**
 * Get the data of a specific node from the NodeDataVector.
 *
 * @param data the NodeDataVector.
 * @param treeIndex index of the acquired node.
 *
 * @return the NodeData object of that node.
 */
Node::NodeData Node::getNodeData(std::shared_ptr<NodeDataVector> data, int treeIndex)
{
    if (treeIndex >= data->size() || treeIndex < 0)
    {
        throw std::out_of_range("Tree index out of range");
    }
    else
    {
        return (*data)[treeIndex];
    }
}

/**
 * Returns the tree rooted at this node a vector, sorted depth first.
 *
 * @param result   The vector to populate with nodes.
 *
 * @return a vector of the nodes in the tree rooted at this node.
 */
std::vector<std::shared_ptr<Node>> Node::asVector(std::vector<std::shared_ptr<Node>> &result)
{
    for (std::shared_ptr<Node> child : children)
    {
        result.push_back(child);
        child->asVector(result);
    }

    return result;
}