#include "CUBehaviorTree.h"

using namespace cugl;
using namespace cugl::btree;

void BehaviorTree::dispose()
{
	rootNode = nullptr;
}

/**
 * Returns a vector of this tree's nodes sorted depth-first.
 *
 * @return a vector of this tree's nodes.
 */
std::vector<std::shared_ptr<Node>> BehaviorTree::asVector()
{
	std::vector<std::shared_ptr<Node>> nodeVector = std::vector<std::shared_ptr<Node>>();
	nodeVector.push_back(rootNode);
	return rootNode->asVector(nodeVector);
}
