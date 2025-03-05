#ifndef __CU_TREE_INSTANCE_H__
#define __CU_TREE_INSTANCE_H__

#include "CUBehaviorTree.h"
#include "CUNode.h"

namespace cugl
{
  namespace btree
  {
    /**
     * @class TreeInstance
     * @brief Represents a runtime instance of a behavior tree, holding per-node state.
     *
     * While @c BehaviorTree holds the static structure (nodes, hierarchy),
     * a @c TreeInstance tracks the **dynamic state** (node statuses, counters, timestamps).
     * This separation allows reusing one tree definition for multiple actors or simulations,
     * each with its own @c TreeInstance state.
     *
     * @c TreeInstance:
     * - Initializes a @c NodeDataVector for each @c Node.
     * - Tracks a per-instance random seed if needed.
     * - Provides @c step() to run the root node once each frame/loop iteration.
     * - Resets node states as needed (e.g., after success/failure).
     */
    class TreeInstance
    {
      /** The unique key for this tree instance */
      std::string instanceKey;
      /** The tree structure (behavior logic) which this instance follows */
      std::shared_ptr<BehaviorTree> tree;
      /** The current node data for each node in the tree. Captures the dynamic state of the tree. */
      std::shared_ptr<Node::NodeDataVector> nodeDataVector;

      /**
       * @param instanceKey The string key uniquely identifying this TreeInstance.
       * @param tree        The behvaior tree defining this instance's logic.
       * @param actionMap   The user-defined map of leaf node behaviors for this TreeInstance.
       *
       * @return true if this TreeInstance was successfully initialized.
       */
      bool init(std::string instanceKey, std::shared_ptr<BehaviorTree> tree, const Node::ActionMap actionMap)
      {
        if (instanceKey.empty())
          return false;
        std::random_device rd;
        this->instanceKey = instanceKey;
        this->tree = tree;
        std::shared_ptr<Node::NodeDataVector> dataVector = std::make_shared<Node::NodeDataVector>();
        unsigned int randSeed = rd();
        this->seed = randSeed;
        nodeDataVector = dataVector;
        this->createNodeDataVector(this->tree->rootNode, nodeDataVector);
        (*nodeDataVector)[0].seed = randSeed;
        assignLeafExecutes(actionMap);

        return true;
      }
      /**
       * @param instanceKey The string key uniquely identifying this TreeInstance.
       * @param tree        The behvaior tree defining this instance's logic.
       * @param actionMap   The user-defined map of leaf node behaviors for this TreeInstance.
       * @param seed        The random seed for the Mercenne Twister engine for the instance.
       *
       * @return true if this TreeInstance was successfully initialized.
       */
      bool initWithSeed(std::string instanceKey, std::shared_ptr<BehaviorTree> tree, const Node::ActionMap actionMap, unsigned int seed)
      {
        if (instanceKey.empty())
          return false;

        this->instanceKey = instanceKey;
        this->tree = tree;
        std::shared_ptr<Node::NodeDataVector> dataVector = std::make_shared<Node::NodeDataVector>();
        nodeDataVector = dataVector;
        this->createNodeDataVector(this->tree->rootNode, nodeDataVector);
        (*nodeDataVector)[0].seed = seed;
        assignLeafExecutes(actionMap);
        // CULog("Instance init with seed %u", seed);
        // CULog("seed is now %u", (*nodeDataVector)[0].seed);
        return true;
      }

      /**
       * Assigns this instance's execute functions to their associated NodeData objects.
       *
       * @param actionMap The map of user-defined leaf executes for this instance.
       *
       * @throws exception if a user-defined leaf node has no matching function in the map.
       */
      void assignLeafExecutes(Node::ActionMap actionMap);

      /**
       * Initializes the node data vector for the tree rooted at the given node.
       *
       * @param node        The root node of the tree represented by the node data vector.
       * @param dataVector  The vector to populate with node data.
       */
      void createNodeDataVector(std::shared_ptr<Node> node, std::shared_ptr<Node::NodeDataVector> dataVector);

    public:
      /** The unique seed for this tree instance's Mercenne Twister engine */
      unsigned int seed = 0;

      /**
       * Resets the dynamic state of this tree by reseting the node data vector.
       */
      void reset();

      TreeInstance() {};

      ~TreeInstance() { dispose(); };

      /**
       * Dispose the TreeInstance object.
       */
      void dispose();

      /**
       * Returns a newly allocated tree instance.
       *
       * @param instanceKey The string key uniquely identifying this TreeInstance.
       * @param tree        The behvaior tree defining this instance's logic.
       * @param actionMap   The user-defined map of leaf node behaviors for this TreeInstance.
       *
       * @return a newly allocated tree instance.
       */
      static std::shared_ptr<TreeInstance> alloc(std::string instanceKey, std::shared_ptr<BehaviorTree> tree, const Node::ActionMap actionMap)
      {
        std::shared_ptr<TreeInstance> result = std::make_shared<TreeInstance>();
        return (result->init(instanceKey, tree, actionMap)) ? result : nullptr;
      }

      /**
       * Returns a newly allocated tree instance. This tree instance has a built-in seed for the Mercenne Twister engine.
       *
       * @param instanceKey The string key uniquely identifying this TreeInstance.
       * @param tree        The behvaior tree defining this instance's logic.
       * @param actionMap   The user-defined map of leaf node behaviors for this TreeInstance.
       * @param seed        The randon seed for this TreeInstance.
       *
       * @return a newly allocated tree instance.
       */
      static std::shared_ptr<TreeInstance> allocWithSeed(std::string instanceKey, std::shared_ptr<BehaviorTree> tree, const Node::ActionMap actionMap, unsigned int seed)
      {
        std::shared_ptr<TreeInstance> result = std::make_shared<TreeInstance>();
        return (result->initWithSeed(instanceKey, tree, actionMap, seed)) ? result : nullptr;
      }

      /**
       * Performs one step of this tree instance by executing the root node.
       */
      void step();
    };
  }
}

#endif /** __CU_TREE_INSTANCE_H__ */