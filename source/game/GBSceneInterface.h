#pragma once

#include <cugl/cugl.h>

class GBSceneInterface {
public:
    virtual void postUpdate(float delta) = 0;
    virtual void preUpdate(float delta) = 0;
    virtual void fixedUpdate(float delta) = 0;
    virtual void render() = 0;
};
