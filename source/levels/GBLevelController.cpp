#include "GBLevelController.h"
#include "../enemies/enemy_variants/GBBoss1Controller.h"
#include "../enemies/enemy_variants/GBBoss2Controller.h"
#include "../enemies/enemy_variants/GBBoss3Controller.h"
#include "../enemies/enemy_variants/GBMinion1AController.h"
#include "../enemies/enemy_variants/GBMinion1BController.h"
#include "../enemies/enemy_variants/GBMinion2AController.h"
#include "../enemies/enemy_variants/GBMinion2BController.h"
#include "../core/GBTypes.h"

using namespace cugl::graphics;
using namespace cugl::physics2;
using namespace cugl::scene2;

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
    } }, // Should return a Minion1BController
    { "boss_2", []() {
        return std::make_shared<Boss2Controller>();
    } }, // Should return a Boss2Controller
    { "minion_2A", []() {
        return std::make_shared<Minion2AController>();
    } }, // Should return a Minion2AController
    { "minion_2B", []() {
        return std::make_shared<Minion2BController>();
    } }, // Should return a Minion2BController
    { "boss_3", []() {
      return std::make_shared<Boss3Controller>();
    } }, // Should return a Boss3Controller
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
	enemy->getEnemy()->getSceneNode()->setVisible(false);
    return enemy;
}

void LevelController::addEnemy(const std::shared_ptr<EnemyController>& enemy_controller) {
    addObstacle(std::pair(enemy_controller->getEnemy(), enemy_controller->getEnemy()->getSceneNode()));
    enemy_controller->getEnemy()->getSceneNode()->setVisible(true);
    enemy_controller->inWorld = true;
	_spawnedEnemyCount++;
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
    float worldRight = _constantsJSON->get("scene")->getFloat("world_width") - wall_thickness;
    _enemiesJSON->get("world_info")->get("worldLeft")->set(wall_thickness);
    _enemiesJSON->get("world_info")->get("worldRight")->set(worldRight);

    _levels = parseLevels(_levelsJSON, _assets);
    if (_levels.empty()) {
        return false;
    }

    _numEnemiesActive = 0;

    // Setup player controller
    _playerController = std::make_shared<PlayerController>();
    _playerController->init(_assets, _constantsJSON);
    _playerInNextZone = true;
    _zoneUpdate = false;
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
        if (!_zoneUpdate) {
            updateRightZone(_currentWaveIndex + 1);
        }
        if (_playerInNextZone) {
            updateLeftZone(++_currentWaveIndex);
            _lastSpawnedTime = 0;
            _numEnemiesActive = 0;
            _currentEnemyIndex = 0;
        }
		//_spawnedEnemyCount = 0;
    }

    updateWave();
	//_totalEnemyCount = _enemyWaves[_currentWaveIndex].size();
}

