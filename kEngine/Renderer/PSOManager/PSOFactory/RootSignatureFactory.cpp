#include "RootSignatureFactory.h"
#include "Logger.h"

#include "Data/Render/GPUData/RenderCommandGPU.h"
#include "Data/Render/GPUData/BlurDataGPU.h"

RootSignatureFactory::RootSignatureFactory() {

	rootSignatureRegistry[RenderModelType::Sprite2D] = [this](DirectXCore* directXDriver_, PSOKey& key) { return MakeStatic(directXDriver_, key); };
	rootSignatureRegistry[RenderModelType::Static] = [this](DirectXCore* directXDriver_, PSOKey& key) { return MakeStatic(directXDriver_, key); };
	rootSignatureRegistry[RenderModelType::Skinned] = [this](DirectXCore* directXDriver_, PSOKey& key) { return MakeStaticSkinning(directXDriver_, key); };
	rootSignatureRegistry[RenderModelType::DebugLine] = [this](DirectXCore* directXDriver_, PSOKey& key) { return MakeStatic(directXDriver_, key); };
	rootSignatureRegistry[RenderModelType::Environment] = [this](DirectXCore* directXDriver_, PSOKey& key) { return MakeStatic(directXDriver_, key); };
	rootSignatureRegistry[RenderModelType::FlameNeonGlow] = [this](DirectXCore* directXDriver_, PSOKey& key) { return MakeStatic(directXDriver_, key); };

	/// PostProcess用の描画モデル
	rootSignatureRegistry[RenderModelType::FullscreenQuad] = [this](DirectXCore* directXDriver_, PSOKey& key) { return MakeStaticFullscreenQuad(directXDriver_, key); };
	rootSignatureRegistry[RenderModelType::ColorGradient] = [this](DirectXCore* directXDriver_, PSOKey& key) { return MakeStaticFullscreenQuad(directXDriver_, key); };
	rootSignatureRegistry[RenderModelType::Vignette] = [this](DirectXCore* directXDriver_, PSOKey& key) { return MakeStaticFullscreenQuad(directXDriver_, key); };
	rootSignatureRegistry[RenderModelType::Blur] = [this](DirectXCore* directXDriver_, PSOKey& key) { return MakeStaticFullscreenQuad(directXDriver_, key); };
	rootSignatureRegistry[RenderModelType::Outline] = [this](DirectXCore* directXDriver_, PSOKey& key) { return MakeStaticFullscreenQuad(directXDriver_, key); };
	rootSignatureRegistry[RenderModelType::OutlinePrewittDepth] = [this](DirectXCore* directXDriver_, PSOKey& key) { return MakeStaticFullscreenQuad(directXDriver_, key); };
	rootSignatureRegistry[RenderModelType::Dissolve] = [this](DirectXCore* directXDriver_, PSOKey& key) { return MakeStaticFullscreenQuad(directXDriver_, key); };
	rootSignatureRegistry[RenderModelType::Noise] = [this](DirectXCore* directXDriver_, PSOKey& key) { return MakeStaticFullscreenQuad(directXDriver_, key); };
}

Microsoft::WRL::ComPtr <ID3D12RootSignature> RootSignatureFactory::Make(PSOKey& key, DirectXCore* directXDriver_) {

	auto checker = rootSignatureMap_.find(key.renderModelType);
	if (checker != rootSignatureMap_.end()) {
		return checker->second;
	}

	auto it = rootSignatureRegistry.find(key.renderModelType);
	if (it == rootSignatureRegistry.end()) {

		/// ここに止まるのはだいたい登録忘れ
		Logger::Log("[kEngine]RootSignatureFactory::Make() RenderModelType not found in rootSignatureRegistry");
		assert(false);

	} else {
		auto rootSignature = it->second(directXDriver_, key);
		rootSignatureMap_[key.renderModelType] = rootSignature;
		return rootSignature;
	}

	return nullptr;
}

