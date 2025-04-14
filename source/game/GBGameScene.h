//
//  GBGameScene.h
//  PlatformDemo
//
//  This is the most important class in this demo.  This class manages the gameplay
//  for this demo.  It also handles collision detection. There is not much to do for
//  collisions; our WorldController class takes care of all of that for us.  This
//  controller mainly transforms input into gameplay.
//
//  You will notice that we do not use a Scene asset this time.  While we could
//  have done this, we wanted to highlight the issues of connecting physics
//  objects to scene graph objects.  Hence we include all of the API calls.
//
//  WARNING: There are a lot of shortcuts in this design that will do not adapt
//  well to data driven design. This demo has a lot of simplifications to make
//  it a bit easier to see how everything fits together. However, the model
//  classes and how they are initialized will need to be changed if you add
//  dynamic level loading.
//
//  This file is based on the CS 3152 PhysicsDemo Lab by Don Holden (2007).
//  This file has been refactored to support the physics changes in CUGL 2.5.
//
//  Author: Walker White and Anthony Perello
//  Version:  2/9/24
//
#pragma once

#include <cugl/cugl.h>
#include <box2d/b2_world_callbacks.h>
#include <box2d/b2_fixture.h>
#include <unordered_set>
#include <vector>
#include "../core/GBInput.h"
#include "objects/GBProjectile.h"
// keep includes for models to access scene nodes when adding as children in init
#include "../player/GBPlayerModel.h"
#include "../enemies/GBEnemyModel.h"
#include "../levels/GBLevelController.h"
#include "GBSceneInterface.h"
#include "ui/GBIngameUI.h"
#include "ui/GBPauseMenu.h"

using namespace cugl;

/**
 * This class is the primary gameplay constroller for the demo.
 *
 * A world has its own objects, assets, and input controller.  Thus this is
 * really a mini-GameEngine in its own right.  As in 3152, we separate it out
 * so that we can have a separate mode for the loading screen.
 */
class GameScene : public scene2::Scene2, public GBSceneInterface {
protected:
    /** The asset manager for this game mode. */
    std::shared_ptr<AssetManager> _assets;
    std::shared_ptr<JsonValue> _constantsJSON;
    // CONTROLLERS
    /** LevelController for managing the level progress of the current game */
    std::shared_ptr<LevelController> _levelController;
    // delete this input controller
    std::shared_ptr<PlatformInput> _input;
    
    // VIEW
    /** Reference to the physics root of the scene graph */
    std::shared_ptr<scene2::SceneNode> _worldnode;
    /** Reference to the debug root of the scene graph */
    std::shared_ptr<scene2::SceneNode> _debugnode;
    /** Reference to the win message label */
    std::shared_ptr<scene2::Label> _winnode;
    /** Reference to the lose message label */
    std::shared_ptr<scene2::Label> _losenode;
    /** Reference to the enemy HP label */
    std::shared_ptr<scene2::Label> _enemyHPNode;
    /** Reference to the player HP label */
    std::shared_ptr<scene2::Label> _playerHPNode;
    /** Reference to the enemy stun label */
    std::shared_ptr<scene2::Label> _enemyStunNode;
    
    // UI
    /** Ingame UI */
    std::shared_ptr<GBIngameUI> _ui;

    std::shared_ptr<GBPauseMenu> _pauseMenu;
    bool _isPaused = false;
    
    /** The Box2D world */
    std::shared_ptr<physics2::ObstacleWorld> _world;
    /** The scale between the physics world and the screen (MUST BE UNIFORM) */
    float _scale;
    Vec2 _offset;
    // Physics objects for the game
    /** Reference to the player avatar */
    std::shared_ptr<PlayerModel>			  _player;

    /** Whether we have completed this "game" */
    bool _complete;
    /** Whether or not debug mode is active */
    bool _debug;
    /** Whether we have failed at this world (and need a reset) */
    bool _failed;
    /** Countdown active for winning or losing */
    int _countdown;

    float _shakeIntensity;
    int _shakeDuration;

    bool _inituiactive = true;

    /** Mark set to handle more sophisticated collision callbacks */
    std::unordered_set<b2Fixture*> _sensorFixtures;

#pragma mark Internal Object Management
    /**
     * Lays out the game geography.
     *
     * Pay close attention to how we attach physics objects to a scene graph.
     * The simplest way is to make a subclass, like we do for the dude.  However,
     * for simple objects you can just use a callback function to lightly couple
     * them.  This is what we do with the crates.
     *
     * This method is really, really long.  In practice, you would replace this
     * with your serialization loader, which would process a level file.
     */
    void populate(const std::shared_ptr<LevelModel>& level);

    
public:
#pragma mark -
#pragma mark Constructors
    /**
     * Creates a new game world with the default values.
     *
     * This constructor does not allocate any objects or start the controller.
     * This allows us to use a controller without a heap pointer.
     */
    GameScene();
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     *
     * This method is different from dispose() in that it ALSO shuts off any
     * static resources, like the input controller.
     */
    ~GameScene() { dispose(); }
    
