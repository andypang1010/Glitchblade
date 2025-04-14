#pragma once

#include <cugl/cugl.h>

class GBSceneInterface {
private:
    int _swapSignal = 0;
public:
    virtual void postUpdate(float delta) = 0;
    virtual void preUpdate(float delta) = 0;
    virtual void fixedUpdate(float delta) = 0;
    virtual void render() = 0;
    int getSwapSignal() {
        return _swapSignal;
    }
    void setSwapSignal(int signal) {
        _swapSignal = signal;
    }
};
