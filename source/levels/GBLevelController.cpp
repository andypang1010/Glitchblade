#include "GBLevelController.h"
#include "../enemies/enemy_variants/GBBoss1Controller.h"
#include "../enemies/enemy_variants/GBMinion1AController.h"
#include "../enemies/enemy_variants/GBMinion1BController.h"
#include "../core/GBTypes.h"

using namespace cugl::graphics;
using namespace cugl::physics2;
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

// EnemyController needs to become a base class that all other types of enemies derive from
std::unordered_map<std::string, std::function<std::shared_ptr<EnemyController>()>> enemyFactoryMap = {
    { "boss_1", []() {
        CULog("USING BOSS 1 CONTROLLER");
        return std::make_shared<Boss1Controller>();
    } }, // Should return a Boss1Controller
    { "minion_1A", []() {
        return std::make_shared<Minion1AController>();
    } }, // Should return a Minion1AController
    { "minion_1B", []() {
        return std::make_shared<Minion1BController>();
    } } // Should return a Minion1BController
    // Add more as desired here
};

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

std::shared_ptr<EnemyController> LevelController::createEnemy(std::string enemyType) {
    // This will call the correct constructor using make_shared from the enemyFactoryMap
    // Here, all enemy controller types will be treated as their parent class, EnemyController
    std::shared_ptr<EnemyController> enemy = enemyFactoryMap[enemyType]();

    std::vector<std::shared_ptr<ActionModel>> actions = LevelController::parseActions(_enemiesJSON, enemyType);
    enemy->init(_assets, _enemiesJSON, actions);
    return enemy;
}

void LevelController::addEnemy(const std::shared_ptr<EnemyController>& enemy_controller) {
    addObstacle(std::pair(enemy_controller->getEnemy(), enemy_controller->getEnemy()->getSceneNode()));
}

bool LevelController::init(const std::shared_ptr<AssetManager>& assetRef, const std::shared_ptr<JsonValue>& constantsRef, const std::shared_ptr<cugl::physics2::ObstacleWorld>& worldRef, const std::shared_ptr<cugl::scene2::SceneNode>& debugNodeRef, const std::shared_ptr<cugl::scene2::SceneNode>& worldNodeRef)
{
    // Store references to world and debugNode
    _worldNode = worldNodeRef;
    _worldRef = worldRef;
    _debugNodeRef = debugNodeRef;

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
    
    // set world_info constants in enemyJSON
    float scale = _constantsJSON->get("scene")->getFloat("scale");
    _enemiesJSON->get("world_info")->get("scale")->set(scale);
    float wall_thickness = _constantsJSON->get("walls")->getFloat("thickness");
    float worldRight = _constantsJSON->get("scene")->getFloat("default_scene_width") - wall_thickness;
    _enemiesJSON->get("world_info")->get("worldLeft")->set(wall_thickness);
    _enemiesJSON->get("world_info")->get("worldRight")->set(worldRight);

    _levels = parseLevels(_levelsJSON, _assets);
    if (_levels.empty()) {
        return false;
    }

    int _numEnemiesActive = 0;

    // Setup player controller
    _playerController = std::make_shared<PlayerController>();
    _playerController->init(_assets, _constantsJSON);

    return true;
}

void LevelController::updateLevel() {
	if (_enemyWaves.empty()) {
		spawnLevel();
		return;
	}

	if (_currentWaveIndex >= _enemyWaves.size()) {
		return;
	}

    if (waveComplete()) {
        _lastSpawnedTime = 0;
        _numEnemiesActive = 0;
		_currentEnemyIndex = 0;
        _currentWaveIndex++;
    }

    updateWave();
}

void LevelController::updateWave() {
	if (_currentWaveIndex >= _enemyWaves.size() || _currentEnemyIndex >= _enemyWaves[_currentWaveIndex].size()) {
        return;
	}

    float spawnInterval = _currentLevel->getWaves()[_currentWaveIndex]->getSpawnIntervals()[_currentEnemyIndex];

    if (_lastSpawnedTime >= spawnInterval && _numEnemiesActive < MAX_NUM_ENEMIES) {
		addEnemy(_enemyWaves[_currentWaveIndex][_currentEnemyIndex]);
		_numEnemiesActive++;
		_currentEnemyIndex++;
		_lastSpawnedTime = 0;
	}
	else {
		_lastSpawnedTime += 1.0f / 60.0f; // Assuming 60 FPS
    }
}