Microsoft::WRL::ComPtr <ID3D12RootSignature> RootSignatureFactory::MakeStatic(DirectXCore* directXDriver_, PSOKey& key) {

	/// b0:InstanceOffset			(VS)
	/// t0:TransformMatrices		(VS)
	/// t1:MaterialIndexList		(VS)
	///=================================
	/// b0:Camera					(PS)
	/// b1:LightingCount			(PS)
	///---------------------------------
	/// t0:Texture					(PS)
	/// t1:MaterialList				(PS)
	/// t2:LightList				(PS)
	/// t3:ENVIRONMENT REFLECTION	(PS)
	///---------------------------------
	/// s0:Sampler					(PS)

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

	/// ============================================ RootParameter作成 =========================================== ///
	D3D12_ROOT_PARAMETER rootParameters[9] = {};

	///// Material用 (大改，最後改)
	//rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;									/// CBV を使う
	//rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;									/// PixelShader で使う
	//rootParameters[0].Descriptor.ShaderRegister = 0;													/// レジスタ番号 0 とバインド

	// MaterialList 用 (StructuredBuffer<MaterialGroup> gMaterialList : t1)
	static D3D12_DESCRIPTOR_RANGE materialListRange[1]{};
	materialListRange[0].BaseShaderRegister = 1;														/// t1
	materialListRange[0].NumDescriptors = 1;
	materialListRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	materialListRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].DescriptorTable.pDescriptorRanges = materialListRange;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = _countof(materialListRange);


	// TransformMatrices（t0, VertexShader）
	static D3D12_DESCRIPTOR_RANGE descriptorRangeForInstancing[1]{};
	descriptorRangeForInstancing[0].BaseShaderRegister = 0;												/// t0
	descriptorRangeForInstancing[0].NumDescriptors = 1;
	descriptorRangeForInstancing[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRangeForInstancing[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;						/// DescriptorTableを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;								/// VertexShaderで使う
	rootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRangeForInstancing;					/// Tableの中身の配列を指定
	rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForInstancing);		/// Tableで利用する数

	// Texture用 (t0)
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;						/// DescriptorTableを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;									/// PixelShaderで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;								/// Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);					/// Tableで利用する数

	// LightingCount (b1, GlowSphereに使える)
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameters[3].Constants.Num32BitValues = 1;
	rootParameters[3].Constants.RegisterSpace = 0;
	rootParameters[3].Constants.ShaderRegister = 1;														/// b1

	// slot 4: InstanceOffset (b0, VertexShader)
	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[4].Descriptor.ShaderRegister = 0;													/// b0

	// Camera 用 (b0)
	rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;									/// 或 ALL
	rootParameters[5].Descriptor.ShaderRegister = 0;													/// b0

	// LightList 用 (StructuredBuffer<LightGPU> gLights : t2)
	static D3D12_DESCRIPTOR_RANGE lightListRange[1]{};
	lightListRange[0].BaseShaderRegister = 2;															/// t2
	lightListRange[0].NumDescriptors = 1;
	lightListRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	lightListRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[6].DescriptorTable.pDescriptorRanges = lightListRange;
	rootParameters[6].DescriptorTable.NumDescriptorRanges = _countof(lightListRange);

	// EnvironmentReflection 用 (t3)
	static D3D12_DESCRIPTOR_RANGE environmentReflectionRange[1]{};
	environmentReflectionRange[0].BaseShaderRegister = 3;												/// t3
	environmentReflectionRange[0].NumDescriptors = 1;
	environmentReflectionRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	environmentReflectionRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[7].DescriptorTable.NumDescriptorRanges = _countof(environmentReflectionRange);
	rootParameters[7].DescriptorTable.pDescriptorRanges = environmentReflectionRange;

	// MaterialIndexList（t1, VertexShader）
	static D3D12_DESCRIPTOR_RANGE descriptorRangeForMaterialIndex[1]{};
	descriptorRangeForMaterialIndex[0].BaseShaderRegister = 1;											/// t1
	descriptorRangeForMaterialIndex[0].NumDescriptors = 1;
	descriptorRangeForMaterialIndex[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRangeForMaterialIndex[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;						/// DescriptorTableを使う
	rootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;								/// VertexShaderで使う
	rootParameters[8].DescriptorTable.pDescriptorRanges = descriptorRangeForMaterialIndex;					/// Tableの中身の配列を指定
	rootParameters[8].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForMaterialIndex);		/// Tableで利用する数


	descriptionRootSignature.pParameters = rootParameters;              // ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);  // 配列の長さ


	// シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob_;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob_;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1,
		signatureBlob_.GetAddressOf(),
		errorBlob_.GetAddressOf());
	if (FAILED(hr)) {
		if (errorBlob_) {
			Logger::Log(reinterpret_cast<char*>(errorBlob_->GetBufferPointer()));
		}
		assert(false);
	}
	// バイナリを元に生成
	Microsoft::WRL::ComPtr <ID3D12RootSignature> rootSignature;
	hr = directXDriver_->GetDevice()->CreateRootSignature(0,
		signatureBlob_->GetBufferPointer(),
		signatureBlob_->GetBufferSize(),
		IID_PPV_ARGS(rootSignature.GetAddressOf()));
	assert(SUCCEEDED(hr));

	return rootSignature;
}

