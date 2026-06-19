#include "ShaderFactory.h"
#include "Logger.h"
#include <cassert>

ShaderFactory::ShaderFactory() {
	shader_compile_ = std::make_unique<Shader_compile>();
	shader_compile_->Initialize();

	shaderFolder_ = "./kEngine/EngineAssets/Shader/";
	shaderRegistry_[RenderModelType::Sprite2D] = [this](PSOKey& key) { return Compile2DShader(key); };
	shaderRegistry_[RenderModelType::Static] = [this](PSOKey& key) { return Compile3DShader(key); };
	shaderRegistry_[RenderModelType::Skinned] = [this](PSOKey& key) { return Compile3DShader(key); };
	shaderRegistry_[RenderModelType::DebugLine] = [this](PSOKey& key) { return CompileShaderByFileName("DebugLine", key); };
	shaderRegistry_[RenderModelType::Environment] = [this](PSOKey& key) { return CompileShaderByFileName("SkyCube", key); };
	shaderRegistry_[RenderModelType::FlameNeonGlow] = [this](PSOKey& key) { return CompileShaderByFileName("FlameNeonGlow", key); };

	/// PostProcess用の描画モデル
	shaderRegistry_[RenderModelType::FullscreenQuad] = [this](PSOKey& key) { return CompileScreenQuad(key); };
	shaderRegistry_[RenderModelType::ColorGradient] = [this](PSOKey& key) { return CompileColorGuard(key); };
	shaderRegistry_[RenderModelType::Vignette] = [this](PSOKey& key) { return CompileVignetting(key); };
	shaderRegistry_[RenderModelType::Blur] = [this](PSOKey& key) { return CompileBlur(key); };
	shaderRegistry_[RenderModelType::Outline] = [this](PSOKey& key) { return CompileOutline(key); };
	shaderRegistry_[RenderModelType::OutlinePrewittDepth] = [this](PSOKey& key) { return CompilePostProcessShader("OutlinePrewittDepth", key); };
}

ShaderPair ShaderFactory::MakeShaderBlob(PSOKey& key) {

	/// PSOKeyのRenderModelTypeからShaderを作る関数を呼び出す
	auto it = shaderRegistry_.find(key.renderModelType);
	if (it != shaderRegistry_.end()) {
		return it->second(key);
	} else {
		/// ここに止まるのはだいたい登録忘れ
		Logger::Log("[kEngine]ShaderFactory::MakeShaderBlob() RenderModelType not found in shaderRegistry_");
		assert(false);
	}

	return ShaderPair();
}


ShaderPair ShaderFactory::CompileShaderByFileName(const std::string& shaderName, PSOKey& key) {

	ShaderPair shaderPair;
	shaderPair.vs = shader_compile_->CompileShader(ConvertString::SwitchStdStringWstring(shaderFolder_ + shaderName + ".VS.hlsl"), L"vs_6_0",key);
	shaderPair.ps = shader_compile_->CompileShader(ConvertString::SwitchStdStringWstring(shaderFolder_ + shaderName + ".PS.hlsl"), L"ps_6_0",key);

	checkCompileResult(shaderPair);
	return shaderPair;
}

ShaderPair ShaderFactory::CompilePostProcessShader(const std::string& shaderName, PSOKey& key) {

	ShaderPair shaderPair;
	std::string postProcessFolder = "PostProcess/";
	shaderPair.vs = shader_compile_->CompileShader(ConvertString::SwitchStdStringWstring(shaderFolder_ + postProcessFolder + "FullScreenQuad.VS.hlsl"), L"vs_6_0", key);
	shaderPair.ps = shader_compile_->CompileShader(ConvertString::SwitchStdStringWstring(shaderFolder_ + postProcessFolder + shaderName + ".PS.hlsl"), L"ps_6_0", key);

	checkCompileResult(shaderPair);
	return shaderPair;
}

ShaderPair ShaderFactory::Compile2DShader(PSOKey& key) {
	return CompileShaderByFileName("Tile2D", key);
}

ShaderPair ShaderFactory::Compile3DShader(PSOKey& key) {
	ShaderPair shaderPair;

	/// 3DモデルにskinningがあるかどうかでShaderを分ける
	if (key.renderModelType == RenderModelType::Skinned) {
		shaderPair.vs = shader_compile_->CompileShader(ConvertString::SwitchStdStringWstring(shaderFolder_ + "SkinningObject3D.VS.hlsl"), L"vs_6_0", key);
	} else {
		shaderPair.vs = shader_compile_->CompileShader(ConvertString::SwitchStdStringWstring(shaderFolder_ + "Particle.VS.hlsl"), L"vs_6_0",key);
	}

	/// FeatureFlagsを摘出
	bool environmentReflectionFlag = (key.featureMask & (uint64_t)FeatureFlags::EnvReflection) != 0;

	/// 3DモデルのShaderは、LightModelTypeによって分ける
	shaderPair.ps = shader_compile_->CompileShader(ConvertString::SwitchStdStringWstring(shaderFolder_ + "Particle.PS.hlsl"), L"ps_6_0", key);

	checkCompileResult(shaderPair);
	return shaderPair;
}

/// =============================== Checker =============================== ///

ShaderPair ShaderFactory::CompileScreenQuad(PSOKey& key) {
	return CompilePostProcessShader("CopyImage", key);
}

ShaderPair ShaderFactory::CompileColorGuard(PSOKey& key) {
	return CompilePostProcessShader("ColorGuard", key);
}

ShaderPair ShaderFactory::CompileVignetting(PSOKey& key) {
	return CompilePostProcessShader("Vignette", key);
}

ShaderPair ShaderFactory::CompileBlur(PSOKey& key) {
	return CompilePostProcessShader("Blur", key);
}

ShaderPair ShaderFactory::CompileOutline(PSOKey& key) {
	return CompilePostProcessShader("Outline", key);
}
	
/// =============================== Checker =============================== ///

void ShaderFactory::checkCompileResult(ShaderPair shaderPair) {

	/// ここに止まるのは、ShaderのCompileに失敗していること
	/// だいたいShaderファイルのパスが間違っているか、Shaderコードにエラーがあるかのどちらか

	/// シェーダーのコンパイル結果をチェック
	assert(shaderPair.vs.Get());
	assert(shaderPair.ps.Get());
}
