#pragma once
#include "Data/Render/GPUData/RenderCommandGPU.h"

class DrawEngine;
class PostProcessPass
{
public:
    virtual ~PostProcessPass() = default;
    virtual void Run(DrawEngine* engine, const RenderCommandGPU& cmd) = 0;
};

class BlurPass : public PostProcessPass {
public:
    void Run(DrawEngine* engine, const RenderCommandGPU& cmd) override;
};