#ifndef __CU_NODE_H__
#define __CU_NODE_H__

#include <cugl/core/util/CUTimestamp.h>
#include "memory"
#include "unordered_map"
#include "unordered_set"
#include "string"
#include "functional"
#include "cmath"
#include <algorithm>
#include <random>
#include <iostream>

namespace cugl
{
  namespace btree
  { /**
     * @class Node
     * @brief A single node in a behavior tree, responsible for logic or actions.
     *
     * Each Node can represent:
     * - A leaf action (e.g., wait, succeed, fail).
     * - A decorator (inverting, repeating, etc.).
     * - A composite behavior (selector, sequence, parallel).
     *
     * A Node stores children, parameters, and function pointers defining how it
     * executes. It also maintains a @c NodeData structure for its current state
     * (fresh, running, success, failure, cancelled).
     *
     * The @c execute() method runs the node’s logic or defers to children,
     * updating node data accordingly.
     *
     * @note @c NodeData contains runtime state separate from the node’s static
     * configuration, allowing for multiple simultaneous “instances” of the same tree.
     */
    class Node
    {
    public:
      /** The possible states of nodes */
      enum State
      {
        Fresh,
        Running,
        Failure,
        Success,
        Cancelled
      };

      class NodeData
      {
      public:
        /** The current state of this node in this instance. */
        State state = State::Fresh;

        /** The index of the child this node is running. */
        int currentChildIndex = 0;

        /** The counter value for this node. */
        int counter = 0;

        /** The timestamp for when this node started running. */
        Timestamp start;

        /** The user-defined execute function for this node, if it is a leaf. */
        std::function<State(std::shared_ptr<std::vector<NodeData>>)> userDefinedExecute;

        /** A map of additional float parameters. */
        std::unordered_map<std::string, float> floatParameters;

        /** A map of additional string parameters. */
        std::unordered_map<std::string, std::string> stringParameters;

        /** A map of additional integer parameters. */
        std::unordered_map<std::string, int> intParameters;

        /** A vector of child indicies for random branch nodes. */
        std::vector<int> shuffledChildren = std::vector<int>();

        /** The reserved random seed for the Mersenne Twister engine */
        unsigned int seed;
      };

      /** Different types of Node, used for parsing the tree */
      enum Type
      {
        Predefined,
        UserDefinedLogic,
        UserDefinedLeaf
      };

      /** The type of a vector of node data, representing the dynamic state of a tree instance. */
      using NodeDataVector = std::vector<NodeData>;

      /** The type of a node execute function. */
      using ExecuteType = std::function<State(std::shared_ptr<NodeDataVector>)>;

      /** The type of user-defined maps from keys to leaf node execute functions. */
      using ActionMap = std::unordered_map<std::string, ExecuteType>;

      /**
       * The type of a wrapper around a user-defined logic lambda function of type ExecuteType.
       * Functions of this type are used to build a LogicMap to define a tree's custom logic (non-leaf) nodes.
       * They take a node as a parameter so that the node can be captured by the inner lambda.
       */
      using LogicFuncWrapper = std::function<Node::ExecuteType(std::shared_ptr<Node>)>;

      /** The type of user-defined maps from keys to non-leaf node execute functions. */
      using LogicMap = std::unordered_map<std::string, Node::LogicFuncWrapper>;

      /** The map of float parameters set at parse-time. Used to reset values for the corresponding node data. */
      std::unordered_map<std::string, float> initialFloatParameters;

      /** The map of float parameters set at parse-time. Used to reset values for the corresponding node data. */
      std::unordered_map<std::string, std::string> initialStringParameters;

      /** The map of float parameters set at parse-time. Used to reset values for the corresponding node data. */
      std::unordered_map<std::string, int> initialIntParameters;

      /** A vector of this node's child nodes. */
      std::vector<std::shared_ptr<Node>> children;

      /** The maximum number of child nodes allowed for this node. */
      int maxChildren;

      /** Index of this node in the tree, ordered depth first. */
      int treeIndex;

      /** The type of this node. */
      Type type;

      /** The string key mapping to this node's execute function. */
      std::string executeKey;

      /** Default constructor */
      Node() {};

