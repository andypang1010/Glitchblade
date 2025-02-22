//
//  GBRopeBridge.cpp
//  PlatformerDemo
//
//  This module provides the connected ropebridge from 3152. This time it is fully
//  assembled for you.  Note that this module handles its own scene graph management.
//  As a ComplexObstacle owns all of its child obstacles, it is natural for it to
//  own the corresponding scene graph.
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
#include "GBRopeBridge.h"

#include <cugl/cugl.h>
#include <box2d/b2_revolute_joint.h>
#include <box2d/b2_world.h>

using namespace cugl;

#pragma mark -
#pragma mark Physics Constants
/** The radius of each anchor pin */
#define BRIDGE_PIN_RADIUS   0.1f
/** The density of each plank in the bridge */
#define BASIC_DENSITY       1.0f

#pragma mark -
#pragma mark Contructors
/**
 * Initializes a new RopeBridge with the given start and end positions, and drawing scale.
 *
 * The scene graph is completely decoupled from the physics system.
 * The node does not have to be the same size as the physics body. We
 * only guarantee that the scene graph node is positioned correctly
 * according to the drawing scale.
 *
 * @param start The starting position of the bridge, in world coordinates
 * @param end   The ending position of the bridge, in world coordinates
 * @param psize The size of a single plank, in world units
 * @param scale The drawing scale (world to screen)
 *
 * @return  true if the obstacle is initialized properly, false otherwise.
 */
bool RopeBridge::init(const Vec2& start, const Vec2& end, const Size& psize, float scale) {

    _name = BRIDGE_NAME;
    _drawScale = scale;

	_linksize = psize.width;
	float spacing;

	// Compute the bridge length
	Vec2 dimen = end-start;
	float length = dimen.length();
	Vec2 norm = dimen;
	norm.normalize();

	// If too small, only make one plank.
	int nLinks = (int)(length / _linksize);
	if (nLinks <= 1) {
		nLinks = 1;
		_linksize = length;
		spacing = 0;
	} else {
		spacing = length - nLinks * _linksize;
		spacing /= (nLinks-1);
	}

	// Create the first pin
	std::shared_ptr<physics2::Obstacle> body = physics2::WheelObstacle::alloc(start,BRIDGE_PIN_RADIUS);
	body->setName(BRIDGE_PIN_NAME+strtool::to_string(0));
	body->setDensity(BASIC_DENSITY);
	body->setBodyType(b2_staticBody);
	_obstacles.push_back(body);

	// Create the planks
    Size planksize = psize;
	planksize.width = _linksize;
	for (int ii = 0; ii < nLinks; ii++) {
		float t = ii*(_linksize+spacing) + _linksize/2.0f;
		Vec2 pos = norm*t+start;
		std::shared_ptr<physics2::BoxObstacle> plank = physics2::BoxObstacle::alloc(pos, planksize);
		body->setName(PLANK_NAME+strtool::to_string(ii));
		plank->setDensity(BASIC_DENSITY);
        _obstacles.push_back(plank);
	}

	// Create the last pin
	body = physics2::WheelObstacle::alloc(end,BRIDGE_PIN_RADIUS);
	body->setName(BRIDGE_PIN_NAME+strtool::to_string(1));
	body->setDensity(BASIC_DENSITY);
	body->setBodyType(b2_staticBody);
    _obstacles.push_back(body);
    createJoints();

	return true;
}


#pragma mark -
#pragma mark Physics Methods
/**
 * Activates all of the obstacles in this model.
 *
 * This method invokes {@link Obstacle#activatePhysics} for the each
 * of the obstacles in this model.
 *
 * @param world    The associated obstacle world
 */
void RopeBridge::activate(const std::shared_ptr<physics2::ObstacleWorld>& world) {
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
void RopeBridge::deactivate(const std::shared_ptr<physics2::ObstacleWorld>& world) {
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
bool RopeBridge::createJoints() {
	Vec2 anchor1(0,0);
	Vec2 anchor2(-_linksize/2.0f,0);

    std::shared_ptr<physics2::RevoluteJoint> joint;

    joint = physics2::RevoluteJoint::allocWithObstacles(_obstacles[0],
                                                        _obstacles[1]);
    joint->setLocalAnchorA(anchor1);
    joint->setLocalAnchorB(anchor2);
    joint->setCollideConnected(false);
	_joints.push_back(joint);

	// Planks together
	anchor1.x = _linksize / 2;
	for (int ii = 1; ii < _obstacles.size()-2; ii++) {
        joint = physics2::RevoluteJoint::allocWithObstacles(_obstacles[ii],
                                                            _obstacles[ii+1]);
        joint->setLocalAnchorA(anchor1);
        joint->setLocalAnchorB(anchor2);
        joint->setCollideConnected(false);
        _joints.push_back(joint);
	}

	// Final joint
	anchor2.x = 0;
	int ii = (int)_obstacles.size();
    joint = physics2::RevoluteJoint::allocWithObstacles(_obstacles[ii-2],
                                                        _obstacles[ii-1]);
    joint->setLocalAnchorA(anchor1);
    joint->setLocalAnchorB(anchor2);
    joint->setCollideConnected(false);
    _joints.push_back(joint);

	return true;
}

/**
 * Disposes all resources and assets of this RopeBridge
 *
 * Any assets owned by this object will be immediately released.  Once
 * disposed, a RopeBridge may not be used until it is initialized again.
 */
void RopeBridge::dispose() {
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
void RopeBridge::setTexture(const std::shared_ptr<graphics::Texture>& texture) {
    _image = texture;
    if (_node == nullptr) {
        return;
    }
    
    // Do not add nodes for the beginning and end bodies
    _node->removeAllChildren();
    for(int ii = 1; ii+1 < _obstacles.size(); ii++) {
        std::shared_ptr<scene2::PolygonNode> sprite = scene2::PolygonNode::allocWithTexture(_image);
        sprite->setPosition(_obstacles[ii]->getPosition() * _drawScale);
        _node->addChild(sprite);
    }
}


/**
 * Performs any necessary additions to the scene graph node.
 *
 * This method is necessary for custom physics objects that are composed
 * of multiple scene graph nodes.
 */
void RopeBridge::setSceneNode(const std::shared_ptr<scene2::SceneNode>& node) {
	_node = node;
    if (_image == nullptr) {
        return;
    }

	// Do not add nodes for the beginning and end bodies
	for(int ii = 1; ii+1 < _obstacles.size(); ii++) {
		std::shared_ptr<scene2::PolygonNode> sprite = scene2::PolygonNode::allocWithTexture(_image);
		sprite->setPosition(_obstacles[ii]->getPosition() * _drawScale);
		_node->addChild(sprite);
	}
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
void RopeBridge::setDebugScene(const std::shared_ptr<scene2::SceneNode>& node) {
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
void RopeBridge::setDebugColor(Color4 color) {
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
void RopeBridge::setDrawScale(float scale) {
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
void RopeBridge::update(float delta) {
	if (_node != nullptr) {
		std::vector<std::shared_ptr<scene2::SceneNode>> children = _node->getChildren();
		//begin at index 1 since body 0 does not correspond to a node
		//given how we populate the children in setSceneNode
        int ii = 1;

		// Update the nodes of the attached bodies
		for (auto it = children.begin(); it != children.end(); ++it) {
            // This interpolates
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
