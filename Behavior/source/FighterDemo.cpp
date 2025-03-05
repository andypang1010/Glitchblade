#include "FighterDemo.h"

using namespace cugl;
using namespace cugl::graphics;
using namespace cugl::scene2;
using namespace std;

#define SCENE_HEIGHT 720
using namespace cugl::btree;

bool FighterDemo::init(const std::shared_ptr<cugl::AssetManager> &assets) {
    // Initialize the scene to a locked height
    if (assets == nullptr) {
        return false;
    } else if (!Scene2::initWithHint(Size(0,SCENE_HEIGHT))) {
        return false;
    }

    delay = 1.;
    delayCounter = 0.;
    step = 0;
    knockout = false;

  _assets = assets;

  // Load in scene textures
  _background = assets->get<Texture>("arenaBackground");
  _foreground = assets->get<Texture>("arenaForeground");
  _blueWins = assets->get<Texture>("blueWins");
  _redWins = assets->get<Texture>("redWins");

  // Create fighter objects
  fighter1 = std::make_shared<Fighter>();
  fighter1->setName("Fighter 1");
  fighter2 = std::make_shared<Fighter>();
  fighter2->setName("Fighter 2");
  fighter1->setTarget(fighter2);
  fighter2->setTarget(fighter1);
  fighter1->_idleTexture = assets->get<Texture>("redIdle");
  fighter2->_idleTexture = assets->get<Texture>("blueIdle");
  fighter1->_jabTexture = assets->get<Texture>("redJab");
  fighter2->_jabTexture = assets->get<Texture>("blueJab");
  fighter1->_punchTexture = assets->get<Texture>("redPunch");
  fighter2->_punchTexture = assets->get<Texture>("bluePunch");
  fighter1->_blockTexture = assets->get<Texture>("redBlock");
  fighter2->_blockTexture = assets->get<Texture>("blueBlock");
  fighter1->init(assets);
  fighter2->init(assets);

  cugl::Vec2 offset = cugl::Vec2(150, 0);
  fighter1->setPosition(-offset + getSize()/2);
  fighter2->setPosition(offset + getSize() / 2);

  // Get the fighter's behavior tree from the fighter json
  std::shared_ptr<JsonValue> jval = assets->get<JsonValue>("fighter");

  // Create a tree manager for that behavior tree
  _treeManager = TreeManager::alloc(jval);

  // Assign balckboard objects to local variables to put them in scope for variable capture
  std::shared_ptr<Fighter> f1 = fighter1;
  std::shared_ptr<Fighter> f2 = fighter2;

  // Define leaf execute functions using the required signature and capturing the blackboard object that
  // leaf node will control.
  Node::ExecuteType idle1 = [f1](std::shared_ptr<Node::NodeDataVector> data)
  {
    return f1->idle();
  };

  Node::ExecuteType block1 = [f1](std::shared_ptr<Node::NodeDataVector> data)
  {
    return f1->block();
  };

  Node::ExecuteType jab1 = [f1](std::shared_ptr<Node::NodeDataVector> data)
  {
    return f1->jab();
  };

  Node::ExecuteType punch1 = [f1](std::shared_ptr<Node::NodeDataVector> data)
  {
    return f1->punch();
  };

  Node::ExecuteType idle2 = [f2](std::shared_ptr<Node::NodeDataVector> data)
  {
    return f2->idle();
  };

  Node::ExecuteType block2 = [f2](std::shared_ptr<Node::NodeDataVector> data)
  {
    return f2->block();
  };

  Node::ExecuteType jab2 = [f2](std::shared_ptr<Node::NodeDataVector> data)
  {
    return f2->jab();
  };

  Node::ExecuteType punch2 = [f2](std::shared_ptr<Node::NodeDataVector> data)
  {
    return f2->punch();
  };

  // Create a map for each blackboard object, matching the keys to the strings used in the behavior tree json.
  Node::ActionMap actions1 = {
      {"idle", idle1},
      {"block", block1},
      {"jab", jab1},
      {"punch", punch1}};

  Node::ActionMap actions2 = {
      {"idle", idle2},
      {"block", block2},
      {"jab", jab2},
      {"punch", punch2}};

  // Create an instance for each fighter blackboard object and action map. Each action map and instance corresponds to one of the blackboard objects.
  _treeManager->createInstance("first", actions1);
  _treeManager->createInstance("second", actions2);

  reset();
  return true;
}

void FighterDemo::update(float timestep)
{
  if (knockout)
    return;
  delayCounter += timestep;
  if (delayCounter >= delay)
  {
    CULog("\n\n== STEP %d ==", step);
    step++;
    CULog("%s has %d health", fighter1->getName().c_str(), fighter1->getHealth());
    CULog("%s has %d health", fighter2->getName().c_str(), fighter2->getHealth());

    if (step % 2 == 0)
    {
      _treeManager->stepInstance("first");
    }
    else
    {
      _treeManager->stepInstance("second");
    }

    if (fighter1->getHealth() <= 0 || fighter2->getHealth() <= 0)
    {
      std::shared_ptr<Fighter> winner = (fighter1->getHealth() > fighter2->getHealth()) ? fighter1 : fighter2;
      knockout = true;
      CULog("%s WINS!!!", winner->getName().c_str());
    }

    delayCounter = 0;
  }
}

void FighterDemo::dispose()
{
  if (_active)
  {
    removeAllChildren();
    _active = false;
  }
}

void FighterDemo::reset()
{
  fighter1->setTexture(fighter1->_idleTexture);
  fighter2->setTexture(fighter2->_idleTexture);
}

void FighterDemo::render() {
  _batch->begin(getCamera()->getCombined());
  _batch->draw(_background, Rect(Vec2::ZERO, getSize()));
  fighter1->draw(_batch, getSize());
  fighter2->draw(_batch, getSize());
  fighter1->drawPow(_batch, getSize());
  fighter2->drawPow(_batch, getSize());
    _batch->draw(_foreground, Rect(Vec2::ZERO, getSize()));

  if (knockout) {
      if (fighter1->getHealth() > 0) {
          _batch->draw(_redWins, Rect(Vec2::ZERO, getSize()));
      }
      else {
          _batch->draw(_blueWins, Rect(Vec2::ZERO, getSize()));
      }
  }
    _batch->end();
}