    /**
     * Disposes of all (non-static) resources allocated to this mode.
     */
    void dispose();

    void setInitUIActive(bool active) {
        _inituiactive = active;
    }
    
    /**
     * Initializes the controller contents, and starts the game
     *
     * The constructor does not allocate any objects or memory.  This allows
     * us to have a non-pointer reference to this controller, reducing our
     * memory allocation.  Instead, allocation happens in this method.
     *
     * The game world is scaled so that the screen coordinates do not agree
     * with the Box2d coordinates.  This initializer uses the default scale.
     *
     * @param assets    The (loaded) assets for this game mode
     *
     * @return true if the controller is initialized properly, false otherwise.
     */
    bool init(const std::shared_ptr<AssetManager>& assets);

    /**
     * Initializes the controller contents, and starts the game
     *
     * The constructor does not allocate any objects or memory.  This allows
     * us to have a non-pointer reference to this controller, reducing our
     * memory allocation.  Instead, allocation happens in this method.
     *
     * The game world is scaled so that the screen coordinates do not agree
     * with the Box2d coordinates.  The bounds are in terms of the Box2d
     * world, not the screen.
     *
     * @param assets    The (loaded) assets for this game mode
     * @param rect      The game bounds in Box2d coordinates
     *
     * @return  true if the controller is initialized properly, false otherwise.
     */
    bool init(const std::shared_ptr<AssetManager>& assets,
              const Rect& rect);
    
    /**
     * Initializes the controller contents, and starts the game
     *
     * The constructor does not allocate any objects or memory.  This allows
     * us to have a non-pointer reference to this controller, reducing our
     * memory allocation.  Instead, allocation happens in this method.
     *
     * The game world is scaled so that the screen coordinates do not agree
     * with the Box2d coordinates.  The bounds are in terms of the Box2d
     * world, not the screen.
     *
     * @param assets    The (loaded) assets for this game mode
     * @param rect      The game bounds in Box2d coordinates
     * @param gravity   The gravitational force on this Box2d world
     *
     * @return  true if the controller is initialized properly, false otherwise.
     */
    void populateUI(const std::shared_ptr<cugl::AssetManager>& assets);
    void disableUI();
    void enableUI();
    bool init(const std::shared_ptr<AssetManager>& assets,
              const Rect& rect, const Vec2& gravity);

    void setAllGameplayUIActive(bool active);
    
    
#pragma mark -
#pragma mark State Access
    /**
     * Returns true if debug mode is active.
     *
     * If true, all objects will display their physics bodies.
     *
     * @return true if debug mode is active.
     */
    bool isDebug( ) const { return _debug; }
    
    /**
     * Sets whether debug mode is active.
     *
     * If true, all objects will display their physics bodies.
     *
     * @param value whether debug mode is active.
     */
    void setDebug(bool value) { _debug = value; _debugnode->setVisible(value); }
    
    /**
     * Returns true if the level is completed.
     *
     * If true, the level will advance after a countdown
     *
     * @return true if the level is completed.
     */
    bool isComplete( ) const { return _complete; }
    
    /**
     * Sets whether the level is completed.
     *
     * If true, the level will advance after a countdown
     *
     * @param value whether the level is completed.
     */
    void setComplete(bool value);

    /**
    * Returns true if the level is failed.
    *
    * If true, the level will reset after a countdown
    *
    * @return true if the level is failed.
    */
    bool isFailure() const { return _failed; }

    /**
    * Sets whether the level is failed.
    *
    * If true, the level will reset after a countdown
    *
    * @param value whether the level is failed.
    */
    void setFailure(bool value);
    
#pragma mark -
#pragma mark Collision Handling
    /**
    * Processes the start of a collision
    *
    * This method is called when we first get a collision between two objects.  We use
    * this method to test if it is the "right" kind of collision.  In particular, we
    * use it to test if we make it to the win door.  We also us it to eliminate bullets.
    *
    * @param  contact  The two bodies that collided
    */
    void beginContact(b2Contact* contact);

    /**
    * Processes the end of a collision
    *
    * This method is called when we no longer have a collision between two objects.
    * We use this method allow the character to jump again.
    *
    * @param  contact  The two bodies that collided
    */
    void endContact(b2Contact* contact);
    
