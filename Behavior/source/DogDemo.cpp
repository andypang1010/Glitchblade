#include "DogDemo.h"

using namespace cugl;
using namespace cugl::graphics;
using namespace cugl::scene2;
using namespace std;

#define SCENE_HEIGHT 720
using namespace cugl::btree;

bool DogDemo::init(const std::shared_ptr<cugl::AssetManager> &assets) {
    // Initialize the scene to a locked height
    if (assets == nullptr) {
        return false;
    } else if (!Scene2::initWithHint(Size(0,SCENE_HEIGHT))) {
        return false;
    }
    Size dimen = getSize();
    delay = 1.;
    delayCounter = 0.;
    step = 0;

  _assets = assets;
  _pineController._texture = assets->get<Texture>("pine");
  _pineController.init(4, getSize());
  _background = assets->get<Texture>("background");
  _dog = std::make_shared<Dog>();
  _dog->_barkTexture = assets->get<Texture>("bark");
  _dog->_idleTexture = assets->get<Texture>("idle");
  _dog->_runTexture = assets->get<Texture>("walk");
  _dog->_sleepTexture = assets->get<Texture>("sleep");
  _dog->_markTexture = assets->get<Texture>("mark");

  _dog2 = std::make_shared<Dog>();
  _dog2->_barkTexture = assets->get<Texture>("bark");
  _dog2->_idleTexture = assets->get<Texture>("idle");
  _dog2->_runTexture = assets->get<Texture>("walk");
  _dog2->_sleepTexture = assets->get<Texture>("sleep");
  _dog2->_markTexture = assets->get<Texture>("mark");



  // Setup Btree
  Node::LogicFuncWrapper logicFunctionWrapper = [](std::shared_ptr<Node> node)
  {
    Node::ExecuteType logicFunction = [node](std::shared_ptr<Node::NodeDataVector> data)
    {
      CULog("run second");
      return node->children[1]->execute(data);
    };

    return logicFunction;
  };

  std::unordered_map<std::string, Node::LogicFuncWrapper> logicMap;
  logicMap["run second"] = logicFunctionWrapper;

  std::shared_ptr<JsonValue> jval = assets->get<JsonValue>("dog");
  _treeManager = TreeManager::alloc(jval, logicMap);

  std::shared_ptr<Dog> d = _dog;
  Node::ExecuteType barkFunc = [d](std::shared_ptr<Node::NodeDataVector> data)
  {
    return d->bark();
  };

  Node::ExecuteType sleepFunc = [d](std::shared_ptr<Node::NodeDataVector> data)
  {
    return d->sleep();
  };

  Node::ExecuteType runFunc = [d](std::shared_ptr<Node::NodeDataVector> data)
  {
    return d->run();
  };

  Node::ExecuteType playFunc = [d](std::shared_ptr<Node::NodeDataVector> data)
  {
    return d->play();
  };

  Node::ExecuteType markFunc = [d](std::shared_ptr<Node::NodeDataVector> data)
  {
    return d->mark();
  };

  std::shared_ptr<Dog> d2 = _dog2;
  Node::ExecuteType barkFunc2 = [d2](std::shared_ptr<Node::NodeDataVector> data)
  {
    return d2->bark();
  };

  Node::ExecuteType sleepFunc2 = [d2](std::shared_ptr<Node::NodeDataVector> data)
  {
    return d2->sleep();
  };

  Node::ExecuteType runFunc2 = [d2](std::shared_ptr<Node::NodeDataVector> data)
  {
    return d2->run();
  };

  Node::ExecuteType playFunc2 = [d2](std::shared_ptr<Node::NodeDataVector> data)
  {
    return d2->play();
  };

  Node::ExecuteType markFunc2 = [d2](std::shared_ptr<Node::NodeDataVector> data)
  {
    return d2->mark();
  };

  Node::ActionMap dogMap = {
      {"bark", barkFunc},
      {"sleep", sleepFunc},
      {"play", playFunc},
      {"mark", markFunc},
      {"run", runFunc},
  };
  Node::ActionMap dogMap2 = {
      {"bark", barkFunc2},
      {"sleep", sleepFunc2},
      {"play", playFunc2},
      {"mark", markFunc2},
      {"run", runFunc2},
  };
  _dog->seed = 100;
  _dog2->seed = 200;
  //_treeManager->createInstanceWithSeed("first", dogMap, 100);
  //_treeManager->createInstanceWithSeed("second", dogMap2, 200);
  _treeManager->createInstance("first", dogMap);
  _treeManager->createInstance("second", dogMap2);

  reset();

  return true;
}

void DogDemo::update(float timestep)
{
  delayCounter += timestep;
  if (delayCounter >= delay)
  {
    CULog("\n\n== STEP %d ==", step);
    step++;
    _treeManager->stepInstance("first");
    _treeManager->stepInstance("second");
    delayCounter = 0;
  }
}

void DogDemo::dispose()
{
  if (_active)
  {
    removeAllChildren();
    _active = false;
  }
}

void DogDemo::reset() {
  _dog->setPosition(Vec2(40, 0) + getSize() / 2);
  _dog->treePositions = _pineController.getPositions();

  _dog->setTexture(_dog->_idleTexture);

  _dog2->setPosition(Vec2(-40, 0) + getSize() / 2);
  _dog2->treePositions = _pineController.getPositions();

  _dog2->setTexture(_dog->_idleTexture);
}

void DogDemo::render() {
    _batch->begin(getCamera()->getCombined());
    _batch->draw(_background, Rect(Vec2::ZERO, getSize()));
    _pineController.draw(_batch);
    _dog->draw(_batch, getSize());
    _dog2->draw(_batch, getSize());

    _batch->end();
}