      /** Default destructor */
      ~Node() { dispose(); }

      /**
       * Dispose this node and its children.
       */
      void dispose();

      /**
       * Reset this node's parameter values to default values.
       *
       * @param data The NodeDataVector of the current tree
       */
      void reset(std::shared_ptr<NodeDataVector> data);

      /** This node's execute function, set by the TreeParser. */
      std::function<State(std::shared_ptr<NodeDataVector>)> executeFunc;

      /**
       * Execute the current node with this node's own execute function.
       *
       * @param data The NodeDataVector of the current tree
       */
      State execute(std::shared_ptr<NodeDataVector> data)
      {
        return executeFunc(data);
      }

      /**
       * Initialize a node with a string indicating its type.
       *
       * @param treeIndex this node's index in the tree.
       * @param actionKey the key for this node's execute function.
       *
       * @return true if the Node was successfully initialized.
       */
      bool init(int treeIndex, std::string actionKey)
      {
        if (treeIndex < 0 || actionKey.empty())
        {
          return false;
        }
        this->treeIndex = treeIndex;
        this->executeKey = actionKey;
        return true;
      }

      /**
       * Adds a node as a child of this node.
       *
       * @param child The node to be added as a child to this node.
       *
       * @return true if the child was successfully added.
       */
      bool addChildToNode(std::shared_ptr<Node> child);

      /**
       * Get the data of a specific node from the NodeDataVector
       *
       * @param data the NodeDataVector
       * @param treeIndex index of the acquired node
       *
       * @return the NodeData object of that node
       */
      NodeData getNodeData(std::shared_ptr<NodeDataVector> data, int treeIndex);

      /**
       * Returns the child node at the given index.
       *
       * @throws out_of_range if childIndex is greater than the number of children or less than zero.
       *
       * @param childIndex The index of the child node.
       *
       * @return the child node at the given index.
       */
      std::shared_ptr<Node> getChild(int childIndex);

      /**
       * Returns the tree rooted at this node a vector, sorted depth first.
       *
       * @param result  The vector to populate with nodes.
       *
       * @return a vector of the nodes in the tree rooted at this node.
       */
      std::vector<std::shared_ptr<Node>> asVector(std::vector<std::shared_ptr<Node>> &result);

      /**
       * Executes a user-defined leaf node. This node executes the user-defined function stored in
       * the node data vector passed in from executing instance.
       *
       * @param data  The node data vector from the tree instance running this node.
       *
       * @throw exception if data does not store an execute function.
       * @return the result of the user-defined execute function.
       */
      std::function<State(std::shared_ptr<NodeDataVector>)> executeUserLeaf = [this](std::shared_ptr<NodeDataVector> data)
      {
        ExecuteType execute = getNodeData(data, treeIndex).userDefinedExecute;
        if (!execute)
        {
            throw std::runtime_error("No matching user-defined execute in this tree instance.");
        }
        State result = execute(data);
        return result;
      };

      /**
       * Executes a root node. Executing this node will execute the tree rooted at this node.
       *
       * A root node has a single child and returns the same state as its child. When a root
       * node completes and returns success or failure it resets the tree.
       *
       * @param data  The node data vector from the tree instance running this node.
       *
       * @return the state its child returns.
       */
      ExecuteType executeRoot = [this](std::shared_ptr<NodeDataVector> data)
      {
          if (children.empty()) {
              return (*data)[treeIndex].state;
          }
          State state = getChild(0)->execute(data);
          (*data)[treeIndex].state = state;
          if (state == State::Success || state == State::Failure) {
              reset(data);
          }
          return (*data)[treeIndex].state;
      };

      /**
       * Executes a success leaf node.
       *
       * @param data  The node data vector from the tree instance running this node.
       *
       * @return success.
       */
      ExecuteType executeSuccess = [](std::shared_ptr<NodeDataVector> data)
      {
        return State::Success;
      };

      /**
       * Executes a failure leaf node.
       *
       * @param data  The node data vector from the tree instance running this node.
       *
       * @return failure.
       */
      ExecuteType executeFailure = [](std::shared_ptr<NodeDataVector> data)
      {
        return State::Failure;
      };

