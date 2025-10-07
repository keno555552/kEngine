#include "PSO.h"
#include "Vector4.h"


PSO::~PSO() {
	//directXDriver_->GetDriver()->Release(); /*借り*/
	signatureBlob_->Release();
	if (errorBlob_) {
		errorBlob_->Release();
	}
	rootSignature_->Release();
	vertexShaderBlob_->Release();
	pixelShaderBlob_->Release();
	rootSignatureList_.clear();
	//graphicsPipelineState_->Release();
}

void PSO::Initialize(DirectXBase* directXDriver) {
	directXDriver_ = directXDriver;
}

ID3D12PipelineState* PSO::createPSO(IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler,
	LightModelType lightModelType) {
	createRootSignature(false);
	createInputLayout();
	SetBlendState();
	SetRasterizerState();
	ShaderCompile(dxcUtils, dxcCompiler, includeHandler, lightModelType);
	SetDepthStencilState();

	SetGraphicsPipelineState();
	return graphicsPipelineState_;
}

ID3D12PipelineState* PSO::createPSO_Tile(IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler) {

	createRootSignature(true);
	createInputLayout();
	SetBlendState();
	SetRasterizerState();
	ShaderCompile_Tile(dxcUtils, dxcCompiler, includeHandler);
	SetDepthStencilState();

	SetGraphicsPipelineState();
	return graphicsPipelineState_;
}

ID3D12RootSignature* PSO::createRootSignature(bool isParticle) {
	///RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	/// DescriptorRange作成。SRVを使う
	D3D12_DESCRIPTOR_RANGE descriptorRange[1]{};
	descriptorRange[0].BaseShaderRegister = 0; // 0から始まる
	descriptorRange[0].NumDescriptors = 1; // 数は1つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // 連続している

	D3D12_STATIC_SAMPLER_DESC staticSampler[1]{};
	staticSampler[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;          //バイリニアフィルタ
	staticSampler[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;        // 0~1の範囲外をリピート
	staticSampler[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;      // 比較しない
	staticSampler[0].MaxLOD = D3D12_FLOAT32_MAX;                        // ありったけのMipmapを使う
	staticSampler[0].ShaderRegister = 0;                                // レジスタ0番を使う
	staticSampler[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;  // PixelShaderで使う
	descriptionRootSignature.pStaticSamplers = staticSampler;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSampler);

	/// RootParameter作成。PixelShaderのMaterialとVertexShaderのTransform
	D3D12_ROOT_PARAMETER rootParameters[4] = {};                                                    ///4になった

	// Material用
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;                                // CBV を使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;                             // PixelShader で使う
	rootParameters[0].Descriptor.ShaderRegister = 0;                                                // レジスタ番号 0 とバインド

	// Transform用
	if (!isParticle) {
		rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;								/// 
		rootParameters[1].Descriptor.ShaderRegister = 0;													/// 
		rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;									///
	} else {
		static D3D12_DESCRIPTOR_RANGE descriptorRangeForInstancing[1]{};
		descriptorRangeForInstancing[0].BaseShaderRegister = 0;
		descriptorRangeForInstancing[0].NumDescriptors = 1;
		descriptorRangeForInstancing[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descriptorRangeForInstancing[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	
		rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;						/// DescirptorTableを使う
		rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;								/// VertexShaderで使う
		rootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRangeForInstancing;					/// Tableの中身の配列を指定
		rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForInstancing);		/// Tableで利用する数
	}

	// Texture用
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;                   /// DescriptorTableを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;                             /// PixelShaderで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;                          /// Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);              /// Tableで利用する数

	// Lighting用
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;                                /// CBV を使う
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;                             /// PixelShaderで使う
	rootParameters[3].Descriptor.ShaderRegister = 1;


	descriptionRootSignature.pParameters = rootParameters;              // ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);  // 配列の長さ


	// シリアライズしてバイナリにする
	signatureBlob_ = nullptr;
	errorBlob_ = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob_, &errorBlob_);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob_->GetBufferPointer()));
		assert(false);
	}
	// バイナリを元に生成
	rootSignature_ = nullptr;
	hr = directXDriver_->GetDriver()->CreateRootSignature(0, signatureBlob_->GetBufferPointer(), signatureBlob_->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));

	rootSignatureList_.push_back(rootSignature_);
	return rootSignature_;
}

