//
//  CUScene.cpp
//  Cornell University Game Library (CUGL)
//
//  This module provides support for a basic scene in the application. Most
//  applications are composed of scenes, with some way of switching between
//  them. Historically, CUGL was composed primarily of 2d scenes which are
//  supported by the Scene package. However, this is not required, and so
//  this base implementation is 2d/3d agnostic.
//
//  That means of course that this base scene has no associated scene graph.
//  That is the responsibility of any subclasses.
//
//  This class uses our standard shared-pointer architecture.
//
//  1. The constructor does not perform any initialization; it just sets all
//     attributes to their defaults.
//
//  2. All initialization takes place via init methods, which can fail if an
//     object is initialized more than once.
//
//  3. All allocation takes place via static constructors which return a shared
//     pointer.
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
//  Author: Walker White
//  Version: 7/3/24 (CUGL 3.0 reorganization)
//
#include <sstream>
#include <cugl/core/CUScene.h>
#include <cugl/core/CUApplication.h>
#include <cugl/core/CUDisplay.h>
#include <cugl/core/util/CUDebug.h>

using namespace cugl;

/**
 * Initializes a Scene to fill the entire screen.
 *
 * @return true if initialization was successful.
 */
bool Scene::init() {
    _size = Application::get()->getDisplaySize();
    return true;
}

/**
 * Initializes a Scene with the given size hint.
 *
 * Scenes are designed to fill the entire screen. If you want a scene that
 * is only part of the screen, that should be implemented with a specific
 * scene graph. However, the size of that screen can vary from device to
 * device. To make scene design easier, designs are typically locked to a
 * dimension: width or height.
 *
 * This is the purpose of the size hint. If either of the values of hint
 * are non-zero, then the scene will lock that dimension to that particular
 * size. If both are non-zero, it will choose its dimension according to the
 * device orientation. Landscape will be height, while portrait will pick
 * width. Devices with no orientation will always priortize height over
 * width.
 *
 * @param hint      The size hint
 *
 * @return true if initialization was successful.
 */
bool Scene::initWithHint(Size hint) {
    if (hint.width < 0 || hint.height < 0) {
        CUAssertLog(false,"Size hint (%f,%f) must be nonnegative",hint.width,hint.height);
        return false;
    }
    
    _size = Application::get()->getDisplaySize();
    if (hint.width == 0 && hint.height == 0) {
        // No hints
        return true;
    } else if (hint.width > 0 && hint.height > 0) {
        Display::Orientation orient = Display::get()->getDisplayOrientation();
        if (orient == Display::Orientation::PORTRAIT ||
            orient == Display::Orientation::UPSIDE_DOWN) {
            hint.height = 0;
        } else {
            hint.width = 0;
        }
    }
    
    if (hint.width > 0) {
        _size *= hint.width/_size.width;
    } else {
        _size *= hint.height/_size.height;
    }
    
    return true;
}

/**
 * Returns a string representation of this scene for debugging purposes.
 *
 * If verbose is true, the string will include class information.  This
 * allows us to unambiguously identify the class.
 *
 * @param verbose Whether to include class information
 *
 * @return a string representation of this scene for debuggging purposes.
 */
std::string Scene::toString(bool verbose) const {
    std::stringstream ss;
    ss << (verbose ? "cugl::Scene(name:" : "(name:");
    ss << _name << ")";
    return ss.str();
}
