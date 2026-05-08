#pragma once
#include "Data/Render/Types/PSOType/LightModelType.h"
#include "Data/Render/Types/PSOType/RenderModelType.h"
#include "Data/Render/Types/PSOType/RasterizerMode.h"
#include "Data/Render/Types/PSOType/BlendModeType.h"
#include "Data/Render/Types/PSOType/DepthStencilType.h"
#include "Data/Render/Types/PSOType/PrimitiveType.h"

#include "config.h"

struct PSOKey{

	LightModelType lightModelType = (LightModelType)config::default_LightModel_;
	RenderModelType renderModelType = (RenderModelType)config::default_RenderModelTypes_;
	BlendModeType blendModeType = (BlendModeType)config::default_BlendMode_;
	RasterizerMode rasterizerMode = (RasterizerMode)config::default_RasterizerMode_;
	DepthStencilType depthStencilType = (DepthStencilType)config::default_DepthStenctilState_;
	PrimitiveType primitiveType = PrimitiveType::TRIANGLE;

};

// 比較
inline bool operator==(const PSOKey& a, const PSOKey& b) {
	return a.lightModelType == b.lightModelType
		&& a.renderModelType == b.renderModelType
		&& a.blendModeType == b.blendModeType
		&& a.rasterizerMode == b.rasterizerMode
		&& a.depthStencilType == b.depthStencilType
		&& a.primitiveType == b.primitiveType;
}

// 比較
inline bool operator!=(const PSOKey& a, const PSOKey& b) {
	return !(a == b);
}

// 順序比較（std::map用）
inline bool operator<(const PSOKey& a, const PSOKey& b) {
	return std::tie(
		a.lightModelType,
		a.renderModelType,
		a.blendModeType,
		a.rasterizerMode,
		a.depthStencilType,
		a.primitiveType
	) < std::tie(
		b.lightModelType,
		b.renderModelType,
		b.blendModeType,
		b.rasterizerMode,
		b.depthStencilType,
		b.primitiveType
	);
}

// hash
template<>
struct std::hash<PSOKey> {
	size_t operator()(const PSOKey& key) const noexcept {
		size_t h = 0;
		auto hash_combine = [&h](auto v) {
			h ^= std::hash<std::underlying_type_t<decltype(v)>>{}(
				static_cast<std::underlying_type_t<decltype(v)>>(v)
				) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
			};
		hash_combine(key.lightModelType);
		hash_combine(key.renderModelType);
		hash_combine(key.blendModeType);
		hash_combine(key.rasterizerMode);
		hash_combine(key.depthStencilType);
		hash_combine(key.primitiveType);
		return h;
	}
};

inline PSOKey CreateDefaultPSOKey(){
	PSOKey defaultKey;
	defaultKey.lightModelType = (LightModelType)config::default_LightModel_;
	defaultKey.renderModelType = (RenderModelType)config::default_RenderModelTypes_;
	defaultKey.blendModeType = (BlendModeType)config::default_BlendMode_;
	defaultKey.rasterizerMode = (RasterizerMode)config::default_RasterizerMode_;
	defaultKey.depthStencilType = (DepthStencilType)config::default_DepthStenctilState_;
	defaultKey.primitiveType = PrimitiveType::TRIANGLE;
	return defaultKey;
}

inline PSOKey CreateDebugLinePSOKey(){
	PSOKey debugLineKey;
	debugLineKey.lightModelType = LightModelType::DebugLine;
	debugLineKey.renderModelType = RenderModelType::DebugLine;
	debugLineKey.blendModeType = BlendModeType::NormalBlend;
	debugLineKey.rasterizerMode = RasterizerMode::Wireframe;
	debugLineKey.depthStencilType = DepthStencilType::Disable;
	debugLineKey.primitiveType = PrimitiveType::LINE;
	return debugLineKey;
}