#include "GBLevelController.h"

using namespace cugl::graphics;
using namespace cugl::physics2;

#include "GBTypes.h"
using namespace cugl::scene2;

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
#define PLAYER_PROJECTILE_TEXTURE "player_projectile"

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

void LevelController::addEnemy(const std::shared_ptr<EnemyController>& cont) {
    _enemyControllers.push_back(cont);
}

bool LevelController::init(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& constantsRef)
{
    // read json
    _assets = assetRef;
    std::shared_ptr<JsonReader> enemies_reader = JsonReader::allocWithAsset("json/enemies.json");
    std::shared_ptr<JsonReader> constants_reader = JsonReader::allocWithAsset("json/constants.json");
    std::shared_ptr<JsonReader> levels_reader = JsonReader::allocWithAsset("json/levels.json");
    
    _enemiesJSON = enemies_reader->readJson();
    if (_enemiesJSON == nullptr) {
        CULog("Failed to load enemies.json");
        return false;
    }
    _constantsJSON = constantsRef;
    
    _levelsJSON = levels_reader->readJson();
    if (_levelsJSON == nullptr) {
        CULog("Failed to load constants.json");
        return false;
    }

	_levels = parseLevels(_levelsJSON);

    int _currentLevelIndex = 0;
    int _currentWaveIndex = 0;
    int _currentEnemyIndex = 0;
    int _numEnemiesActive = 0;
    float _lastSpawnedInterval = 0;

    // Setup player controller
    _playerController = std::make_shared<PlayerController>();
    _playerController->init(_assets, _constantsJSON);

    return true;
}

std::shared_ptr<cugl::scene2::PolygonNode> LevelController::makeWorldNode(std::string levelName) {
    std::shared_ptr<LevelModel> levelRef = getLevelByName(levelName);
    std::shared_ptr<Texture> bgimage = levelRef->getBackground();
    std::shared_ptr<cugl::scene2::PolygonNode> worldNode = scene2::PolygonNode::allocWithTexture(bgimage);
    return worldNode;
}

void LevelController::spawnWave(int waveNum) {
    // Now loop through the enemies in levelRef, store their actions, make controllers, & init them.
    std::vector<std::shared_ptr<WaveModel>> waves = _levelModel->getWaves();
    std::shared_ptr<WaveModel> wave = waves[waveNum];
    std::vector<std::string> enemiesString = wave->getEnemies();
    CULog("SPAWN WAVE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    for (auto s : enemiesString) {
        CULog(s.c_str());
    }

    //std::vector<std::shared_ptr<ActionModel>> actions = LevelController::parseActions(_enemiesJSON, "boss1");
    //std::shared_ptr<EnemyController> cont = std::make_shared<EnemyController>();
    //cont->init(_assets, _constantsJSON, actions);
    //addEnemy(cont); // Add the controller to the list of enemy controllers stored in the LevelController
}

ObstacleNodePairs LevelController::populateLevel(std::string levelName) {
    std::shared_ptr<LevelModel> levelRef = getLevelByName(levelName);
    // Finally, create and return the static obstacles for this level (need to pass & use the levelName instead of assets in the future if we want level-specific backgrounds, grounds, etc.)
    return createStaticObstacles(levelName, levelRef);
}

// TODO: we should not use assetRef, load background & ground based on the level in the future
ObstacleNodePairs LevelController::createStaticObstacles(std::string levelName, const std::shared_ptr<LevelModel>& levelRef) {
    float scale = _constantsJSON->get("scene")->getFloat("scale");
    CULog("in level controller scale is %f", scale);
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
    // Get the ground from the level itself
    image = levelRef->getGround();

    std::shared_ptr<physics2::PolygonObstacle> groundObj;
    Poly2 ground(calculateGroundVertices());

    EarclipTriangulator triangulator;
    triangulator.set(ground.vertices);
    triangulator.calculate();
    ground.setIndices(triangulator.getTriangulation());
    triangulator.clear();

    groundObj = physics2::PolygonObstacle::allocWithAnchor(ground, Vec2::ANCHOR_CENTER);
    // You cannot add constant "".  Must stringify
    groundObj->setName("GROUND OBJECT");

    // Set the physics attributes
    setStaticPhysics(groundObj);

    ground *= scale;
    sprite = scene2::PolygonNode::allocWithTexture(image, ground);
    ObstacleNodePair ground_pair = std::make_pair(groundObj, sprite);
    obstacle_pairs.push_back(ground_pair);

    return obstacle_pairs;
}