void PSO::createInputLayout() {
	///InputLayout
	//D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs_[0] = {};
	inputElementDescs_[0].SemanticName = "POSITION";
	inputElementDescs_[0].SemanticIndex = 0;
	inputElementDescs_[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs_[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs_[1] = {};
	inputElementDescs_[1].SemanticName = "TEXCOORD";
	inputElementDescs_[1].SemanticIndex = 0;
	inputElementDescs_[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs_[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs_[2] = {};
	inputElementDescs_[2].SemanticName = "NORMAL";
	inputElementDescs_[2].SemanticIndex = 0;
	inputElementDescs_[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs_[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputLayoutDesc_ = {};
	inputLayoutDesc_.pInputElementDescs = inputElementDescs_;
	inputLayoutDesc_.NumElements = _countof(inputElementDescs_);
}

void PSO::SetBlendState() {
	// BlendStateの設定
	//D3D12_BLEND_DESC blendDesc{};
	blendDesc_ = {};
	// すべての色要素を書き込む
	blendDesc_.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;
}

void PSO::SetRasterizerState() {
	// RasiterzerStateの設定
	//D3D12_RASTERIZER_DESC rasterizerDesc{};
	rasterizerDesc_ = {};
	/// 裏面（時計回り）を表示しない
	rasterizerDesc_.CullMode = D3D12_CULL_MODE_BACK;
	/// 三角形の中を塗りつぶす
	rasterizerDesc_.FillMode = D3D12_FILL_MODE_SOLID;

}

void PSO::ShaderCompile(IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler, LightModelType lightModelType) {
	/// Shaderをコンパイルする
	vertexShaderBlob_ = {};
	vertexShaderBlob_ = CompileShader(L"./resources/Shader/Object3d.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(vertexShaderBlob_ != nullptr);

	pixelShaderBlob_ = {};
	pixelShaderBlob_ = CompileShader(L"./resources/Shader/Object3d.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler, lightModelType);
	assert(pixelShaderBlob_ != nullptr);
}

void PSO::ShaderCompile_Tile(IDxcUtils* dxcUtils, IDxcCompiler3* dxcCompiler, IDxcIncludeHandler* includeHandler) {
	/// Shaderをコンパイルする
	vertexShaderBlob_ = {};
	vertexShaderBlob_ = CompileShader(L"./resources/Shader/Tile2D.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(vertexShaderBlob_ != nullptr);

	pixelShaderBlob_ = {};
	pixelShaderBlob_ = CompileShader(L"./resources/Shader/Tile2D.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(pixelShaderBlob_ != nullptr);
}

void PSO::SetDepthStencilState() {
	// DepthStencilStateの設定
	depthStencilDesc = {};
	// Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	// 書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	// 比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
}

void PSO::SetGraphicsPipelineState() {
	// graphicsPipelineStateに設定する情報をまとめる
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature_; // RootSignature
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc_; // InputLayout
	graphicsPipelineStateDesc.VS = { vertexShaderBlob_->GetBufferPointer(), vertexShaderBlob_->GetBufferSize() }; // VertexShader
	graphicsPipelineStateDesc.PS = { pixelShaderBlob_->GetBufferPointer(), pixelShaderBlob_->GetBufferSize() }; // PixelShader
	graphicsPipelineStateDesc.BlendState = blendDesc_; // BlendState
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc_; // RasterizerState
	// 書き込むRTVの情報
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ（形状）のタイプ。三角形
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定（気にしなくて良い）
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// DepthStencilの設定
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	// 実際に生成
	graphicsPipelineState_ = nullptr;
	HRESULT hr = directXDriver_->GetDriver()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState_));
	assert(SUCCEEDED(hr));
}


