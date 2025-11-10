#pragma once
#include "DircetXCore.h"

class FrameRateLimiter {
public:
    FrameRateLimiter(double targetFPS = 60.0f);

    void Wait();

private:
    LARGE_INTEGER frequency;
    LARGE_INTEGER lastTime;
    double targetFrameTime;
};