      /**
       * Executes a wait leaf node. This node runs until it has waited for a specified
       * duration and then returns success.
       *
       * Uses an additional parameter "seconds" defined in the JSON to determine wait duration.
       * If "seconds" is not specified in the JSON, this node will always return running.
       *
       * @param data  The node data vector from the tree instance running this node.
       *
       * @return running until this node has waited for the duration, then return success.
       */
      ExecuteType executeWait = [this](std::shared_ptr<NodeDataVector> data)
      {
        if ((*data)[this->treeIndex].state == State::Fresh)
        {
          Timestamp start;
          (*data)[this->treeIndex].start = start;
        }
        else if ((*data)[this->treeIndex].state == State::Running)
        {
          Timestamp end;
          Uint64 currTime = Timestamp::ellapsedMillis((*data)[this->treeIndex].start, end);
          int elapsed = static_cast<int>(currTime) - (*data)[this->treeIndex].intParameters["startTime"];

          if ((*data)[this->treeIndex].floatParameters.find("seconds") != (*data)[this->treeIndex].floatParameters.end())
          {
            float duration = (*data)[this->treeIndex].floatParameters["seconds"];
            if (elapsed >= duration * 1000)
            {
              (*data)[this->treeIndex].state = State::Success;
              return (*data)[this->treeIndex].state;
            }
          }
        }

        (*data)[this->treeIndex].state = State::Running;
        return (*data)[this->treeIndex].state;
      };

      /**
       * Executes a repeat decorator node. This node runs until its child has completed a specified number of times and then returns success.
       *
       * Uses an additional parameter "max repeats" defined in the JSON to determine the number of times to repeat.
       * If "max repeats" is not specified in the JSON, this node will repeat infinitely.
       *
       * @param data  The node data vector from the tree instance running this node.
       *
       * @return success when child has repeated the specified number of times and running otherwise.
       */
      ExecuteType executeRepeat = [this](std::shared_ptr<NodeDataVector> data)
      {
        int maxRepeats = static_cast<int>(round((*data)[this->treeIndex].floatParameters["max repeats"]));
        State childState = getChild(0)->execute(data);
        if (childState == State::Success || childState == State::Failure)
        {
          getChild(0)->reset(data);
          (*data)[this->treeIndex].counter++;
        }

        if ((*data)[this->treeIndex].counter >= maxRepeats)
        {
          return State::Success;
        }
        return State::Running;
      };

      /**
       * Executes an invert decorator node. This node returns success if its child fails and failure if its child succeeds.
       *
       * @param data  The node data vector from the tree instance running this node.
       *
       * @return success if its child fails, failure if its child succeeds, and running otherwise.
       */
      ExecuteType executeInvert = [this](std::shared_ptr<NodeDataVector> data)
      {
        State childState = getChild(0)->execute(data);
        if (childState == State::Success)
        {
          return State::Failure;
        }
        else if (childState == State::Failure)
        {
          return State::Success;
        }
        return State::Running;
      };

      /**
       * Executes an always succeed decorator node. This node returns success as soon as its child either succeeds or fails.
       *
       * @param data  The node data vector from the tree instance running this node.
       *
       * @return success when child completes and running otherwise.
       */
      ExecuteType executeAlwaysSucceed = [this](std::shared_ptr<NodeDataVector> data)
      {
        State childState = getChild(0)->execute(data);
        if (childState == State::Success || childState == State::Failure)
        {
          return State::Success;
        }

        return State::Running;
      };

      /**
       * Executes an always fail decorator node. This node returns failure as soon as its child either succeeds or fails.
       *
       * @param data  The node data vector from the tree instance running this node.
       *
       * @return failure when child completes and running otherwise.
       */
      ExecuteType executeAlwaysFail = [this](std::shared_ptr<NodeDataVector> data)
      {
        State childState = getChild(0)->execute(data);
        if (childState == State::Success || childState == State::Failure)
        {
          return State::Failure;
        }

        return State::Running;
      };

