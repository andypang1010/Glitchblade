/*
 * SDL_app:  An all-in-one library for packing SDL applications.
 * Copyright (C) 2022-2023 Walker M. White
 *
 * This library is built on the assumption that an application built for SDL
 * will contain its own versions of the SDL libraries (either statically linked
 * or packaged with a specific set of dynamic libraries).  While this is not
 * considered the right way to do it on Unix, it makes one step installation
 * easier for Mac and Windows. It is also the only way to create SDL apps for
 * mobile devices.
 *
 * SDL License:
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */
#include "SDL_app.h"

/**
 * Returns the version of the given dependency
 *
 * This allows the program to query the versions of the various libaries that
 * SDL_app depends on.
 *
 * @param dep   The library dependency
 *
 * @return the version of the given dependency
 */
const char* APP_GetVersion(APP_Depedency dep) {
    switch (dep) {
        case APP_DEPENDENCY_SDL:
            return "2.30.4";
        case APP_DEPENDENCY_IMG:
            return "2.8.2";
        case APP_DEPENDENCY_TTF:
            return "2.22.0";
        case APP_DEPENDENCY_ATK:
            return "2.0.0";
        case APP_DEPENDENCY_APP:
            return "2.2.0";
        default:
            break;
    }
    return "";
}
