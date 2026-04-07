#pragma once
#include <vector>
#include <string>
#include <optional>
#include "MathsIncluder.h"
#include "Transform.h"

struct Joint {

    int32_t ID;                                // 自身のIndex
    Transform transform;                       // Transform情報
    Matrix4x4 localMatrix;                     // localMatrix
    Matrix4x4 skeletonSpaceMatrix;             // skeletonSpaceでの変換行列
    std::string name;                          // 名前
    std::vector<int32_t> childrenIDList;       // 子JointのIndexのリスト。  いなければ空
    std::optional<int32_t> parentIDList;       // 親JointのIndex。         いなければnull
};