Microsoft::WRL::ComPtr <ID3D12RootSignature> RootSignatureFactory::MakeStaticSkinning(DirectXCore* directXDriver_, PSOKey& key) {

	/// b0:InstanceOffset			(VS)
	/// t0:TransformMatrices		(VS)
	/// t1:MaterialIndexList		(VS)
	///---------------------------------
	/// t1:BoneMatrices				(VS)
	///=================================
	/// b0:Camera					(PS)
	/// b1:LightingCount			(PS)
	///---------------------------------
	/// t0:Texture					(PS)
	/// t1:MaterialList				(PS)
	/// t2:LightList				(PS)
	/// t3:ENVIRONMENT REFLECTION	(PS)
	///---------------------------------
	/// s0:Sampler					(PS)


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

	/// ============================================ RootParameter作成 =========================================== ///
	D3D12_ROOT_PARAMETER rootParameters[10] = {};

	///// Material用 (大改，最後改)
	//rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;									/// CBV を使う
	//rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;									/// PixelShader で使う
	//rootParameters[0].Descriptor.ShaderRegister = 0;													/// レジスタ番号 0 とバインド

	// MaterialList 用 (StructuredBuffer<MaterialGroup> gMaterialList : t1)
	static D3D12_DESCRIPTOR_RANGE materialListRange[1]{};
	materialListRange[0].BaseShaderRegister = 1;														/// t1
	materialListRange[0].NumDescriptors = 1;
	materialListRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	materialListRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].DescriptorTable.pDescriptorRanges = materialListRange;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = _countof(materialListRange);


	// TransformMatrices（t0, VertexShader）
	static D3D12_DESCRIPTOR_RANGE descriptorRangeForInstancing[1]{};
	descriptorRangeForInstancing[0].BaseShaderRegister = 0;												/// t0
	descriptorRangeForInstancing[0].NumDescriptors = 1;
	descriptorRangeForInstancing[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRangeForInstancing[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;						/// DescriptorTableを使う
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;								/// VertexShaderで使う
	rootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRangeForInstancing;					/// Tableの中身の配列を指定
	rootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForInstancing);		/// Tableで利用する数

	// Texture用 (t0)
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;						/// DescriptorTableを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;									/// PixelShaderで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;								/// Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);					/// Tableで利用する数

	// LightingCount (b1, GlowSphereに使える)
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameters[3].Constants.Num32BitValues = 1;
	rootParameters[3].Constants.RegisterSpace = 0;
	rootParameters[3].Constants.ShaderRegister = 1;														/// b1

	// slot 4: InstanceOffset (b0, VertexShader)
	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[4].Descriptor.ShaderRegister = 0;

	// Camera 用 (b0)
	rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;									/// 或 ALL
	rootParameters[5].Descriptor.ShaderRegister = 0;													/// b0

	// LightList 用 (StructuredBuffer<LightGPU> gLights : t2)
	static D3D12_DESCRIPTOR_RANGE lightListRange[1]{};
	lightListRange[0].BaseShaderRegister = 2;															/// t2
	lightListRange[0].NumDescriptors = 1;
	lightListRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	lightListRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[6].DescriptorTable.pDescriptorRanges = lightListRange;
	rootParameters[6].DescriptorTable.NumDescriptorRanges = _countof(lightListRange);

	// EnvironmentReflection 用 (t3)
	static D3D12_DESCRIPTOR_RANGE environmentReflectionRange[1]{};
	environmentReflectionRange[0].BaseShaderRegister = 3;												/// t3
	environmentReflectionRange[0].NumDescriptors = 1;
	environmentReflectionRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	environmentReflectionRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[7].DescriptorTable.pDescriptorRanges = environmentReflectionRange;
	rootParameters[7].DescriptorTable.NumDescriptorRanges = _countof(environmentReflectionRange);

	// MaterialIndexList（t1, VertexShader）
	static D3D12_DESCRIPTOR_RANGE descriptorRangeForMaterialIndex[1]{};
	descriptorRangeForMaterialIndex[0].BaseShaderRegister = 1;											/// t1
	descriptorRangeForMaterialIndex[0].NumDescriptors = 1;
	descriptorRangeForMaterialIndex[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRangeForMaterialIndex[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;						/// DescriptorTableを使う
	rootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;								/// VertexShaderで使う
	rootParameters[8].DescriptorTable.pDescriptorRanges = descriptorRangeForMaterialIndex;					/// Tableの中身の配列を指定
	rootParameters[8].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForMaterialIndex);		/// Tableで利用する数

	/// =========================================== skinning専用スロット ========================================== ///

	// SkinningのWell (t2, VS)
	static D3D12_DESCRIPTOR_RANGE wellRange[1]{};
	wellRange[0].BaseShaderRegister = 2;																/// t2
	wellRange[0].NumDescriptors = 1;
	wellRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	wellRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[9].DescriptorTable.pDescriptorRanges = wellRange;
	rootParameters[9].DescriptorTable.NumDescriptorRanges = _countof(wellRange);


	descriptionRootSignature.pParameters = rootParameters;              // ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);  // 配列の長さ


	// シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob_;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob_;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1,
		signatureBlob_.GetAddressOf(),
		errorBlob_.GetAddressOf());
	if (FAILED(hr)) {
		if (errorBlob_) {
			Logger::Log(reinterpret_cast<char*>(errorBlob_->GetBufferPointer()));
		}
		assert(false);
	}
	// バイナリを元に生成
	Microsoft::WRL::ComPtr <ID3D12RootSignature> rootSignature;
	hr = directXDriver_->GetDevice()->CreateRootSignature(0,
		signatureBlob_->GetBufferPointer(),
		signatureBlob_->GetBufferSize(),
		IID_PPV_ARGS(rootSignature.GetAddressOf()));
	assert(SUCCEEDED(hr));

	return rootSignature;
}

Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignatureFactory::MakeStaticFullscreenQuad(DirectXCore* directXDriver_, PSOKey& key) {
	key;	/// 使われなかった、将来の拡張のために残しておく
	
	/// s0:Sampler 				(PS)
	/// s1:DepthSampler			(PS)
	/// t0:SourceTexture		(PS)
	/// t2:DepthTexture			(PS)
	/// b0:RenderCommand		(PS)
	/// t1:KernelData			(PS)
	/// t3:Texture2用 			(PS)

	///RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	/// -------------------------------- Static Sampler (s0) -------------------------------- ///

	// StaticSampler (s0, PixelShader)
	D3D12_STATIC_SAMPLER_DESC staticSampler[2]{};
	staticSampler[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;          //バイリニアフィルタ
	staticSampler[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;        // 0~1の範囲外をリピート
	staticSampler[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;      // 比較しない
	staticSampler[0].MaxLOD = D3D12_FLOAT32_MAX;                        // ありったけのMipmapを使う
	staticSampler[0].ShaderRegister = 0;                                // レジスタ0番を使う
	staticSampler[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;  // PixelShaderで使う
	// StaticSampler (s1, PixelShader)
	staticSampler[1].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;          //ポイントフィルタ
	staticSampler[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;        // 0~1の範囲外をリピート
	staticSampler[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSampler[1].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;      // 比較しない
	staticSampler[1].MaxLOD = D3D12_FLOAT32_MAX;                        // ありったけのMipmapを使う
	staticSampler[1].ShaderRegister = 1;                                // レジスタ1番を使う
	staticSampler[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;  // PixelShaderで使う
	descriptionRootSignature.pStaticSamplers = staticSampler;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSampler);

	/// RootParameter作成。PixelShaderのMaterialとVertexShaderのTransform
	/// ------------------------------- Descriptor Range (t0) ------------------------------- ///
	D3D12_ROOT_PARAMETER rootParameters[5] = {};

	// SourceTexture用descriptor
	static D3D12_DESCRIPTOR_RANGE descriptorRangeForInstancing[1]{};
	descriptorRangeForInstancing[0].BaseShaderRegister = 0;
	descriptorRangeForInstancing[0].NumDescriptors = 1;
	descriptorRangeForInstancing[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRangeForInstancing[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// SourceTexture（t0, PixelShader）
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;						/// DescriptorTableを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;									/// PixelShaderで使う
	rootParameters[0].DescriptorTable.pDescriptorRanges = descriptorRangeForInstancing;					/// Tableの中身の配列を指定
	rootParameters[0].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForInstancing);		/// Tableで利用する数

	// RenderCommand（b0, PixelShader）
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	//rootParameters[1].Constants.Num32BitValues = 64 / 4; // 最大 64 bytes
	rootParameters[1].Constants.Num32BitValues = 160 / 4; // 最大 64 bytes
	rootParameters[1].Constants.ShaderRegister = 0; // b0
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

	// kernelDataList 用 (t1, PS)
	static D3D12_DESCRIPTOR_RANGE kernelDataListRange[1]{};
	kernelDataListRange[0].BaseShaderRegister = 1; // t1
	kernelDataListRange[0].NumDescriptors = 1;
	kernelDataListRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	kernelDataListRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = kernelDataListRange;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(kernelDataListRange);

	// DepthTexture用 (t2, PS)
	static D3D12_DESCRIPTOR_RANGE descriptorRangeForDepthTexture[1]{};
	descriptorRangeForDepthTexture[0].BaseShaderRegister = 2; // t2
	descriptorRangeForDepthTexture[0].NumDescriptors = 1;
	descriptorRangeForDepthTexture[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRangeForDepthTexture[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[3].DescriptorTable.pDescriptorRanges = descriptorRangeForDepthTexture;
	rootParameters[3].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForDepthTexture);

	// Texture2用 (t3, PS)
	D3D12_DESCRIPTOR_RANGE texture2Range{};
	texture2Range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	texture2Range.NumDescriptors = 1;
	texture2Range.BaseShaderRegister = 3; // t3
	texture2Range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[4].DescriptorTable.pDescriptorRanges = &texture2Range;
	rootParameters[4].DescriptorTable.NumDescriptorRanges = 1;

	descriptionRootSignature.pParameters = rootParameters;              // ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters);  // 配列の長さ

	// シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob_;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob_;
	HRESULT hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1,
		signatureBlob_.GetAddressOf(),
		errorBlob_.GetAddressOf());
	if (FAILED(hr)) {
		if (errorBlob_) {
			Logger::Log(reinterpret_cast<char*>(errorBlob_->GetBufferPointer()));
		}
		assert(false);
	}
	// バイナリを元に生成
	Microsoft::WRL::ComPtr <ID3D12RootSignature> rootSignature;
	hr = directXDriver_->GetDevice()->CreateRootSignature(0,
		signatureBlob_->GetBufferPointer(),
		signatureBlob_->GetBufferSize(),
		IID_PPV_ARGS(rootSignature.GetAddressOf()));
	assert(SUCCEEDED(hr));

	return rootSignature;
}