void LevelController::spawnLevel() {
    std::vector<std::shared_ptr<WaveModel>> waves = _currentLevel->getWaves();

    for (auto wave : waves) {
        std::vector<std::string> enemiesString = wave->getEnemies();
        std::vector<float> spawnIntervals = wave->getSpawnIntervals();
		std::vector<std::shared_ptr<EnemyController>> enemyControllers;

        for (auto enemyType : enemiesString) {
            enemyControllers.push_back(createEnemy(enemyType));
        }

        _enemyWaves.push_back(enemyControllers);
    }
}

bool LevelController::waveComplete() {
    bool isComplete = true;
	if (_currentWaveIndex >= _enemyWaves.size()) {
		return true;
	}

    for (auto enemy : _enemyWaves[_currentWaveIndex]) {
        isComplete &= enemy->getEnemy()->isRemoved();
    }

    return isComplete;
}

void LevelController::populateLevel(const std::shared_ptr<LevelModel>& level) {
    _currentLevel = level;
    createStaticObstacles(level);
    addObstacle(std::make_pair(getPlayerModel(), getPlayerNode()));
}

// TODO: we should not use assetRef, load background & ground based on the level in the future
void LevelController::createStaticObstacles(const std::shared_ptr<LevelModel>& levelRef) {
    float scale = _constantsJSON->get("scene")->getFloat("scale");
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
    image = Texture::alloc(1, 1, Texture::PixelFormat::RGBA);

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
    
    for (const auto& pair : obstacle_pairs) {
        // add obstacle and set node position
        addObstacle(pair);
    }

}

void LevelController::reset() {
    // Reset player controller
    if (_playerController != nullptr) {
        CULog("Resetting player controller");
        _playerController->reset();
    }

    // Reset wave attributes
    _enemyWaves.clear();
	_currentWaveIndex = 0;
    _currentEnemyIndex = 0;
	_lastSpawnedTime = 0;
    _numEnemiesActive = 0;
}

void LevelController::preUpdate(float dt)
{
    std::shared_ptr<PlayerModel> player = _playerController->getPlayer();

    if (_enemyWaves.size() > 0
        && _currentWaveIndex < _enemyWaves.size()
        && _enemyWaves[_currentWaveIndex].size() > 0) {
        for (auto enemyCtrlr : _enemyWaves[_currentWaveIndex]) {
            if (canUpdate(enemyCtrlr) && enemyCtrlr->getEnemy()->getHP() > 0){
                std::shared_ptr<EnemyModel> enemodel = enemyCtrlr->getEnemy();
                enemodel->setTargetPos(player->getPosition());
                enemyCtrlr->preUpdate(dt);
            }
        }
    }

	_playerController->preUpdate(dt);
}

void LevelController::fixedUpdate(float timestep)
{
	_playerController->fixedUpdate(timestep);
    updateLevel();

    if (_enemyWaves.size() > 0 
        && _currentWaveIndex < _enemyWaves.size() 
        && _enemyWaves[_currentWaveIndex].size() > 0) {
        for (auto enemyCtrlr : _enemyWaves[_currentWaveIndex]) {
            if (canUpdate(enemyCtrlr) && enemyCtrlr->getEnemy()->getHP() > 0 ) {
                enemyCtrlr->fixedUpdate(timestep);
            }
        }
    }
}

