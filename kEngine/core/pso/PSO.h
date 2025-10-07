#pragma once

///DirectX12
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#include <dxgidebug.h>
#pragma comment(lib,"dxguid.lib")

#include <dxcapi.h>
#pragma comment(lib,"dxcompiler.lib")

#include "DircetXBase.h"
#include "config.h"
#include <vector>

class PSO{
public:
	~PSO();

	void Initialize(DirectXBase* directXDriver);

	ID3D12RootSignature* getRootSignature(int handle) { return rootSignatureList_[handle]; }

	ID3D12RootSignature* createRootSignature(bool isParticle);

	ID3D12PipelineState* createPSO(IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler, 
								   LightModelType lightMadelType);

	ID3D12PipelineState* createPSO_Tile(IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler);

private:
	//D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	DirectXBase* directXDriver_;
	ID3DBlob* signatureBlob_ = nullptr;
	ID3DBlob* errorBlob_ = nullptr;
	std::vector<ID3D12RootSignature*> rootSignatureList_;
	ID3D12RootSignature* rootSignature_;
	D3D12_INPUT_ELEMENT_DESC inputElementDescs_[3] = {};
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc_ = {};
	D3D12_BLEND_DESC blendDesc_ = {};
	D3D12_RASTERIZER_DESC rasterizerDesc_ = {};
	IDxcBlob* vertexShaderBlob_ = nullptr;
	IDxcBlob* pixelShaderBlob_ = nullptr;
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	ID3D12PipelineState* graphicsPipelineState_ = nullptr;

private:
	void createInputLayout();
	void SetBlendState();
	void SetRasterizerState();
	void ShaderCompile(IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler, LightModelType lightModelType);
	void ShaderCompile_Tile(IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler);
	void SetDepthStencilState();
	void SetGraphicsPipelineState();
	//ID3D12Resource* CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);
};