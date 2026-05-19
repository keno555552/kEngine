#pragma once
#include <memory>
#include <unordered_map>
#include <functional>

///DirectX12
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#include <dxgidebug.h>
#pragma comment(lib,"dxguid.lib")
#include <wrl/client.h>
#include <dxcapi.h>

#include "ShaderManager/shader_compile.h"
#include "PSOManager/PSOKeys.h"

struct ShaderPair {
    Microsoft::WRL::ComPtr<IDxcBlob> vs;
    Microsoft::WRL::ComPtr<IDxcBlob> ps;
};

class ShaderFactory{
public:

	ShaderFactory();

    ShaderPair MakeShaderBlob(PSOKey& key);

private:

	/// ShaderFilePath
	std::string shaderFolder_;

	/// ShaderCompileクラス
	std::unique_ptr<Shader_compile> shader_compile_;

	/// シェーダーコンパイルクラス
	std::unordered_map<RenderModelType, std::function<ShaderPair(PSOKey&)>> shaderRegistry_;

private:

	/// 他の関数に呼び出す用のShaderCompile関数
	ShaderPair CompileShader(const std::string& shaderName, PSOKey& key);

	/// PSOKeyのRenderModelTypeごとにShaderをコンパイルする関数
	ShaderPair Compile2DShader(PSOKey& psoKeys);
	ShaderPair Compile3DShader(PSOKey& psoKeys);
	ShaderPair CompileDebugLineShader(PSOKey& psoKeys);
	ShaderPair CompileSkyCubeShader(PSOKey& psoKeys);
	ShaderPair CompileFlameNeonGlowShader(PSOKey& psoKeys);
	ShaderPair CompileParticleScreenQuad(PSOKey& psoKeys);

	void checkCompileResult(ShaderPair shaderPair);
    
};