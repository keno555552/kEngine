#pragma once
#include "Utility/EnumNameFinder/EnumNameFinder.h"

enum class RenderModelType {

	/// 描画モデルの種類
	Static = 0,
	Skinned,
	DebugLine,
	Environment,
	Sprite2D,
	FlameNeonGlow,

	/// PostProcess用の描画モデル
	ColorGradient,
	Vignette,
	Blur,
	Outline,
	OutlinePrewittDepth,
	Dissolve,
	Noise,
	FullscreenQuad,

	/// RenderModelの数
	NumOfRenderModels,
};

template<>
inline const std::map<RenderModelType, std::string> EnumNameMap<RenderModelType> = {
	{ RenderModelType::Static,				"Static" },
	{ RenderModelType::Skinned,				"Skinned" },
	{ RenderModelType::DebugLine,			"DebugLine" },
	{ RenderModelType::Environment,			"Environment" },
	{ RenderModelType::Sprite2D,			"Sprite2D" },
	{ RenderModelType::FlameNeonGlow,		"FlameNeonGlow" },
	{ RenderModelType::ColorGradient,		"ColorGradient" },
	{ RenderModelType::Vignette,			"Vignette" },
	{ RenderModelType::Blur,				"Blur" },
	{ RenderModelType::Outline,				"Outline" },
	{ RenderModelType::OutlinePrewittDepth, "OutlinePrewittDepth" },
	{ RenderModelType::Dissolve,			"Dissolve" },
	{ RenderModelType::Noise,				"Noise" },
	{ RenderModelType::FullscreenQuad,		"FullscreenQuad" }
};
