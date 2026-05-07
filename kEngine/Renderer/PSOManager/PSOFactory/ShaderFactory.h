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

	ShaderPair CompileShader(const std::string& shaderName);
	ShaderPair Compile2DShader();
	ShaderPair Compile3DShader(PSOKey& psoKeys);
	ShaderPair CompileDebugLineShader();
	ShaderPair CompileSkyCubeShader();
	ShaderPair CompileFlameNeonGlowShader();

	void checkCompileResult(ShaderPair shaderPair);
    
};