#pragma once
#include <vector>
#include "Data/Render/Types/PostProcessType.h"

class DrawEngine;
class PostProcessRunner {
public:
    void SetChain(const std::vector<PostProcessType>& chain);

    void Execute(DrawEngine* engine);

private:
    std::vector<PostProcessType> chain_;

    void RunPass(DrawEngine* drawEngine, PostProcessType type);
};
