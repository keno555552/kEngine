#pragma once
#include <stdint.h>
#include "LinearAlgebra/Vector4.h"
#include "LinearAlgebra/Matrix4x4.h"
#include "Renderer/PSOManager/PSOKeys.h"
#include "config.h"

class MaterialConfig
{
public:

	/// Texture関連
	bool useModelTexture = true;
	int textureHandle = 0;

	/// UV関連
	Vector3 uvTranslate = { 0,0,0 };
	Vector3 uvScale = { 1,1,1 };
	Vector3 uvRotate = { 0,0,0 };
	Matrix4x4 uvTransformMatrix{};
	Vector4 textureColor{ 1.0f,1.0f,1.0f,1.0f };

	/// ======================== PSO関連 ======================== ///
	// LightModelType ライティングモデルの種類、今主にParticle.PSの中にライティングを影響している
	// RenderModelType 使用するshaderに影響する、今はInputLayout, RootSignature, Shaderのfactoryに影響している
	// BlendModeType Blendの仕方に影響する、今はBlendStateFactoryに影響している
	// RasterizerMode 描画方法に影響する、特別な描画する時にいじる、今はRasterizerModeFactoryに影響している
	// DepthStencilType 震度バッファの処理方法に影響する、今はDepthStencilStateFactoryに影響している
	// PrimitiveTypeは他のPSOデータにより決定されるため、MaterialConfigには含めない

    LightModelType lightModelType =       (LightModelType)    config::default_LightModel_;
    RenderModelType renderModelType =     (RenderModelType)   config::default_RenderModelTypes_;
    BlendModeType blendModeType =         (BlendModeType)     config::default_BlendMode_;
    RasterizerMode rasterizerMode =       (RasterizerMode)    config::default_RasterizerMode_;
    DepthStencilType depthStencilType =   (DepthStencilType)  config::default_DepthStenctilState_;

	/// 環境マッピングに使用する、0になると使ってない
	float reflectiveStrength = 0.0f;

	/// ライティングの有効無効、今はParticle.PSの中に影響している
	bool enableLighting = true;

	/// 内部処理用
	int materialResourceHandle = -1;
	int drawState = 0;

	/// ===================== 追加パラメータ ===================== ///
	Vector4 textureColor2{ 1.0f,1.0f,1.0f,1.0f };
	Vector4 textureColor3{ 1.0f,1.0f,1.0f,1.0f };
	float shininess = 32.0f;
    float intensity = 1.0f;
    float heightScale = 2.0f;

	bool operator==(const MaterialConfig& target)const;
	bool operator!=(const MaterialConfig& target)const;

	void MakeUVMatrix();

	/// =================== 便利PSO関連生成関数 =================== ///

	void MakePSOParticle();
	void MakePSOEnvironment();
	void MakePSODebugLine();
};

//struct MaterialConfigHash {
//    size_t operator()(const MaterialConfig& m) const noexcept {
//        size_t h = 0;
//        auto hash_combine = [&](auto&& v) {
//            h ^= std::hash<std::decay_t<decltype(v)>>{}(v)
//                +0x9e3779b9 + (h << 6) + (h >> 2);
//            };
//
//        // -------- 使用者設定（可比較、可 hash） --------
//        hash_combine(m.useModelTexture);
//        hash_combine(m.textureHandle);
//
//        hash_combine(m.uvTranslate.x);
//        hash_combine(m.uvTranslate.y);
//        hash_combine(m.uvTranslate.z);
//
//        hash_combine(m.uvScale.x);
//        hash_combine(m.uvScale.y);
//        hash_combine(m.uvScale.z);
//
//        hash_combine(m.uvRotate.x);
//        hash_combine(m.uvRotate.y);
//        hash_combine(m.uvRotate.z);
//
//        hash_combine(m.textureColor.x);
//        hash_combine(m.textureColor.y);
//        hash_combine(m.textureColor.z);
//        hash_combine(m.textureColor.w);
//
//        hash_combine((int)m.lightModelType);
//        hash_combine(m.enableLighting);
//        hash_combine(m.shininess);
//
//        // -------- 引擎內部資料（不 hash） --------
//        // m.uvTransformMatrix
//        // m.materialResourceHandle
//        // m.drawState
//
//        return h;
//    }
//};

/// MaterialConfig初期化
void InitMaterialConfig(MaterialConfig* materialConfig);
MaterialConfig InitMaterialConfig();

/// BlendMode簡易設定
void SetBlendFlameNeonGlow(MaterialConfig* materialConfig);
