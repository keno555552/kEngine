#pragma once
#include "Utility/EnumNameFinder/EnumNameFinder.h"

enum class RasterizerMode{

    CullBack = 0,           // ノーマルモデル
    CullFront,              // フロントカリングモデル
    CullNone,               // ダブルサイドモデル / パーティクル
    Wireframe,              // デバッグ
    //Shadow,               // シャドウマップ専用
    NumRasterizerModes
};

template <>
inline const std::map<RasterizerMode, std::string> EnumNameMap<RasterizerMode> = {
	{ RasterizerMode::CullBack,		"CullBack" },
	{ RasterizerMode::CullFront,	"CullFront" },
	{ RasterizerMode::CullNone,		"CullNone" },
	{ RasterizerMode::Wireframe,	"Wireframe" }
};