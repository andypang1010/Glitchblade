//
//  GBSpinner.cpp
//  PlatformerDemo
//
//  This class provides a spinning rectangle on a fixed pin, as ComplexObstacles
//  always make joint management easier.
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
#include "GBSpinner.h"
#include <cugl/cugl.h>
#include <box2d/b2_revolute_joint.h>
#include <box2d/b2_world.h>

using namespace cugl;
using namespace cugl::graphics;

#pragma mark -
#pragma mark Physics Constants

/** The radius of the central pin */
#define SPIN_PIN_RADIUS 0.1f
/** The density for the spinning barrier */
#define HEAVY_DENSITY  10.0f
/** The density for the central pin */
#define LIGHT_DENSITY   1.0f
/** The dimensions of the spinner */

#pragma mark -
#pragma mark Initializers
/**
 * Initializes a new spinner at the given position.
 *
 * The spinner is sized according to the given drawing scale. The spinner
 * is centered on its position.
 *
 * The scene graph is completely decoupled from the physics system.
 * The node does not have to be the same size as the physics body. We
 * only guarantee that the scene graph node is positioned correctly
 * according to the drawing scale.
 *
 * @param pos   Initial position in world coordinates
 * @param size  The spinner size in world units
 * @param scale The drawing scale (world to screen)
 *
 * @return  true if the obstacle is initialized properly, false otherwise.
 */
bool Spinner::init(const Vec2 pos, const Vec2 size, float scale) {
    _name = SPINNER_NAME;
	_drawScale = scale;
    
	// Create the barrier
	std::shared_ptr<physics2::Obstacle> body = physics2::BoxObstacle::alloc(pos,size);
	body->setName(BARRIER_NAME);
	body->setDensity(HEAVY_DENSITY);
	_obstacles.push_back(body);

	// Create the pin
	body = physics2::WheelObstacle::alloc(pos,SPIN_PIN_RADIUS);
	body->setName(SPIN_PIN_NAME);
	body->setDensity(LIGHT_DENSITY);
	body->setBodyType(b2_staticBody);
    _obstacles.push_back(body);
    createJoints();
	return true;
}

/**
 * Activates all of the obstacles in this model.
 *
 * This method invokes {@link Obstacle#activatePhysics} for the each
 * of the obstacles in this model.
 *
 * @param world    The associated obstacle world
 */
void Spinner::activate(const std::shared_ptr<physics2::ObstacleWorld>& world) {
    for(auto it = _obstacles.begin(); it != _obstacles.end(); ++it) {
        world->addObstacle(*it);
    }
    for(auto it = _joints.begin(); it != _joints.end(); ++it) {
        world->addJoint(*it);
    }
}

/**
 * Destroys the obstacles in this model, if appropriate,
 * removing them from the world.
 *
 * @param world    The associated obstacle world
 */
void Spinner::deactivate(const std::shared_ptr<physics2::ObstacleWorld>& world) {
    for(auto it = _obstacles.begin(); it != _obstacles.end(); ++it) {
        world->removeObstacle(*it);
    }
    for(auto it = _joints.begin(); it != _joints.end(); ++it) {
        world->removeJoint(*it);
    }
}

/**
 * Creates the joints for this object.
 *
 * This method defines the joints but does not activate them. That is the
 * purpose of {@link #activate}.
 *
 * @return true if object allocation succeeded
 */
bool Spinner::createJoints() {
    std::shared_ptr<physics2::RevoluteJoint> joint;

    joint = physics2::RevoluteJoint::allocWithObstacles(_obstacles[0],
                                                        _obstacles[1]);
    joint->setLocalAnchorA(Vec2::ZERO);
    joint->setLocalAnchorB(Vec2::ZERO);
    _joints.push_back(joint);

	return true;
}

/**
 * Disposes all resources and assets of this Spinner
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a Spinner may not be used until it is initialized again.
 */
