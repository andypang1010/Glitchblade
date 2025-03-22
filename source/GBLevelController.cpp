#include "GBLevelController.h"

using namespace cugl::graphics;
using namespace cugl::physics2;

#include "GBTypes.h"

#pragma mark -
#pragma mark Constants moved from gamescene to here - these maybe should be parsed from JSON or in separate file

#pragma mark -
#pragma mark Level Geography
/** This is adjusted by screen aspect ratio to get the height */
#define GAME_SCENE_WIDTH 1024
#define GAME_SCENE_HEIGHT 576

/** This is the aspect ratio for physics */
#define GAME_SCENE_ASPECT 9.0/16.0

/** Width of the game world in Box2d units */
#define GAME_DEFAULT_WIDTH   32.0f
/** Height of the game world in Box2d units */
#define GAME_DEFAULT_HEIGHT  18.0f


/** Bullet Spawn Points */


/** Directions */
Vec2 LEFT = { -1.0f, 0.0f };
Vec2 RIGHT = { 1.0f, 0.0f };
Vec2 UP = { 0.0f, 1.0f };
Vec2 DOWN = { 0.0f, -1.0f };

#pragma mark -
#pragma mark Physics Constants
/** The density for most physics objects */
#define BASIC_DENSITY   0.0f
/** The density for a projectile */
#define HEAVY_DENSITY   10.0f
/** Friction of most platforms */
#define BASIC_FRICTION  0.4f
/** The restitution for all physics objects */
#define BASIC_RESTITUTION   0.1f
/** Offset for pojectile when firing */
#define PROJECTILE_OFFSET   0.5f
/** The speed of the projectile after firing */
#define PROJECTILE_SPEED   30.0f


#pragma mark -
#pragma mark Testing Constants
/** The radius for enemy to initial attack */
#define ENEMY_ATTACK_RADIUS     6.0f

#pragma mark -
#pragma mark Asset Constants

///////////////// TEXTURES //////////////////////////////////
/** The key for the ground texture in the asset manager */
#define GROUND_TEXTURE  "ground"
/** The key for the regular projectile texture in the asset manager */
#define PROJECTILE_TEXTURE  "projectile"
/** The key for the player projectile texture in the asset manager */
#define PLAYER_PROJECTILE_TEXTURE "player-projectile"

///////////////// NAMES /////////////////////////////////////
#define PROJECTILE_NAME "projectile"
/** The name of a platform (for object identification) */
#define GROUND_NAME   "ground"

/** Opacity of the physics outlines */
#define DEBUG_OPACITY   192

#define WORLD_DEBUG_COLOR    Color4::WHITE


void LevelController::checkWinCondition()
{
}

void LevelController::checkLoseCondition()
{
}

LevelController::LevelController()
{
}

LevelController::~LevelController()
{
}

ObstacleNodePairs LevelController::populateLevel(std::string levelName) {
    // Setup enemy controller: one controller per enemy
    std::vector<std::shared_ptr<ActionModel>> actions = LevelController::parseActions(_enemiesJSON, "boss1");

    _testEnemyController = std::make_shared<EnemyController>();
    _testEnemyController->init(_assets, _scale, actions);

    // Setup player controller
    _playerController = std::make_shared<PlayerController>();
    _playerController->init(_assets, *_bounds, _scale);

    CULog("LevelController::init");

    return createStaticObstacles(_assets, _scale);
}

bool LevelController::init(const std::shared_ptr<AssetManager>& assetRef, cugl::Rect bounds, float scale)
{
    _bounds = &bounds;
    _assets = assetRef;
    _scale = scale;

    // read json
    std::shared_ptr<JsonReader> enemies_reader = JsonReader::allocWithAsset("json/enemies.json");
    std::shared_ptr<JsonReader> constants_reader = JsonReader::allocWithAsset("json/constants.json");
    std::shared_ptr<JsonReader> levels_reader = JsonReader::allocWithAsset("json/levels.json");
    
    _enemiesJSON = enemies_reader->readJson();
    if (_enemiesJSON == nullptr) {
        CULog("Failed to load enemies.json");
        return false;
    }
    _constantsJSON = constants_reader->readJson();
    if (_constantsJSON == nullptr) {
        CULog("Failed to load constants.json");
        return false;
    }
    _levelsJSON = levels_reader->readJson();
    if (_levelsJSON == nullptr) {
        CULog("Failed to load constants.json");
        return false;
    }

    // old populate location

    return true;
}