    /**
     *
     * This function returns true if the provided Obstacle b is an enemy body
     * and if the fixture name `f` matches the enemy's body name.
     *
     * @param b Pointer to the Obstacle  being checked.
     * @param f Pointer to the fixture's user data (string).
     * @return True if the fixture and body correspond to an enemy body, false otherwise.
     */
    bool isEnemyBody(physics2::Obstacle* b, std::string f);
    /**
     *
     * This function returns true if the provided Obstacle b is the player body
     * and if the fixture name `f` matches the player's body name.
     *
     * @param b Pointer to the Obstacle  being checked.
     * @param f Pointer to the fixture's user data (string).
     * @return True if the fixture and body correspond to the player body, false otherwise.
     */
    bool isPlayerBody(physics2::Obstacle* b,const std::string* f);
    /**
     * @brief Checks if a projectile is hitting the player's shield.
     *
     * This function examines two possible (body, fixture) pairs to determine if a projectile
     * has collided with the player's shield. It returns a pointer to the projectile if a valid
     * collision is detected, otherwise returns nullptr.
     *
     * @param bd1 The first body involved in the contact.
     * @param fd1 The user data of the first fixture.
     * @param bd2 The second body involved in the contact.
     * @param fd2 The user data of the second fixture.
     * @return A pointer to the projectile that hit the shield, or nullptr if no such collision occurred.
     *
     * @note The function ensures that the projectile was not fired by the player and that the player's
     * shield is active before confirming a valid collision.
     */
    Projectile* getProjectileHitShield(physics2::Obstacle* bd1, std::string* fd1,
                                                physics2::Obstacle* bd2, std::string* fd2) const;

    /**
 Checks if contact is projectile hitting player shield and returns the Projectile if so, else NULL.
 */
    Projectile* getEnemyHitShield(physics2::Obstacle* bd1, std::string* fd1,
        physics2::Obstacle* bd2, std::string* fd2) const;

#pragma mark -
#pragma mark Gameplay Handling
     
    /**
     * The method called to indicate the start of a deterministic loop.
     *
     * This method is used instead of {@link #update} if {@link #setDeterministic}
     * is set to true. It marks the beginning of the core application loop,
     * which is concluded with a call to {@link #postUpdate}.
     *
     * This method should be used to process any events that happen early in
     * the application loop, such as user input or events created by the
     * {@link schedule} method. In particular, no new user input will be
     * recorded between the time this method is called and {@link #postUpdate}
     * is invoked.
     *
     * Note that the time passed as a parameter is the time measured from the
     * start of the previous frame to the start of the current frame. It is
     * measured before any input or callbacks are processed. It agrees with
     * the value sent to {@link #postUpdate} this animation frame.
     *
     * @param dt    The amount of time (in seconds) since the last frame
     */
    void preUpdate(float dt);
    
    /**
     * The method called to provide a deterministic application loop.
     *
     * This method provides an application loop that runs at a guaranteed fixed
     * timestep. This method is (logically) invoked every {@link getFixedStep}
     * microseconds. By that we mean if the method {@link draw} is called at
     * time T, then this method is guaranteed to have been called exactly
     * floor(T/s) times this session, where s is the fixed time step.
     *
     * This method is always invoked in-between a call to {@link #preUpdate}
     * and {@link #postUpdate}. However, to guarantee determinism, it is
     * possible that this method is called multiple times between those two
     * calls. Depending on the value of {@link #getFixedStep}, it can also
     * (periodically) be called zero times, particularly if {@link #getFPS}
     * is much faster.
     *
     * As such, this method should only be used for portions of the application
     * that must be deterministic, such as the physics simulation. It should
     * not be used to process user input (as no user input is recorded between
     * {@link #preUpdate} and {@link #postUpdate}) or to animate models.
     *
     * The time passed to this method is NOT the same as the one passed to
     * {@link #preUpdate}. It will always be exactly the same value.
     *
     * @param step  The number of fixed seconds for this step
     */
    void fixedUpdate(float step);

    /**
     * The method called to indicate the end of a deterministic loop.
     *
     * This method is used instead of {@link #update} if {@link #setDeterministic}
     * is set to true. It marks the end of the core application loop, which was
     * begun with a call to {@link #preUpdate}.
     *
     * This method is the final portion of the update loop called before any
     * drawing occurs. As such, it should be used to implement any final
     * animation in response to the simulation provided by {@link #fixedUpdate}.
     * In particular, it should be used to interpolate any visual differences
     * between the the simulation timestep and the FPS.
     *
     * This method should not be used to process user input, as no new input
     * will have been recorded since {@link #preUpdate} was called.
     *
     * Note that the time passed as a parameter is the time measured from the
     * last call to {@link #fixedUpdate}. That is because this method is used
     * to interpolate object position for animation.
     *
     * @param remain    The amount of time (in seconds) last fixedUpdate
     */
    void postUpdate(float remain);


    /**
     * Resets the status of the game so that we can play again.
     */
    void reset();

    // Make compiler stop whining
    void render(void) {
        scene2::Scene2::render();
    }

    /**
    * Removes the input Bullet from the world.
    *
    * @param  bullet   the bullet to remove
    */
    void removeProjectile(Projectile* bullet);
    
    void setPaused(bool paused) {
        _isPaused = paused;
    }

    /**
 * Sets a screen shake effect on current game scene.
 *
 * @param intensity the intensity of the shake
 * @param duration the duration of the shake
 */
    void setScreenShake(float intensity, int duration);

    /**
     * Moves the world node to process the current screen shake.
     *
     */
    void processScreenShake();
    
  };