void LevelController::reset() {
    CULog("LC RESET CALLED!!!!!!");
    //_testEnemyController->reset();
    _playerController->reset();
}

void LevelController::preUpdate(float dt)
{
    //std::shared_ptr<EnemyModel> testEnemy = _testEnemyController->getEnemy();
    std::shared_ptr<PlayerModel> player = _playerController->getPlayer();
    // TODO: refactor using Box2d
    // Vec2 dist = testEnemy->getPosition() - player->getPosition();
    /*bool hit = false;
    if(player->iframe > 0) player->iframe--;
    if (testEnemy->isDamaging() && player->iframe <= 0) {
        if (testEnemy->_isSlamming) {
            if (dist.x > 0 && dist.x <= 6 && !testEnemy->isFacingRight() && std::abs(dist.y) <= 6) {
                hit = true;
            }
            else if (dist.x < 0 && dist.x >= -6 && testEnemy->isFacingRight() && std::abs(dist.y) <= 6) {
                hit = true;
            }
        }
        else if (testEnemy->_isStabbing) {
            if (dist.x > 0 && dist.x <= 6 && !testEnemy->isFacingRight() && std::abs(dist.y) <= 2) {
                hit = true;
            }
            else if (dist.x < 0 && dist.x >= -6 && testEnemy->isFacingRight() && std::abs(dist.y) <= 2) {
                hit = true;
            }
        }
    }

    if (hit) {
        player->setKnocked(true, player->getPosition().subtract(testEnemy->getPosition()).normalize());
        if (player->iframe <= 0 && !player->isParryActive() && !player->isGuardActive()) {
            player->damage(20);
        }
        else if (player->iframe <= 0 && player->isParryActive()) {
            testEnemy->setStun(120);
        }
        else if (player->iframe <= 0 && player->isGuardActive()) {
            player->damage(10);
        }
        player->iframe = 60;
    }
    testEnemy->setTargetPos(player->getPosition());*/


    if (player->isJumpBegin() && player->isGrounded()) {
        std::shared_ptr<JsonValue> fxJ = _constantsJSON->get("audio")->get("effects");
        std::shared_ptr<audio::Sound> source = _assets->get<audio::Sound>(fxJ->getString("jump"));
        audio::AudioEngine::get()->play(fxJ->getString("jump"),source,false,fxJ->getFloat("volume"));
    }

	//_testEnemyController->preUpdate(dt);
	_playerController->preUpdate(dt);
}

void LevelController::postUpdate(float dt)
{
	// _testEnemyController->postUpdate(dt);
	_playerController->postUpdate(dt);
}

void LevelController::fixedUpdate(float timestep)
{
	// _testEnemyController->fixedUpdate(timestep);
	_playerController->fixedUpdate(timestep);
}

/**
 * Parses the JSON file and returns a vector of parsed actions.
 */