ObstacleNodePairs LevelController::createStaticObstacles(const std::shared_ptr<AssetManager>& assetRef, float scale){
    ObstacleNodePairs obstacle_pairs;
    std::shared_ptr<Texture> image;
    std::shared_ptr<scene2::PolygonNode> sprite;
    std::shared_ptr<scene2::SceneNode> node = scene2::SceneNode::alloc();

#pragma mark : Walls
    // All walls and platforms share the same texture
    std::shared_ptr<JsonValue> wallsJ = _constantsJSON->get("walls");
    image = Texture::alloc(1, 1, Texture::PixelFormat::RGBA);
    std::vector<std::vector<Vec2>> walls = calculateWallVertices();
    int w_count = wallsJ->getInt("wall_count");
    for (int ii = 0; ii < w_count ; ii++) {
        std::shared_ptr<physics2::PolygonObstacle> wallObj;
        Poly2 wall(walls[ii]);
        // Call this on a polygon to get a solid shape
        EarclipTriangulator triangulator;
        triangulator.set(wall.vertices);
        triangulator.calculate();
        wall.setIndices(triangulator.getTriangulation());
        triangulator.clear();

        wallObj = physics2::PolygonObstacle::allocWithAnchor(wall, Vec2::ANCHOR_CENTER);
        // You cannot add constant "".  Must stringify
        wallObj->setName(std::string(wallsJ->getString("name")));
        // Set the physics attributes
        setStaticPhysics(wallObj);
        wall *= scale;
        sprite = scene2::PolygonNode::allocWithTexture(image, wall);
        
        ObstacleNodePair wall_pair = std::make_pair(wallObj, sprite);
        obstacle_pairs.push_back(wall_pair); // Add wall obj
    }

#pragma mark : GROUND
    std::shared_ptr<JsonValue> groundJ = _constantsJSON->get("ground");
    image = assetRef->get<Texture>(groundJ->getString("texture"));

    std::shared_ptr<physics2::PolygonObstacle> groundObj;
    Poly2 ground(calculateGroundVertices());

    EarclipTriangulator triangulator;
    triangulator.set(ground.vertices);
    triangulator.calculate();
    ground.setIndices(triangulator.getTriangulation());
    triangulator.clear();

    groundObj = physics2::PolygonObstacle::allocWithAnchor(ground, Vec2::ANCHOR_CENTER);
    // You cannot add constant "".  Must stringify
    groundObj->setName(std::string(groundJ->getString("name")));

    // Set the physics attributes
    setStaticPhysics(groundObj);

    ground *= scale;
    sprite = scene2::PolygonNode::allocWithTexture(image, ground);
    ObstacleNodePair ground_pair = std::make_pair(groundObj, sprite);
    obstacle_pairs.push_back(ground_pair);
    
    // return all static obstacle-node pairs
    return obstacle_pairs;
}

void LevelController::reset() {
    CULog("LC RESET CALLED!!!!!!");
    _testEnemyController->reset();
    _playerController->reset();
}

void LevelController::preUpdate(float dt)
{
	_testEnemyController->preUpdate(dt);
	_playerController->preUpdate(dt);
}

void LevelController::postUpdate(float dt)
{
	_testEnemyController->postUpdate(dt);
	_playerController->postUpdate(dt);
}

void LevelController::fixedUpdate(float timestep)
{
	_testEnemyController->fixedUpdate(timestep);
	_playerController->fixedUpdate(timestep);
}

/** Parses the JSON file and returns a vector of parsed levels. */
std::vector<std::shared_ptr<LevelModel>> parseLevels(const std::shared_ptr<JsonValue>& json) {
    std::vector<std::shared_ptr<LevelModel>> levels;

    if (!json || json->children().empty()) {
        CULogError("Invalid or empty JSON node!");
        return levels;
    }

}

/** Parses the JSON file and returns a parsed level. */
std::shared_ptr<LevelModel> parseLevel(const std::shared_ptr<JsonValue>& json) {
    std::shared_ptr<LevelModel> level;

	if (!json || json->children().empty()) {
		CULogError("Invalid or empty JSON node!");
		return level;
	}

}

/**
 * Parses the JSON file and returns a vector of parsed actions.
 */
