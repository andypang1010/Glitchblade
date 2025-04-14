#include "GBLevelSelectScene.h"

using namespace cugl;
using namespace cugl::graphics;
using namespace cugl::physics2;

#define SCENE_WIDTH 1248
#define SCENE_HEIGHT 576
/** This is the aspect ratio for physics */
#define SCENE_ASPECT 9.0/19.5
/** Width of the game world in Box2d units */
#define DEFAULT_WIDTH   39.0f
/** Height of the game world in Box2d units */
#define DEFAULT_HEIGHT  18.0f

bool LevelSelectScene::init(const std::shared_ptr<cugl::AssetManager>& assets)
{
    return init(assets, Rect(0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT));
}

bool LevelSelectScene::init(const std::shared_ptr<AssetManager>& assets, const Rect& rect) {
    if (assets == nullptr) {
        return false;
    }

    _assets = assets;
    // prepare constants
    std::shared_ptr<JsonReader> constants_reader = JsonReader::allocWithAsset("json/constants.json");

    _constantsJSON = constants_reader->readJson();
    if (_constantsJSON == nullptr) {
        CULog("Failed to load constants.json");
        return false;
    }

    std::shared_ptr<JsonValue> sceneJ = _constantsJSON->get("scene");
    if (!Scene2::initWithHint(Size(sceneJ->getInt("width"), sceneJ->getInt("height")))) {
        return false;
    }

    _offset = Vec2((_size.width - sceneJ->getInt("width")) / 2.0f, (_size.height - sceneJ->getInt("height")) / 2.0f);

    Rect bounds = getBounds();
    std::shared_ptr<JsonValue> boundsJ = sceneJ->get("bounds");
    boundsJ->get("origin")->get("x")->set(bounds.origin.x);
    boundsJ->get("origin")->get("y")->set(bounds.origin.y);
    boundsJ->get("size")->get("width")->set(bounds.size.width);
    boundsJ->get("size")->get("height")->set(bounds.size.height);


    // IMPORTANT: SCALING MUST BE UNIFORM
    // This means that we cannot change the aspect ratio of the physics world
    // Shift to center if a bad fit
    _scale = _size.width == sceneJ->getInt("width") ? _size.width / rect.size.width : _size.height / rect.size.height;
    sceneJ->get("scale")->set(_scale);

    Application::get()->setClearColor(Color4f::BLACK);

    std::shared_ptr<Texture> bgimage = Texture::allocWithFile("textures/menu_background.png");
    _rootNode = scene2::PolygonNode::allocWithTexture(bgimage);
    _rootNode->setAnchor(Vec2::ANCHOR_BOTTOM_LEFT);
    _rootNode->setPosition(_offset);
    addChild(_rootNode);

    return true;
}

void LevelSelectScene::dispose()
{
    if (_active) {
        Scene2::dispose();
    }
}

int LevelSelectScene::getSwapSignal() {
    return _swapSceneSignal;
}

void LevelSelectScene::setSwapSignal(int signal) {
    _swapSceneSignal = signal;
}

void LevelSelectScene::preUpdate(float dt)
{
}

void LevelSelectScene::postUpdate(float remain)
{
}

void LevelSelectScene::fixedUpdate(float step)
{
}