std::vector<std::shared_ptr<ActionModel>> LevelController::parseActions(const std::shared_ptr<JsonValue>& json, const std::string enemyName) {
    std::vector<std::shared_ptr<ActionModel>> actions;

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
        std::shared_ptr<SpriteNode> animationSprite = SpriteNode::allocWithSheet(animationTexture, action->getInt("animation_row"), action->getInt("animation_col"), action->getInt("animation_size"));
        animationSprite->setFrame(0);

        if (type == "melee") {
            auto meleeAction = std::make_shared<MeleeActionModel>();
            meleeAction->setActionName(name);
            meleeAction->setActionAnimation(animationSprite);

            cugl::Vec2 hitboxPos(action->get("hitboxPos")->asFloatArray().front(), action->get("hitboxPos")->asFloatArray().back());
            cugl::Vec2 hitboxSize(action->get("hitboxSize")->asFloatArray().front(), action->get("hitboxSize")->asFloatArray().back());

            meleeAction->setHitboxPos(hitboxPos);
            meleeAction->setHitboxSize(hitboxSize);
            meleeAction->setHitboxStartTime(action->getFloat("hitboxStartFrame"));
            meleeAction->setHitboxEndTime(action->getFloat("hitboxEndFrame"));
            meleeAction->setHitboxDamage(action->getFloat("hitboxDamage"));

            actions.push_back(meleeAction);
        }
        else if (type == "ranged") {
            auto rangedAction = std::make_shared<RangedActionModel>();
            rangedAction->setActionName(name);
            rangedAction->setActionAnimation(animationSprite);

			std::vector<std::shared_ptr<Projectile>> projectiles;
			std::vector<Vec2> spawnPositions;
			std::vector<int> spawnFrames;

			for (std::shared_ptr<JsonValue> projectileJSON : action->get("projectiles")->children()) {
				std::shared_ptr<Projectile> projectile = std::make_shared<Projectile>();

                spawnPositions.push_back(Vec2(projectileJSON->get("projectileSpawnPosition")->get("x")->asFloat(), projectileJSON->get("projectileSpawnPosition")->get("y")->asFloat()));
                spawnFrames.push_back(projectileJSON->get("projectileSpawnFrame")->asInt());
				projectile->setLinearVelocity(Vec2(projectileJSON->get("projectileVelocity")->get("x")->asFloat(), projectileJSON->get("projectileVelocity")->get("y")->asFloat()));
                
                //projectile->setSpriteNode(
                //    SpriteNode::allocWithSheet(
                //        Texture::allocWithFile(projectileJSON->getString("projectileAnimation")), 
                //        action->getInt("animation_row"), 
                //        action->getInt("animation_col"), 
                //        action->getInt("animation_size")
                //    )
                //);
                // projectile->setDamage(projectileJSON->get("projectileDamage")->asInt());

				projectiles.push_back(projectile);
			}

			rangedAction->setProjectiles(projectiles);


            actions.push_back(rangedAction);
        }
        else {
            std::cerr << "Unknown action type: " << type << std::endl;
        }
    }

    return actions;
}

std::unordered_map<std::string, std::shared_ptr<LevelModel>> LevelController::parseLevels(const std::shared_ptr<JsonValue>& json)
{
    std::unordered_map<std::string, std::shared_ptr<LevelModel>> levels;

    if (!json || json->children().empty()) {
        CULogError("Invalid or empty JSON node!");
        return levels;
    }

    for (std::shared_ptr<JsonValue> level : json->get("levels")->children()) {
        std::shared_ptr<LevelModel> lModel = parseLevel(level);
        levels[lModel->getLevelName()] = lModel;
    }

	return levels;
}

std::shared_ptr<LevelModel> LevelController::parseLevel(const std::shared_ptr<JsonValue>& json)
{
    std::shared_ptr<LevelModel> level = std::make_shared<LevelModel>();

    if (!json || json->children().empty()) {
        CULogError("Invalid or empty JSON node!");
        return level;
    }

	CULog(json->toString().c_str());

	level->setLevelName(json-> getString("name"));

	CULog(level->getLevelName().c_str());
    level->setBackground(Texture::allocWithFile(json->getString("background")));
    level->setGround(Texture::allocWithFile(json->getString("ground")));
    
	// Parse platforms
    std::vector<std::shared_ptr<Rect>> platforms;

	for (std::shared_ptr<JsonValue> platform : json->get("platforms")->children()) {
		std::shared_ptr<Rect> rect = std::make_shared<Rect>();

		rect->origin.x = platform->get("origin")->get("x")->asFloat();
		rect->origin.y = platform->get("origin")->get("y")->asFloat();
		rect->size.width = platform->get("size")->get("width")->asFloat();
		rect->size.height = platform->get("size")->get("height")->asFloat();

		platforms.push_back(rect);
	}

	level->setPlatforms(platforms);


    // Parse Waves
    std::vector<std::shared_ptr<WaveModel>> waves;

    for (std::shared_ptr<JsonValue> wave : json->get("waves")->children()) {
        std::shared_ptr<WaveModel> waveModel = std::make_shared<WaveModel>();
		waveModel->setSpawnIntervals(wave->get("spawn_intervals")->asFloatArray());
		waveModel->setEnemies(wave->get("enemies")->asStringArray());

		waves.push_back(waveModel);
    }

	level->setWaves(waves);
	return level;
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