void LevelController::postUpdate(float dt)
{
    std::shared_ptr<PlayerModel> player = _playerController->getPlayer();
    if (player->isShooting() && player->hasProjectile()) {
        //ObstacleNodePair p = Projectile::createProjectile(_assets, _constantsJSON, player->getPosition(), player->isFacingRight() ? Vec2(1, 0) : Vec2(-1, 0), true, player->isFacingRight());
        //addObstacle(p);
        //player->setHasProjectile(false);
    }
	_playerController->postUpdate(dt);

    if (_enemyWaves.size() > 0
        && _currentWaveIndex < _enemyWaves.size()
        && _enemyWaves[_currentWaveIndex].size() > 0) {
        for (auto enemyCtrlr : _enemyWaves[_currentWaveIndex]) {
            if (!canUpdate(enemyCtrlr)) {
                continue;
            }

            auto damagingAction = enemyCtrlr->getEnemy()->getDamagingAction();
            auto projectileAction = enemyCtrlr->getEnemy()->getProjectileAction();

            if (damagingAction) {
                createHitbox(enemyCtrlr->getEnemy(), damagingAction->getHitboxPos(), Size(damagingAction->getHitboxSize()), damagingAction->getHitboxDamage(), damagingAction->getHitboxEndFrame() - damagingAction->getHitboxStartFrame() + 1);
            }

            if (projectileAction) {
                auto projectilePair = Projectile::createProjectileNodePair(_assets, _constantsJSON, enemyCtrlr->getEnemy()->getPosition(), projectileAction->getProjectiles()[0], enemyCtrlr->getEnemy()->isFacingRight());
                addObstacle(projectilePair);
            }

            if (enemyCtrlr->getEnemy()->getHP() <= 0) {
                if (enemyCtrlr->getEnemy()->isRemoved()) {
                    continue;
                }
                

                enemyCtrlr->getEnemy()->die(_worldNode);
                _worldNode->removeChild(enemyCtrlr->getEnemy()->getSceneNode());
                enemyCtrlr->getEnemy()->markRemoved(true);
                _numEnemiesActive--;
                //_enemyControllers.erase(std::remove(_enemyControllers.begin(), _enemyControllers.end(), enemyCtrlr), _enemyControllers.end());
            }

            enemyCtrlr->postUpdate(dt);

        }
    }
}

/**
 * Add a new projectile to the world and send it in the right direction.
 */
void LevelController::createHitbox(std::shared_ptr<EnemyModel> enemy, Vec2 pos, Size size, int damage, float duration) {
    std::shared_ptr<Texture> image = Texture::alloc(1, 1);

    // Change last parameter to test player-fired or regular projectile
    auto hitbox = Hitbox::alloc(enemy, pos, size, _scale, damage, duration);
    hitbox->setDebugColor(Color4::RED);

    std::shared_ptr<scene2::PolygonNode> sprite = scene2::PolygonNode::allocWithTexture(image);
    sprite->setAnchor(Vec2::ANCHOR_CENTER);

    if (_worldRef->inBounds(hitbox.get())) {
        addObstacle(ObstacleNodePair(hitbox, sprite));
    }
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

        std::string type = action->getString("type");
        std::string name = action->getString("name");

        if (type == "melee") {
            auto meleeAction = std::make_shared<MeleeActionModel>();
            meleeAction->setActionName(name);

            cugl::Vec2 hitboxPos(action->get("hitboxPos")->asFloatArray().front(), action->get("hitboxPos")->asFloatArray().back());
            cugl::Vec2 hitboxSize(action->get("hitboxSize")->asFloatArray().front(), action->get("hitboxSize")->asFloatArray().back());

            meleeAction->setHitboxPos(hitboxPos);
            meleeAction->setHitboxSize(hitboxSize);
            meleeAction->setHitboxStartFrame(action->getFloat("hitboxStartFrame"));
            meleeAction->setHitboxEndFrame(action->getFloat("hitboxEndFrame"));
            meleeAction->setHitboxDamage(action->getFloat("hitboxDamage"));

            

            actions.push_back(meleeAction);
        }
        else if (type == "ranged") {
            auto rangedAction = std::make_shared<RangedActionModel>();
            rangedAction->setActionName(name);

			std::vector<std::shared_ptr<Projectile>> projectiles;
			std::vector<Vec2> spawnPositions;
			std::vector<int> spawnFrames;

			for (std::shared_ptr<JsonValue> projectileJSON : action->get("projectiles")->children()) {
				std::shared_ptr<Projectile> projectile = std::make_shared<Projectile>();
                
                spawnFrames.push_back(projectileJSON->get("projectileSpawnFrame")->asInt());

				projectile->setIsPlayerFired(false);
                projectile->setDamage(projectileJSON->get("projectileDamage")->asInt());
                projectile->setLinearVelocity(
                    Vec2(
                        projectileJSON->get("projectileVelocity")->get("x")->asFloat(), 
                        projectileJSON->get("projectileVelocity")->get("y")->asFloat()
                    )
                );

                projectile->setSize(
                    Vec2(
                        projectileJSON->get("projectileSize")->get("width")->asFloat(),
                        projectileJSON->get("projectileSize")->get("height")->asFloat()
                    )
                );

                projectile->setSpawnOffset(
                    Vec2(
                        projectileJSON->get("projectileSpawnPosition")->get("x")->asFloat(),
                        projectileJSON->get("projectileSpawnPosition")->get("y")->asFloat()
                    )
                );
                projectile->setAnimOffset(
                    Vec2(
                        projectileJSON->get("projectile_animation_offset")->get("x")->asFloat(),
                        projectileJSON->get("projectile_animation_offset")->get("y")->asFloat()
                    )
                );

                projectile->setSceneNode(
                    SpriteNode::allocWithSheet(
                        Texture::allocWithFile(projectileJSON->getString("projectileAnimation")), 
                        projectileJSON->getInt("projectile_animation_row"),
                        projectileJSON->getInt("projectile_animation_col"),
                        projectileJSON->getInt("projectile_animation_size")
                    )
                );

				projectiles.push_back(projectile);
			}

			rangedAction->setProjectileSpawnPositions(spawnPositions);
			rangedAction->setProjectileSpawnFrames(spawnFrames);
			rangedAction->setProjectiles(projectiles);

            actions.push_back(rangedAction);
        }
        else {
            std::cerr << "Unknown action type: " << type << std::endl;
        }
    }

    return actions;
}