void Spinner::dispose() {
	_node = nullptr;
	_image = nullptr;
	_obstacles.clear();
}


#pragma mark -
#pragma mark Scene Graph Internals
/**
 * Sets the plank texture
 *
 * This texture is used to construct the individual scene graph nodes
 * for each plank
 *
 * @param texture   The plank texture
 */
void Spinner::setTexture(const std::shared_ptr<graphics::Texture>& texture) {
    _image = texture;
    if (_node != nullptr) {
        std::shared_ptr<scene2::PolygonNode> sprite = scene2::PolygonNode::allocWithTexture(_image);
        _node->addChild(sprite);
    }
}

/**
 * Performs any necessary additions to the scene graph node.
 *
 * This method is necessary for custom physics objects that are composed
 * of multiple scene graph nodes.
 */
void Spinner::setSceneNode(const std::shared_ptr<scene2::SceneNode>& node) {
	_node = node;
    if (_image != nullptr) {
        std::shared_ptr<scene2::PolygonNode> sprite = scene2::PolygonNode::allocWithTexture(_image);
        _node->addChild(sprite);
    }
	// Spinner is invisible
}

/**
 * Sets the parent scene graph node for the debug wireframe
 *
 * The given node is the parent coordinate space for drawing physics.
 * All debug nodes for physics objects are drawn within this coordinate
 * space.  Setting the visibility of this node to false will disable
 * any debugging.  Similarly, setting this value to nullptr will
 * disable any debugging.
 *
 * This scene graph node is intended for debugging purposes only.  If
 * you want a physics body to update a proper texture image, you should
 * either use the method {@link update(float)} for subclasses or
 * {@link setListener} for decoupled classes.
 *
 * @param node  he parent scene graph node for the debug wireframe
 */
void Spinner::setDebugScene(const std::shared_ptr<scene2::SceneNode>& node) {
    for(auto it = _obstacles.begin(); it != _obstacles.end(); it++) {
        (*it)->setDebugScene(node);
    }
}

/**
 * Sets the color of the debug wireframe.
 *
 * The default color is white, which means that the objects will be shown
 * with a white wireframe.
 *
 * @param color the color of the debug wireframe.
 */
void Spinner::setDebugColor(Color4 color) {
    for(auto it = _obstacles.begin(); it!= _obstacles.end(); ++it) {
        (*it)->setDebugColor(color);
    }
}

/**
 * Sets the ratio of the Ragdoll sprite to the physics body
 *
 * The Ragdoll needs this value to convert correctly between the physics
 * coordinates and the drawing screen coordinates.  Otherwise it will
 * interpret one Box2D unit as one pixel.
 *
 * All physics scaling must be uniform.  Rotation does weird things when
 * attempting to scale physics by a non-uniform factor.
 *
 * @param scale The ratio of the Ragdoll sprite to the physics body
 */
void Spinner::setDrawScale(float scale) {
    _drawScale = scale;
}

#pragma mark -
#pragma mark Interpolation
/**
 * Updates the scene graph to interpolate the visuals.
 *
 * This method is called AFTER all physics is complete. The value delta
 * is the time remaining that could not be simulated. The objects in the
 * scene graph node are interpolated to match that time.
 *
 * @param delta The time since the last physics simulation call
 */
void Spinner::update(float delta) {
	if (_node != nullptr) {
		std::vector<std::shared_ptr<scene2::SceneNode>> children = _node->getChildren();
		int ii = 0;

        // Update the nodes of the attached bodies
        for (auto it = children.begin(); it != children.end(); ++it) {
            Vec2 pos = _obstacles[ii]->getPosition();
            pos += _obstacles[ii]->getLinearVelocity()*delta;
            (*it)->setPosition(pos*_drawScale);
            
            float angle = _obstacles[ii]->getAngle();
            angle += _obstacles[ii]->getAngularVelocity()*delta;
            (*it)->setAngle(angle);

            ii++;
        }
        
	}
}
