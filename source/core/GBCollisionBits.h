#ifndef __GB_COLLISION_BITS_H__
#define __GB_COLLISION_BITS_H__

#pragma once
#include <cstdint>

namespace CollisionGroup {
    constexpr uint16_t CATEGORY_DEFAULT = 0x0001;
    constexpr uint16_t CATEGORY_ENEMY = 0x0002;
    constexpr uint16_t CATEGORY_PLAYER = 0x0004;
    constexpr uint16_t CATEGORY_PLATFORM = 0x0008;
    constexpr uint16_t CATEGORY_PLATFORM_WALL = 0x00010;
    constexpr uint16_t CATEGORY_PROJECTILE = 0x0012;
}

#endif /* __GB_COLLISION_BITS_H__ */
