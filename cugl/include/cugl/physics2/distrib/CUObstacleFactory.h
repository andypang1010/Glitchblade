//
//  CUObstacleFactory.h
//  Cornell University Game Library (CUGL)
//
//  This module provides a standard template for an shareable creation of
//  obstacles. Users can create their own factory and subclass this class
//  to create their custom obstacles.
// 
//  CUGL MIT License:
//      This software is provided 'as-is', without any express or implied
//      warranty.  In no event will the authors be held liable for any damages
//      arising from the use of this software.
//
//      Permission is granted to anyone to use this software for any purpose,
//      including commercial applications, and to alter it and redistribute it
//      freely, subject to the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//      claim that you wrote the original software. If you use this software
//      in a product, an acknowledgment in the product documentation would be
//      appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not
//      be misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
//
//  Author: Barry Lyu
//  Version: 7/3/24 (CUGL 3.0 reorganization)
//
#ifndef __CU_OBSTACLE_FACTORY_H__
#define __CU_OBSTACLE_FACTORY_H__
#include <cugl/scene2/CUSceneNode2.h>
#include <cugl/physics2/CUObstacle.h>

namespace cugl {

    /**
     * The classes to represent 2-d physics.
     *
     * For 2-d physics, CUGL uses the venerable box2d. For the most part, we
     * do not need anything more than that. However, box2d does involve a lot
     * of boilerplate code in setting up bodies and fixtures. Students have
     * found that they like the "training wheel" classes in this package.
     */
    namespace physics2 {
    
        /**
         * The classes to implement distributed box2d physics.
         *
         * This namespace represents an extension of our 2-d physics engine
         * to support networking. This package provides automatic synchronization
         * of physics objects across devices.
         */
        namespace distrib {
        
/** Group the obstacle/scene pair into a type for easier reading */
typedef std::pair<std::shared_ptr<physics2::Obstacle>, std::shared_ptr<scene2::SceneNode>> ObstacleScene;
    
/**
 * A factory for creating shared obstacles.
 *
 * This class provides a standard template for an shareable creation of obstacles.
 * These factories can be attached to the provided net physics controller so that
 * creation of obstacles can be synchronized across devices without the need to
 * send texture and body information.
 *
 * In order for every client to be updated when a new obstacle is created, they
 * must have a uniform way to share info about the obstacle to be created. However,
 * syncing the texture and body data is very costly, so this class is a template
 * for creating an obstacle and (optionally) a scene node from serialized parameters.
 */
class ObstacleFactory {
public:
    /**
     * Returns a newly allocated obstacle factory.
     */
    static ::std::shared_ptr<ObstacleFactory> alloc() {
        return std::make_shared<ObstacleFactory>();
    };
    
    /**
     * Returns a new obstacle from the serialized parameters
     *
     * If you want the obstacle to be accompanied by a scene node, you must
     * return a pair of the obstacle and the scene node. You must also call the
     * {@link NetEventController#enablePhysics() } method and provide it a
     * function for linking the obstacle and scene node.
     *
     * It is possible for the SceneNode in the returned part to be nullptr. In
     * that case, the obstacle will be created without a scene node.
     *
     * The actual parameters are up to your network protocol. Some typical
     * parameters include:
     *  - The texture name
     *  - The size of the obstacle
     *  - The position/velocity of the obstacle
     *  - Any metadata about the obstacle
     *
     * @param params    The serialized parameters for the obstacle
     */
    virtual ObstacleScene createObstacle(const std::vector<std::byte>& params) {
        return std::make_pair(std::make_shared<physics2::Obstacle>(), std::make_shared<scene2::SceneNode>());
    }
    
};
        }
    }
}

#endif /* __CU_OBSTACLE_FACTORY_H__ */