void LevelController::updateWave() {
	if (_currentWaveIndex >= _enemyWaves.size() || _currentEnemyIndex >= _enemyWaves[_currentWaveIndex].size()) {
        return;
	}
    
    float spawnInterval = _currentLevel->getWaves()[_currentWaveIndex]->getSpawnIntervals()[_currentEnemyIndex];
    Vec2 spawnPosition = _currentLevel->getWaves()[_currentWaveIndex]->getSpawnPositions()[_currentEnemyIndex];
    if (_lastSpawnedTime >= spawnInterval && _numEnemiesActive < MAX_NUM_ENEMIES) {

		_enemyWaves[_currentWaveIndex][_currentEnemyIndex]->spawnAt(spawnPosition);
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
    
    _totalEnemyCount = 0;
    _spawnedEnemyCount = 0;
    
    for (auto wave : waves) {
        std::vector<std::string> enemiesString = wave->getEnemies();
		std::vector<std::shared_ptr<EnemyController>> enemyControllers;

        for (auto enemyType : enemiesString) {
            enemyControllers.push_back(createEnemy(enemyType));
            _totalEnemyCount++;
        }

        _enemyWaves.push_back(enemyControllers);
        _playerInNextZone = false;
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

	for (const Rect& platform : _currentLevel->getPlatforms()) {
		createPlatform(platform);
	}
}

// TODO: we should not use assetRef, load background & ground based on the level in the future
void LevelController::createStaticObstacles(const std::shared_ptr<LevelModel>& levelRef) {
    float scale = _constantsJSON->get("scene")->getFloat("scale");
    ObstacleNodePairs obstacle_pairs;
    std::shared_ptr<Texture> image;
    std::shared_ptr<scene2::PolygonNode> sprite;

    EarclipTriangulator triangulator;

#pragma mark : GROUND

    image = Texture::alloc(1, 1, Texture::PixelFormat::RGBA);
    std::shared_ptr<physics2::PolygonObstacle> groundObj;
    Poly2 ground(calculateGroundVertices());

    triangulator.set(ground.vertices);
    triangulator.calculate();
    ground.setIndices(triangulator.getTriangulation());
    triangulator.clear();

    groundObj = physics2::PolygonObstacle::allocWithAnchor(ground, Vec2::ANCHOR_CENTER);
    groundObj->setName("ground");

    setStaticPhysics(groundObj);
    ground *= scale;
    sprite = scene2::PolygonNode::allocWithTexture(image, ground);

    ObstacleNodePair ground_pair = std::make_pair(groundObj, sprite);
    obstacle_pairs.push_back(ground_pair);

#pragma mark : Walls
    // All walls and platforms share the same texture
    std::shared_ptr<JsonValue> wallsJ = _constantsJSON->get("walls");
    auto default_walls = wallsJ->get("level_bounds")->asFloatArray();
    createWall(default_walls[0], true);
    createWall(default_walls[1], false);
    
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

	_platforms.clear();

	_timeSpentInLevel = 0.0f;
}

void LevelController::preUpdate(float dt)
{
    std::shared_ptr<PlayerModel> player = _playerController->getPlayer();

    if (_enemyWaves.size() > 0
        && _currentWaveIndex < _enemyWaves.size()
        && _enemyWaves[_currentWaveIndex].size() > 0) {
        for (auto enemyCtrlr : _enemyWaves[_currentWaveIndex]) {
            if (canUpdate(enemyCtrlr)){
                std::shared_ptr<EnemyModel> enemodel = enemyCtrlr->getEnemy();
                enemodel->setTargetPos(player->getPosition());
                enemyCtrlr->preUpdate(dt);
            }
        }
    }

    for (auto platform : _platforms) {
        platform.second->setPosition(platform.first->getPosition() * _constantsJSON->get("scene")->getFloat("scale"));
		platform.second->setScale(2 * 0.0004006410 * Application::get()->getDisplayWidth());
		platform.second->setAnchor(Vec2::ANCHOR_CENTER);
    }

    _playerController->preUpdate(dt);

}

void LevelController::fixedUpdate(float timestep)
{
	if (!isLevelLost() && !isLevelWon()) {
		_timeSpentInLevel += timestep;
	}

	_playerController->fixedUpdate(timestep);
    updateLevel();

    if (_enemyWaves.size() > 0 
        && _currentWaveIndex < _enemyWaves.size() 
        && _enemyWaves[_currentWaveIndex].size() > 0) {
        for (auto enemyCtrlr : _enemyWaves[_currentWaveIndex]) {
            if (canUpdate(enemyCtrlr)) {
                auto damagingAction = enemyCtrlr->getEnemy()->getDamagingAction();
                auto enemyProjectile = enemyCtrlr->getEnemy()->getProjectile();

                if (damagingAction) {
                    // CULog("Action name: %s", damagingAction->getActionName().c_str());
                    float rot = damagingAction->getActionName() == "laser_diag" ? M_PI / 4 : 0;
                    createHitbox(enemyCtrlr->getEnemy(), damagingAction->getHitboxPos(), Size(damagingAction->getHitboxSize()), rot, damagingAction->getHitboxDamage(), damagingAction->getHitboxKnockBack(), 4 * (damagingAction->getHitboxEndFrame() - damagingAction->getHitboxStartFrame() + 1), damagingAction->getIsParriable());
                }

                if (enemyProjectile) {
					Vec2 offset = enemyCtrlr->getEnemy()->isFacingRight() ? enemyProjectile->getSpawnOffset() : Vec2(-enemyProjectile->getSpawnOffset().x, enemyProjectile->getSpawnOffset().y);
                    auto projectilePair = Projectile::createProjectileNodePair(_assets, _constantsJSON, enemyCtrlr->getEnemy()->getPosition() + offset, enemyProjectile, enemyCtrlr->getEnemy()->isFacingRight());
                    if (_worldRef->inBounds(projectilePair.first.get())) {
                        addObstacle(projectilePair);
                    }
                }

                enemyCtrlr->fixedUpdate(timestep);
            }

            else if (enemyCtrlr->getEnemy()->getHP() <= 0) {
                if (enemyCtrlr->getEnemy()->isRemoved()) {
                    continue;
                }

                enemyCtrlr->getEnemy()->die(_worldNode);
                enemyCtrlr->inWorld = false;
                _numEnemiesActive--;
            }
        }
    }
}

void LevelController::postUpdate(float dt)
{
    
    std::shared_ptr<PlayerModel> player = _playerController->getPlayer();
	_playerController->postUpdate(dt);

    if (_enemyWaves.size() > 0
        && _currentWaveIndex < _enemyWaves.size()
        && _enemyWaves[_currentWaveIndex].size() > 0) {
        for (auto enemyCtrlr : _enemyWaves[_currentWaveIndex]) {
            if (canUpdate(enemyCtrlr)) {

                enemyCtrlr->postUpdate(dt);
            }
        }
    }
    
    // change platform colors
    for (auto& pair : _platforms) {
        NodePtr node = pair.second;

        //float pulseFactor = cugl::EasingFactory::sineInOut(cugl::Application::get()->getEllapsedMicros() / 3000000.0f);

        //int startR = 115, startG = 42, startB = 86;   // Original pinkish color
        //int endR   = 40, endG = 40, endB = 40;      // Grayish tone

        //int newRed   = static_cast<int>(startR + (endR - startR) * pulseFactor);
        //int newGreen = static_cast<int>(startG + (endG - startG) * pulseFactor);
        //int newBlue  = static_cast<int>(startB + (endB - startB) * pulseFactor);

        //node->setColor(cugl::Color4(newRed, newGreen, newBlue, 255));
        node->setColor(Color4::BLACK);
    }
}

/**
 * Add a new projectile to the world and send it in the right direction.
 */
void LevelController::createHitbox(std::shared_ptr<EnemyModel> enemy, Vec2 pos, Size size, float rotation, int damage, float knockback, float duration, bool parriable) {
    std::shared_ptr<Texture> image = Texture::alloc(1, 1);

    // Change last parameter to test player-fired or regular projectile
    auto hitbox = Hitbox::alloc(enemy, pos, size, _scale, damage, knockback, duration, parriable);
    hitbox->setDebugColor(Color4::RED);
	hitbox->setAngle(enemy->isFacingRight() ? -rotation : rotation);

    std::shared_ptr<scene2::PolygonNode> sprite = scene2::PolygonNode::allocWithTexture(image);
    sprite->setAnchor(Vec2::ANCHOR_CENTER);

    if (_worldRef->inBounds(hitbox.get())) {
        addObstacle(ObstacleNodePair(hitbox, sprite));
    }
}

void LevelController::createPlatform(Rect rect)
{
    b2Filter enemyWallFilter = b2Filter();
    enemyWallFilter.categoryBits = 0x0001;
    enemyWallFilter.maskBits = 0x0002;
    std::shared_ptr<physics2::BoxObstacle> platform = physics2::BoxObstacle::alloc(rect.origin, rect.size);
    std::shared_ptr<physics2::BoxObstacle> leftWall = physics2::BoxObstacle::alloc(Vec2(rect.origin.x - rect.size.width / 2 - 0.05, rect.origin.y + rect.size.height / 2 + 0.1), Size(0.1, 0.2));
    std::shared_ptr<physics2::BoxObstacle> rightWall = physics2::BoxObstacle::alloc(Vec2(rect.origin.x + rect.size.width / 2 + 0.05, rect.origin.y + rect.size.height / 2 + 0.1), Size(0.1, 0.2));

    // You cannot add constant "".  Must stringify
    platform->setName(std::string(_constantsJSON->get("platforms")->getString("name")));
    leftWall->setName(std::string(_constantsJSON->get("walls")->getString("name")));
    rightWall->setName(std::string(_constantsJSON->get("walls")->getString("name")));

    // Set the physics attributes
    setStaticPhysics(platform);
    setStaticPhysics(leftWall);
    setStaticPhysics(rightWall);

	leftWall->setFilterData(enemyWallFilter);
	rightWall->setFilterData(enemyWallFilter);

    std::shared_ptr<PolygonNode> leftWallSprite = scene2::PolygonNode::alloc();
    std::shared_ptr<PolygonNode> rightWallSprite = scene2::PolygonNode::alloc();
    
    std::shared_ptr<PolygonNode> platformSprite = scene2::PolygonNode::allocWithTexture(Texture::getBlank());
    platformSprite->setContentSize(rect.size * 32);
    platformSprite->setVisible(true);
    platformSprite->setColor(Color4(115,42,86));
    
	//platformSprite->setAnchor(Vec2::ANCHOR_TOP_RIGHT);
	//platformSceneNode->addChild(platformSprite);
    
    addObstacle(std::make_pair(platform, platformSprite));
	addObstacle(std::make_pair(leftWall, leftWallSprite));
    addObstacle(std::make_pair(rightWall, rightWallSprite));

	_platforms.push_back(std::make_pair(platform, platformSprite));
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
            meleeAction->setHitboxStartFrame(action->getInt("hitboxStartFrame"));
            //CULog("PARSING: %i", action->getInt("hitboxStartFrame"));
            meleeAction->setHitboxEndFrame(action->getInt("hitboxEndFrame"));
            meleeAction->setHitboxDamage(action->getInt("hitboxDamage"));
            meleeAction->setHitboxKnockBack(action->getFloat("hitboxKnockBack"));
			meleeAction->setIsParriable(action->getBool("parriable"));

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
    level->setScale(0.0004006410 * Application::get()->getDisplayWidth());
    level->setBGN(json->getInt("width"));
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
    std::vector<Rect> platforms;

	for (std::shared_ptr<JsonValue> platform : json->get("platforms")->children()) {
        Rect rect = Rect();

        rect.origin.x = platform->get("origin")->get("x")->asFloat();
        rect.origin.y = platform->get("origin")->get("y")->asFloat();
        rect.size.width = platform->get("size")->get("width")->asFloat();
        rect.size.height = platform->get("size")->get("height")->asFloat();

		platforms.push_back(rect);
	}

	level->setPlatforms(platforms);


    // Parse Waves
    std::vector<std::shared_ptr<WaveModel>> waves;

    for (std::shared_ptr<JsonValue> wave : json->get("waves")->children()) {
        std::shared_ptr<WaveModel> waveModel = std::make_shared<WaveModel>();
		waveModel->setSpawnIntervals(wave->get("spawn_intervals")->asFloatArray());

		std::vector<Vec2> spawnPositions;
        std::shared_ptr<JsonValue> positions = wave->get("spawn_positions");
//		CULog(positions->toString().c_str());

        for (std::shared_ptr<JsonValue> position : wave->get("spawn_positions")->children()) {
			spawnPositions.push_back(Vec2(position->get("x")->asFloat(), position->get("y")->asFloat()));
        }
		waveModel->setSpawnPositions(spawnPositions);

		waveModel->setEnemies(wave->get("enemies")->asStringArray());

		waves.push_back(waveModel);
    }
    level->setWaves(waves);
    
    // Parse Walls
    std::vector<std::pair<float, float>> walls;

    for (std::shared_ptr<JsonValue> wall : json->get("walls")->children()) {
        auto wallLR = std::make_pair(wall->get("left")->asFloat(), wall->get("right")->asFloat());
        walls.push_back(wallLR);
    }

	level->setWalls(walls);
	return level;
}


std::shared_ptr<LevelController::WallZone> LevelController::createWall(float xPos, bool isLeft) {
    std::shared_ptr<JsonValue> wallsJ = _constantsJSON->get("walls");
    float scale = _constantsJSON->get("scene")->getFloat("scale");
    float height =  _constantsJSON->get("scene")->getFloat("world_height");
    float thickness = _constantsJSON->get("walls")->getFloat("thickness");
    
    std::vector<Vec2> wallVerts = {
        Vec2(xPos, height),
        Vec2(xPos, 0.0f),
        Vec2(xPos + thickness, 0.0f),
        Vec2(xPos + thickness, height)
    };

    Poly2 wall(wallVerts);

    EarclipTriangulator triangulator;
    triangulator.set(wall.vertices);
    triangulator.calculate();
    wall.setIndices(triangulator.getTriangulation());
    triangulator.clear();

    wall *= scale;

    auto wallObj = physics2::PolygonObstacle::allocWithAnchor(wall, Vec2::ANCHOR_BOTTOM_LEFT);
    wallObj->setName(std::string(wallsJ->getString("name")));
    setStaticPhysics(wallObj);
    std::shared_ptr<Texture> image;
    std::shared_ptr<scene2::PolygonNode> sprite;
    image = Texture::alloc(1, 1, Texture::PixelFormat::RGBA);
    sprite = scene2::PolygonNode::allocWithTexture(image, wall);
    
    _worldRef->addObstacle(wallObj);
    wallObj->setDebugScene(_debugNodeRef);
    sprite->setPosition(wallObj->getPosition() * _scale);
    _worldNode->addChild(sprite);
    float pixel_pos = xPos * 1024 * 2 * 0.0004006410 * Application::get()->getDisplayWidth();
    auto wallZone = std::make_shared<WallZone>(WallZone{wallObj, sprite, pixel_pos});
    if (isLeft) {
        _leftWallZone = wallZone;
    }
    else {
        _rightWallZone  = wallZone;
    }
    return wallZone;
}

void LevelController::removeWall(std::shared_ptr<WallZone> wallZone) {
    if (!wallZone) return;

    auto obj = wallZone->physicsWall;
    obj->getDebugNode()->removeFromParent();
    obj->deactivatePhysics(*_worldRef->getWorld());
    obj->markRemoved(true);

    wallZone->sceneWall->removeFromParent();
}

void LevelController::updateRightZone(int index) {
    removeWall(_rightWallZone);
    createWall(_currentLevel->getWalls()[index].second / (2 * 0.0004006410 * Application::get()->getDisplayWidth()), false);
    _zoneUpdate = true;
    _nextTrigger = _currentLevel->getWalls()[index].first*1024;
    _enemiesJSON->get("world_info")->get("worldRight")->set(_rightWallZone->xPosition);
}

void LevelController::updateLeftZone(int index) {
    removeWall(_leftWallZone);
    createWall(_currentLevel->getWalls()[index].first / (2 * 0.0004006410 * Application::get()->getDisplayWidth()), true);
    _zoneUpdate = false;
    _playerInNextZone = false;
    _enemiesJSON->get("world_info")->get("worldLeft")->set(_leftWallZone->xPosition);
}

std::vector<std::vector<Vec2>> LevelController::calculateWallVertices() {
    float worldWidth =  _constantsJSON->get("scene")->getFloat("world_width");
    float worldHeight =  _constantsJSON->get("scene")->getFloat("world_height");
    float wallThickness = _constantsJSON->get("walls")->getFloat("thickness");

    // Using Vec2 instead of float pairs
    std::vector<std::vector<Vec2>> wallVertices = {
        {
            Vec2(0.0f, 0.0f),
            Vec2(wallThickness, 0.0f),
            Vec2(wallThickness, worldHeight),
            Vec2(0.0f, worldHeight)
        },
        {
            Vec2(worldWidth - wallThickness, 0.0f),
            Vec2(worldWidth, 0.0f),
            Vec2(worldWidth, worldHeight),
            Vec2(worldWidth - wallThickness, worldHeight)
        }
    };

    return wallVertices;
}


std::vector<Vec2> LevelController::calculateGroundVertices() {
    float worldWidth = _constantsJSON->get("scene")->getFloat("world_width");
    float groundThickness = _constantsJSON->get("ground")->getFloat("thickness");

    std::vector<Vec2> groundVertices = {
        Vec2(0.0f, 0.0f),
        Vec2(worldWidth, 0.0f),
        Vec2(worldWidth, groundThickness),
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

float LevelController::getTimeSpentInLevel() const {
    return _timeSpentInLevel;
}

int LevelController::getTotalEnemyCount() const {
    return _totalEnemyCount;
}

int LevelController::getSpawnedEnemyCount() const {
    return _spawnedEnemyCount;
}

int LevelController::getCurrentWaveIndex() const {
    if (_currentWaveIndex >= _enemyWaves.size() - 1) {
        return -1;  // Final boss wave
    }
    return _currentWaveIndex;
}

std::shared_ptr<PlayerController> LevelController::getPlayerController() const {
    return _playerController;
}
