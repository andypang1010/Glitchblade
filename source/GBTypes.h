//
//  GBTypes.h
//  GB
//
//  Created by Sasha Loayza on 3/15/25.
//

#ifndef __GB_TYPES_H__
#define __GB_TYPES_H__

#include <utility> // For std::pair
#include <memory>  // For std::shared_ptr

using ObstaclePtr = std::shared_ptr<cugl::physics2::Obstacle>;
using NodePtr = std::shared_ptr<cugl::scene2::SceneNode>;

// Define a pair of ObstaclePtr and SceneNodePtr
using ObstacleNodePair = std::pair<ObstaclePtr, NodePtr>;
using ObstacleNodePairs = std::vector<ObstacleNodePair>;

#endif /* __GB_TYPES_H__ */