std::unordered_map<std::string, std::shared_ptr<LevelModel>> LevelController::parseLevels(const std::shared_ptr<JsonValue>& json, const std::shared_ptr<AssetManager>& assetRef)
{
    std::unordered_map<std::string, std::shared_ptr<LevelModel>> levels;

    if (!json || json->children().empty()) {
        CULogError("Invalid or empty JSON node!");
        return levels;
    }

    for (std::shared_ptr<JsonValue> level : json->get("levels")->children()) {
        std::shared_ptr<LevelModel> lModel = parseLevel(level, assetRef);
        levels[lModel->getLevelName()] = lModel;
    }

	return levels;
}

std::shared_ptr<LevelModel> LevelController::parseLevel(const std::shared_ptr<JsonValue>& json, const std::shared_ptr<AssetManager>& assetRef)
{
    std::shared_ptr<LevelModel> level = std::make_shared<LevelModel>();

    if (!json || json->children().empty()) {
        CULogError("Invalid or empty JSON node!");
        return level;
    }
	CULog("%s", json->toString().c_str());

	level->setLevelName(json->getString("name"));
    level->setScale(json->getFloat("scale"));
    auto bg = assetRef->get<graphics::Texture>(json->getString("background"));
    level->setBackground(bg);
    auto gr = Texture::allocWithFile(json->getString("ground"));
    level->setGround(gr);
    
    for (std::shared_ptr<JsonValue> layer : json->get("layers")->children()) {
        auto texture = assetRef->get<graphics::Texture>(layer->getString("file"));
        unsigned int speed = layer->getInt("speed");
        level->addLayer(texture, speed);
    }
    
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
            Vec2(defaultWidth*3.2 / 2, defaultHeight),
            Vec2(0.0f, defaultHeight),
            Vec2(0.0f, 0.0f),
            Vec2(wallThickness, 0.0f),
            Vec2(wallThickness, defaultHeight - wallThickness),
            Vec2(defaultWidth*3.7 / 2, defaultHeight - wallThickness)
        },
        {
            Vec2(defaultWidth*3.2, defaultHeight),
            Vec2(defaultWidth*3.2 / 2, defaultHeight),
            Vec2(defaultWidth*3.2 / 2, defaultHeight - wallThickness),
            Vec2(defaultWidth*3.2 - wallThickness, defaultHeight - wallThickness),
            Vec2(defaultWidth*3.2 - wallThickness, 0.0f),
            Vec2(defaultWidth*3.2, 0.0f)
        }
    };

    return wallVertices;
}


std::vector<Vec2> LevelController::calculateGroundVertices() {
    float defaultWidth = _constantsJSON->get("scene")->get("default_width")->asFloat(32.0f);
    float groundThickness = _constantsJSON->get("ground")->get("thickness")->asFloat(4.0f);

    std::vector<Vec2> groundVertices = {
        Vec2(0.0f, 0.0f),
        Vec2(defaultWidth*3.2, 0.0f),
        Vec2(defaultWidth*3.2, groundThickness),
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

void LevelController::addObstacle(ObstacleNodePair obstacle_pair) {
    std::shared_ptr<Obstacle> obj = obstacle_pair.first;
    std::shared_ptr<scene2::SceneNode> node = obstacle_pair.second;
    _worldRef->addObstacle(obj);
    obj->setDebugScene(nullptr);
    obj->setDebugScene(_debugNodeRef);
    node->setPosition(obj->getPosition() * _scale);
    _worldNode->addChild(node);
}

