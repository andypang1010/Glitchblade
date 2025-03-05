#ifndef __CU_BEHAVIOR_TREE_H__
#define __CU_BEHAVIOR_TREE_H__

#include "CUNode.h"
#include <cugl/core/assets/CUJsonValue.h>

namespace cugl
{
  namespace btree
  {

    /**
     * @class BehaviorTree
     * @brief A hierarchical structure for organizing behavioral logic.
     *
     * This class represents the root of a behavior tree, which is a tree-like
     * arrangement of "nodes." The root node points to child nodes, and so on,
     * until leaf nodes define actual actions or conditions.
     */
    class BehaviorTree
    {
    public:
#pragma mark Values
      /** The root node of this tree. */
      std::shared_ptr<Node> rootNode;

#pragma mark -

      /**
       * Returns a vector of this tree's nodes sorted depth-first.
       *
       * @return a vector of this tree's nodes.
       */
      std::vector<std::shared_ptr<Node>> asVector();

#pragma mark Constructors
      /**
       * Creates a new degenerate BehaviorTree on the stack.
       *
       * The tree has no root node and must be initialized.
       *
       * NEVER USE A CONSTRUCTOR WITH NEW. If you want to allocate an object on
       * the heap, use one of the static constructors instead.
       */
      BehaviorTree() {};

      /**
       *  Deletes this tree, disposing all resources.
       */
      ~BehaviorTree() { dispose(); };

      /**
       * Disposes all of the resources used by this tree.
       */
      void dispose();

      /**
       * Initializes a Behavior Tree rooted at the given node.
       *
       * @param rootNode  The root node of this tree
       *
       * @return true if the initialization was successful.
       */
      bool init(const std::shared_ptr<Node> &rootNode)
      {
        this->rootNode = rootNode;
        return true;
      }

      /**
       * Initializes an empty Behavior Tree.
       *
       * @return true if the initialization was successful.
       */
      bool init()
      {
        rootNode = nullptr;
        return true;
      }

#pragma mark -
#pragma mark Static Constructors

      /**
       * This method creates a new behavior tree object with the specified root node.
       *
       * @return the object or nullptr if fails.
       */
      static std::shared_ptr<BehaviorTree> alloc(const std::shared_ptr<Node> &rootNode)
      {
        std::shared_ptr<BehaviorTree> result = std::make_shared<BehaviorTree>();
        return (result->init(rootNode) ? result : nullptr);
      }

      /**
       * This method creates a new behavior tree object using the default init method.
       *
       * @return the object or nullptr if fails.
       */
      static std::shared_ptr<BehaviorTree> alloc()
      {
        std::shared_ptr<BehaviorTree> result = std::make_shared<BehaviorTree>();
        return (result->init() ? result : nullptr);
      }
#pragma mark -
    };
  }
}

#endif /* __CU_BEHAVIOR_TREE_H__ */
