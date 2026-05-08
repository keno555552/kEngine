#include "ShaderFactory.h"
#include "Logger.h"
#include <cassert>

ShaderFactory::ShaderFactory() {
	shader_compile_ = std::make_unique<Shader_compile>();
	shader_compile_->Initialize();

	shaderFolder_ = "./kEngine/EngineAssets/Shader/";
	shaderRegistry_[RenderModelType::Sprite2D] = [this](PSOKey& key) { return Compile2DShader(); };
	shaderRegistry_[RenderModelType::Static] = [this](PSOKey& key) { return Compile3DShader(key); };
	shaderRegistry_[RenderModelType::Skinned] = [this](PSOKey& key) { return Compile3DShader(key); };
	shaderRegistry_[RenderModelType::PARTICLEENVREFLECTION] = [this](PSOKey& key) { return Compile3DShader(key,true); };
	shaderRegistry_[RenderModelType::DebugLine] = [this](PSOKey& key) { return CompileDebugLineShader(); };
	shaderRegistry_[RenderModelType::SkyCube] = [this](PSOKey& key) { return CompileSkyCubeShader(); };
	shaderRegistry_[RenderModelType::FlameNeonGlow] = [this](PSOKey& key) { return CompileFlameNeonGlowShader(); };
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


ShaderPair ShaderFactory::CompileShader(const std::string& shaderName) {

	ShaderPair shaderPair;
	shaderPair.vs = shader_compile_->CompileShader(ConvertString::SwitchStdStringWstring(shaderFolder_ + shaderName + ".VS.hlsl"), L"vs_6_0");
	shaderPair.ps = shader_compile_->CompileShader(ConvertString::SwitchStdStringWstring(shaderFolder_ + shaderName + ".PS.hlsl"), L"ps_6_0");

	checkCompileResult(shaderPair);
	return shaderPair;
}

ShaderPair ShaderFactory::Compile2DShader() {
	return CompileShader("Tile2D");
}

ShaderPair ShaderFactory::Compile3DShader(PSOKey& psoKeys,bool isEnvironmentReflection) {
	ShaderPair shaderPair;

	/// 3DモデルにskinningがあるかどうかでShaderを分ける
	if (psoKeys.renderModelType == RenderModelType::Skinned) {
		shaderPair.vs = shader_compile_->CompileShader(ConvertString::SwitchStdStringWstring(shaderFolder_ + "SkinningObject3D.VS.hlsl"), L"vs_6_0");
	} else {
		shaderPair.vs = shader_compile_->CompileShader(ConvertString::SwitchStdStringWstring(shaderFolder_ + "Particle.VS.hlsl"), L"vs_6_0");
	}

	shaderPair.ps = shader_compile_->CompileShader(ConvertString::SwitchStdStringWstring(shaderFolder_ + "Particle.PS.hlsl"), L"ps_6_0", psoKeys.lightModelType, isEnvironmentReflection);

	checkCompileResult(shaderPair);
	return shaderPair;
}

ShaderPair ShaderFactory::CompileDebugLineShader() {
	return CompileShader("DebugLine");
}

ShaderPair ShaderFactory::CompileSkyCubeShader() {
	return CompileShader("SkyCube");;
}

ShaderPair ShaderFactory::CompileFlameNeonGlowShader() {
	return CompileShader("FlameNeonGlow");
}

void ShaderFactory::checkCompileResult(ShaderPair shaderPair) {

	/// ここに止まるのは、ShaderのCompileに失敗していること
	/// だいたいShaderファイルのパスが間違っているか、Shaderコードにエラーがあるかのどちらか

	/// シェーダーのコンパイル結果をチェック
	assert(shaderPair.vs.Get());
	assert(shaderPair.ps.Get());
}
