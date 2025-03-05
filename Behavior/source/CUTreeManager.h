#ifndef __CU_TREE_MANAGER_H__
#define __CU_TREE_MANAGER_H__

#include "CUTreeInstance.h"
#include "CUBTreeParser.h"

namespace cugl
{
  namespace btree
  {
    /**
     * @class TreeManager
     * @brief Manages one @c BehaviorTree and multiple @c TreeInstance objects.
     *
     * A @c TreeManager is responsible for:
     * - Parsing a single behavior tree from JSON using @c BTreeParser.
     * - Creating and storing multiple @c TreeInstance objects (keyed by a string).
     * - Stepping instances individually or collectively.
     *
     * This allows a system to define one tree structure and run multiple
     * agents/instances each with separate state. The manager can spawn new
     * instances, reset them, or step them as needed.
     */
    class TreeManager
    {

    protected:
      /** A map of each TreeInstance that walks this tree */
      std::unordered_map<std::string, std::shared_ptr<TreeInstance>> _instances;

      /** A map of user defined logic nodes */
      Node::ActionMap _userLogicNodes;

      /** The behavior tree logic this manager walks its instances through */
      std::shared_ptr<BehaviorTree> tree;

      /** The behavior tree parser used for the manager */
      BTreeParser _parser;

    public:
      TreeManager() {};
      ~TreeManager() {};

      /**
       * Initializes a new TreeManager.
       *
       * @param jval      The json value defining this manager's behavior tree.
       * @param logicMap  The map of user-defined logic node behaviors for this manager.
       *
       * @return true if the TreeManager was initialized successfully.
       */
      bool init(std::shared_ptr<JsonValue> jval, Node::LogicMap logicMap)
      {
        this->_parser = BTreeParser();
        this->tree = _parser.parseFromJson(jval, logicMap);
        this->_instances = std::unordered_map<std::string, std::shared_ptr<TreeInstance>>();
        return true;
      }

      /**
       * Initializes a new TreeManager.
       *
       * @param jval      The json value defining this manager's behavior tree.
       *
       * @return true if the TreeManager was initialized successfully.
       */
      bool init(std::shared_ptr<JsonValue> jval)
      {
        this->_parser = BTreeParser();
        this->tree = _parser.parseFromJson(jval, {});
        this->_instances = std::unordered_map<std::string, std::shared_ptr<TreeInstance>>();
        return true;
      }

      /**
       * Returns a newly allocated tree manager
       *
       * This method requires user-provided definitions for any additional logic nodes.
       *
       * @param jval      The json value defining this manager's behavior tree.
       * @param logicMap  The map of user-defined logic node behaviors for this manager.
       *
       * @return a newly allocated TreeManager
       */
      static std::shared_ptr<TreeManager> alloc(std::shared_ptr<JsonValue> jval, Node::LogicMap logicMap)
      {
        std::shared_ptr<TreeManager> result = std::make_shared<TreeManager>();
        return (result->init(jval, logicMap)) ? result : nullptr;
      }

      /**
       * Returns a newly allocated tree manager
       *
       * This method requires user-provided definitions for any additional logic nodes.
       *
       * @param jval  The json value defining this manager's behavior tree.
       *
       * @return a newly allocated TreeManager
       */
      static std::shared_ptr<TreeManager> alloc(std::shared_ptr<JsonValue> jval)
      {
        std::shared_ptr<TreeManager> result = std::make_shared<TreeManager>();
        return (result->init(jval)) ? result : nullptr;
      }

      /**
       * Creates a new TreeInstance.
       *
       * @param instanceKey The string key uniquely identifying this TreeInstance.
       * @param actionMap   The user-defined map of leaf node behaviors for this TreeInstance.
       *
       * @return  true if the Instance was created successfully.
       */
      bool createInstance(std::string instanceKey, const Node::ActionMap actionMap);

      /**
       * Creates a new TreeInstance with a Mercenne Twister engine seed.
       *
       * @param instanceKey The string key uniquely identifying this TreeInstance.
       * @param actionMap   The user-defined map of leaf node behaviors for this TreeInstance.
       * @param seed        The random seed for this execution.
       *
       * @return  true if the Instance was created successfully.
       */
      bool createInstanceWithSeed(std::string instanceKey, const Node::ActionMap actionMap, unsigned int seed);

      /**
       * Steps the specified tree instance managed by this Tree Manager.
       *
       * @param instanceKey The string key uniquely identifying this TreeInstance.
       */
      void stepInstance(std::string instanceKey);
    };
  }
}

#endif /** __CU_TREE_MANAGER_H__ */