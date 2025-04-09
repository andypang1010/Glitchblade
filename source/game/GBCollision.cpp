//
//  GBCollision.cpp
//  gb-mac
//
//  Created by Sasha Loayza on 4/9/25.
//

#include "GBCollision.h"


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
