#include "CUTreeManager.h"

using namespace cugl;
using namespace cugl::btree;

/**
 * Creates a new TreeInstance.
 *
 * @param instanceKey The string key uniquely identifying this TreeInstance.
 * @param actionMap   The user-defined map of leaf node behaviors for this TreeInstance.
 *
 * @return  true if the Instance was created successfully.
 */
bool TreeManager::createInstance(std::string instanceKey, const Node::ActionMap actionMap)
{
  if (_instances.find(instanceKey) != _instances.end())
  {
    return false;
  }
  std::shared_ptr<TreeInstance> instance = TreeInstance::alloc(instanceKey, this->tree, actionMap);
  this->_instances[instanceKey] = instance;
  return true;
}

/**
 * Creates a new TreeInstance with a Mercenne Twister engine seed.
 *
 * @param instanceKey The string key uniquely identifying this TreeInstance.
 * @param actionMap   The user-defined map of leaf node behaviors for this TreeInstance.
 * @param seed        The random seed for this execution.
 *
 * @return  true if the Instance was created successfully.
 */
bool TreeManager::createInstanceWithSeed(std::string instanceKey, const Node::ActionMap actionMap, unsigned int seed)
{
  if (_instances.find(instanceKey) != _instances.end())
  {
    return false;
  }
  std::shared_ptr<TreeInstance> instance = TreeInstance::allocWithSeed(instanceKey, this->tree, actionMap, seed);
  this->_instances[instanceKey] = instance;
  return true;
}

/**
 * Steps the specified tree instance managed by this Tree Manager.
 *
 * @param instanceKey The string key uniquely identifying this TreeInstance.
 */
void TreeManager::stepInstance(std::string instanceKey) {
  std::shared_ptr<TreeInstance> instance = _instances[instanceKey];
  if (instance == nullptr)
  {
      CULogError("No instance with key %s", instanceKey.c_str());
  }
  instance->step();
}