      /**
       * Executes an until success decorator node. This node runs its child until the child returns success, at which point
       * this node returns success. If the child returns failure, it is reset and ran again.
       *
       * @param data  The node data vector from the tree instance running this node.
       *
       * @return success when child returns success and running otherwise.
       */
      ExecuteType executeUntilSuccess = [this](std::shared_ptr<NodeDataVector> data)
      {
        State childState = getChild(0)->execute(data);
        if (childState == State::Success)
        {
          return State::Success;
        }
        else if (childState == State::Failure)
        {
          getChild(0)->reset(data);
        }
        return State::Running;
      };

      /**
       * Executes an until failure decorator node. This node runs its child until the child returns failure, at which point
       * this node returns success. If the child returns success, it is reset and ran again.
       *
       * @param data  The node data vector from the tree instance running this node.
       *
       * @return success when child returns failure and running otherwise.
       */
      ExecuteType executeUntilFailure = [this](std::shared_ptr<NodeDataVector> data)
      {
        State childState = getChild(0)->execute(data);
        if (childState == State::Failure)
        {
          return State::Success;
        }
        else if (childState == State::Success)
        {
          getChild(0)->reset(data);
        }
        return State::Running;
      };

      /**
       * Executes a selector branch node. This node runs its children in order, moving to the next when
       * its current child returns success or failure.
       *
       * @param data  The node data vector from the tree instance running this node.
       *
       * @return failure if all children fail, success as soon as any one child succeeds, and running otherwise.
       */
      ExecuteType executeSelector = [this](std::shared_ptr<NodeDataVector> data)
      {
        State childState = getChild((*data)[this->treeIndex].currentChildIndex)->execute(data);
        if (childState == State::Success)
        {
          reset(data);
          return State::Success;
        }
        else if (childState == State::Failure)
        {
          (*data)[this->treeIndex].currentChildIndex++;
        }
        if ((*data)[this->treeIndex].currentChildIndex >= this->children.size())
        {
          reset(data);
          return State::Failure;
        }
        return State::Running;
      };

      /**
       * Executes a sequence branch node. This node runs its children in order, moving to the next when
       * its current child returns success or failure.
       *
       * @param data  The node data vector from the tree instance running this node.
       *
       * @return success if all children succeed, failure as soon as any one child fails, and running otherwise.
       */
      ExecuteType executeSequence = [this](std::shared_ptr<NodeDataVector> data)
      {
        State childState = getChild((*data)[this->treeIndex].currentChildIndex)->execute(data);
        if (childState == State::Failure)
        {
          reset(data);
          return State::Failure;
        }
        else if (childState == State::Success)
        {
          (*data)[this->treeIndex].currentChildIndex++;
        }

        if ((*data)[this->treeIndex].currentChildIndex >= this->children.size())
        {
          reset(data);
          return State::Success;
        }
        return State::Running;
      };

      /**
       * Executes a parallel branch node. This node runs all chidren in parallel and returns based on its policy.
       *
       * "policy" is an additional parameter set in the JSON and can be either "sequence" or "selector".
       * If no policy is set then "policy" defaults to "sequence".
       *
       * A "sequence" parallel returns success if all children succeed and fails soon as any one child fails.
       * A "selector" parallel returns failure if all children fail and succeeds as soon as any on child succeeds.
       *
       * @param data  The node data vector from the tree instance running this node.
       *
       * @return running until this node's policy is satisfied.
       */
      ExecuteType executeParallel = [this](std::shared_ptr<NodeDataVector> data)
      {
        std::string policy;
        if ((*data)[treeIndex].stringParameters.find("policy") != (*data)[treeIndex].stringParameters.end())
        {
          policy = (*data)[treeIndex].stringParameters["policy"];
        }
        else
        {
          policy = "sequence";
        }

        for (std::shared_ptr<Node> child : children)
        {
          State childState = (*data)[child->treeIndex].state;

          if (childState == State::Fresh || childState == State::Running)
          {
            (*data)[child->treeIndex].state = child->execute(data);
            switch ((*data)[child->treeIndex].state)
            {
            case State::Success:
              (*data)[treeIndex].intParameters["finished children"]++;
              if (policy == "selector")
              {
                (*data)[treeIndex].state = State::Success;
                return State::Success;
              }
              else if (policy == "sequence" && (*data)[treeIndex].intParameters["finished children"] >= children.size())
              {
                (*data)[treeIndex].state = State::Success;
                return State::Success;
              }
            case State::Failure:
              (*data)[treeIndex].intParameters["finished children"]++;
              if (policy == "selector" && (*data)[treeIndex].intParameters["finished children"] >= children.size())
              {
                (*data)[treeIndex].state = State::Failure;
                return State::Failure;
              }
              else if (policy == "sequence")
              {
                (*data)[treeIndex].state = State::Failure;
                return State::Failure;
              }
            }
          }
        }

        return State::Running;
      };