std::vector<std::shared_ptr<ActionModel>> LevelController::parseActions(const std::shared_ptr<JsonValue>& json, const std::string enemyName) {
    std::vector<std::shared_ptr<ActionModel>> actions;

    //CULog((json->toString()).c_str());

    if (!json || json->children().empty()) {
        CULogError("Invalid or empty JSON node!");
        return actions;
    }

    if (json->get(enemyName) == nullptr) {
        CULogError("Enemy not found!");
        return actions;
    }

    if (!json->get(enemyName)->has("actions")) {
        CULogError("Invalid JSON format: Missing actions!");
        return actions;
    }

    std::vector<std::shared_ptr<JsonValue>> actionArray = json->get(enemyName)->get("actions")->children();
    for (std::shared_ptr<JsonValue> action : actionArray) {

        // CULog(action->toString().c_str()); // Causes android studio to have a seizure for some reason

        std::string type = action->getString("type");
        std::string name = action->getString("name");

        std::string animationPath = action->getString("animation");
        std::shared_ptr<Texture> animationTexture = Texture::allocWithFile(animationPath);
        std::shared_ptr<SpriteSheet> animationSprite = SpriteSheet::alloc(animationTexture, action->getInt("animation_row"), action->getInt("animation_col"), 40);
        animationSprite->setFrame(0);

        if (type == "melee") {
            auto meleeAction = std::make_shared<MeleeActionModel>();
            meleeAction->setActionName(name);
            meleeAction->setActionAnimation(animationSprite);

            cugl::Vec2 hitboxPos(action->get("hitboxPos")->asFloatArray().front(), action->get("hitboxPos")->asFloatArray().back());
            cugl::Vec2 hitboxSize(action->get("hitboxSize")->asFloatArray().front(), action->get("hitboxSize")->asFloatArray().back());

            meleeAction->setHitboxPos(hitboxPos);
            meleeAction->setHitboxSize(hitboxSize);
            meleeAction->setHitboxStartTime(action->getFloat("hitboxStartTime"));
            meleeAction->setHitboxEndTime(action->getFloat("hitboxEndTime"));
            meleeAction->setHitboxDamage(action->getFloat("hitboxDamage"));

            actions.push_back(meleeAction);
        }
        else if (type == "ranged") {
            auto rangedAction = std::make_shared<RangedActionModel>();
            rangedAction->setActionName(name);
            rangedAction->setActionAnimation(animationSprite);

            std::shared_ptr<Texture> projectileTexture = Texture::allocWithFile(action->getString("projectileSprite"));
            rangedAction->setProjectileTexture(projectileTexture);
            rangedAction->setProjectileSpeed(action->getFloat("projectileSpeed"));

            cugl::Vec2 projectileDirection(action->get("projectileDirection")->asFloatArray().front(), action->get("projectileDirection")->asFloatArray().back());
            rangedAction->setProjectileDirection(projectileDirection);
            rangedAction->setProjectileDamage(action->getFloat("projectileDamage"));

            actions.push_back(rangedAction);
        }
        else if (type == "movement") {
            auto movementAction = std::make_shared<MovementActionModel>();
            movementAction->setActionName(name);
            movementAction->setActionAnimation(animationSprite);

            movementAction->setMoveToPlayer(action->getBool("moveToPlayer"));

            cugl::Vec2 moveDirection(action->get("moveDirection")->asFloatArray().front(), action->get("moveDirection")->asFloatArray().back());
            movementAction->setMoveDirection(moveDirection);
            movementAction->setMoveDistance(action->getFloat("moveDistance"));

            actions.push_back(movementAction);
        }
        else {
            std::cerr << "Unknown action type: " << type << std::endl;
        }
    }

    return actions;
}



std::vector<std::vector<Vec2>> LevelController::calculateWallVertices() {
    float defaultWidth =  _constantsJSON->get("scene")->get("default_width")->asFloat(32.0f);
    float defaultHeight =  _constantsJSON->get("scene")->get("default_height")->asFloat(18.0f);
    float wallThickness = _constantsJSON->get("walls")->get("thickness")->asFloat(1.0f);

    // Using Vec2 instead of float pairs
    std::vector<std::vector<Vec2>> wallVertices = {
        {
            Vec2(defaultWidth / 2, defaultHeight),
            Vec2(0.0f, defaultHeight),
            Vec2(0.0f, 0.0f),
            Vec2(wallThickness, 0.0f),
            Vec2(wallThickness, defaultHeight - wallThickness),
            Vec2(defaultWidth / 2, defaultHeight - wallThickness)
        },
        {
            Vec2(defaultWidth, defaultHeight),
            Vec2(defaultWidth / 2, defaultHeight),
            Vec2(defaultWidth / 2, defaultHeight - wallThickness),
            Vec2(defaultWidth - wallThickness, defaultHeight - wallThickness),
            Vec2(defaultWidth - wallThickness, 0.0f),
            Vec2(defaultWidth, 0.0f)
        }
    };

    return wallVertices;
}


std::vector<Vec2> LevelController::calculateGroundVertices() {
    float defaultWidth = _constantsJSON->get("scene")->get("default_width")->asFloat(32.0f);
    float groundThickness = _constantsJSON->get("ground")->get("thickness")->asFloat(4.0f);

    std::vector<Vec2> groundVertices = {
        Vec2(0.0f, 0.0f),
        Vec2(defaultWidth, 0.0f),
        Vec2(defaultWidth, groundThickness),
        Vec2(0.0f, groundThickness)
    };

    return groundVertices;
}

Vec2 LevelController::calculateLeftBulletPosition() {
    return { _constantsJSON->get("walls")->get("thickness")->asFloat(1.0f) + 1.5f, 9.0f };
}

Vec2 LevelController::calculateRightBulletPosition() {
    return { _constantsJSON->get("scene")->get("thickness")->asFloat(1.0f) - 1.5f, 13.0f };
}

void LevelController::setStaticPhysics(const std::shared_ptr<physics2::Obstacle>& obj){
    
    std::shared_ptr<JsonValue> physicsJ = _constantsJSON->get("physics");
    // Set the physics attributes
    obj->setBodyType(b2_staticBody);
    obj->setDensity(physicsJ->getFloat("basic_density"));
    obj->setFriction(physicsJ->getFloat("basic_friction"));
    obj->setRestitution(physicsJ->getFloat("basic_restitution"));
    obj->setDebugColor(physicsJ->get("debug")->getString("color"));
}