      /**
       * Executes a random selector branch node. This node shuffles the order of its children on first execution
       * and then executes them as a selector.
       *
       * @param data  The node data vector from the tree instance running this node.
       *
       * @return failure when all children fail, success when any one child succeeds, and running otherwise.
       */
      ExecuteType executeRandomSelector = [this](std::shared_ptr<NodeDataVector> data)
      {
              if ((*data)[treeIndex].shuffledChildren.empty())
              {
                  for (int i = 0; i < children.size(); i++)
                  {
                      (*data)[treeIndex].shuffledChildren.push_back(i);
                  }
                  std::random_device rd;
                  unsigned int seed;
                  seed = (*data)[0].seed;
                  //std::mt19937 g(seed);
                  std::mt19937 g(rd());
                  std::shuffle((*data)[treeIndex].shuffledChildren.begin(), (*data)[treeIndex].shuffledChildren.end(), g);
              }
              State childState = getChild((*data)[treeIndex].shuffledChildren[(*data)[treeIndex].currentChildIndex])->execute(data);
              if (childState == State::Success)
              {
                  reset(data);
                  return State::Success;
              }
              else if (childState == State::Failure)
              {
                  (*data)[treeIndex].currentChildIndex++;
              }
              if ((*data)[treeIndex].currentChildIndex >= children.size())
              {
                  reset(data);

                  return State::Failure;
              }

              return State::Running;
      };

      /**
       * Executes a random sequence branch node. This node shuffles the order of its children on first execution
       * and then executes them in sequence.
       *
       * @param data  The node data vector from the tree instance running this node.
       *
       * @return success when all children succeed, failure when any one child fails, and running otherwise.
       */
      ExecuteType executeRandomSequence = [this](std::shared_ptr<NodeDataVector> data)
      {
        if ((*data)[treeIndex].shuffledChildren.empty())
        {
          for (int i = 0; i < children.size(); i++)
          {
            (*data)[treeIndex].shuffledChildren.push_back(i);
          }
          std::random_device rd;
          unsigned int seed;
          seed = (*data)[0].seed;
          //std::mt19937 g(seed);
          std::mt19937 g(rd());
          std::shuffle((*data)[treeIndex].shuffledChildren.begin(), (*data)[treeIndex].shuffledChildren.end(), g);
        }
        std::vector<int> shuffledChildren = (*data)[treeIndex].shuffledChildren;
        State childState = getChild(shuffledChildren[(*data)[treeIndex].currentChildIndex])->execute(data);
        if (childState == State::Failure)
        {
          reset(data);
          return State::Failure;
        }
        else if (childState == State::Success)
        {
          (*data)[treeIndex].currentChildIndex++;
        }
        if ((*data)[treeIndex].currentChildIndex >= this->children.size())
        {
          reset(data);
          return State::Success;
        }
        return State::Running;
      };

#pragma mark -

      /** The map of predefined node executes. */
      ActionMap predefined = {
          {"root", executeRoot},
          {"success", executeSuccess},
          {"failure", executeFailure},
          {"wait", executeWait},
          {"invert", executeInvert},
          {"always succeed", executeAlwaysSucceed},
          {"always fail", executeAlwaysFail},
          {"repeat", executeRepeat},
          {"selector", executeSelector},
          {"sequence", executeSequence},
          {"parallel", executeParallel},
          {"random selector", executeRandomSelector},
          {"random sequence", executeRandomSequence},
          {"until success", executeUntilSuccess},
          {"until failure", executeUntilFailure}};
    };
  };
}

#endif /* __CU_NODE_H__ */
