#include "PSOManager.h"

void PSOManager::Initialize(DirectXCore* directXDriver) {

	/// 
	psoFactory_.Initialize(directXDriver);
	directXDriver_ = directXDriver;
	commandList_ = directXDriver_->GetCommandList();
	
	/// PSOcacheのファイルの位置を保存
	psoCacheFilePath_ = config::GetPsoCacheFolderPath() + config::GetPsoCacheFolderName();
	psoRecorder_.Load(psoCacheFilePath_);

	///// PSOを全部作る
	//CreateAllPSO();
	/// PSOcacheからPSOを作る
	LasyCreatePSO();
}

void PSOManager::Finalize() {

	psoKeyToHandleMap_.clear();
	psoMap_.clear();

}

Microsoft::WRL::ComPtr<ID3D12PipelineState> PSOManager::GetPso(PSOKey keys) {

	/// PSOKeyからハンドルを探す
	int handle = GetPSOHandle(keys);
	if (handle == -1) {
		Logger::Log("[kEngine]PSOManager::GetPso() PSOKey not found");
		assert(false);
	}
	return psoMap_[handle];

}

void PSOManager::SetPSO(PSOKey keys) {

	/// PSOKeyからハンドルを探す
	int newPSOHandle = GetPSOHandle(keys);

	/// PSOがない場合PSO生成する
	if (newPSOHandle == -1) {
		Logger::Log("[kEngine]PSOManager::SetPso() pso not found, create Pso");

		// factoryで新しいPSO生成
		auto pso = psoFactory_.createPSO(keys);

		// PSOKeyとハンドルのマップに追加
		int handle = (int)psoMap_.size();
		psoMap_[handle] = pso;
		psoKeyToHandleMap_[keys] = handle;

		// 記録
		psoRecorder_.AppendPSOKeyRecord(keys);

		// PSO更新
		newPSOHandle = handle;
	}

	/// ある場合、今使ってるPSOと比較。違う場合更新する
	if (currentPSOHandle_ != newPSOHandle) {

		// PSOをセット
		commandList_->SetPipelineState(psoMap_[newPSOHandle].Get());
		currentPSOHandle_ = newPSOHandle;

		// RootSignatureをセット
		rootSignature_ = psoFactory_.getRootSignatureFactory()->Make(keys, directXDriver_).Get();
		commandList_->SetGraphicsRootSignature(rootSignature_);
	}
}


void PSOManager::SetPSOStrong(PSOKey keys) {

	/// PSOKeyからハンドルを探す
	int newPSOHandle = GetPSOHandle(keys);

	/// PSOがない場合PSO生成する
	if (newPSOHandle == -1) {
		Logger::Log("[kEngine]PSOManager::SetPSOStrong() pso not found, create Pso");

		// factoryで新しいPSO生成
		auto pso = psoFactory_.createPSO(keys);

		// PSOKeyとハンドルのマップに追加
		int handle = (int)psoMap_.size();
		psoMap_[handle] = pso;
		psoKeyToHandleMap_[keys] = handle;

		// 記録
		psoRecorder_.AppendPSOKeyRecord(keys);

		// PSO更新
		newPSOHandle = handle;
	}

	///　どうしても更新する
	
	// PSOをセット
	commandList_->SetPipelineState(psoMap_[newPSOHandle].Get());
	currentPSOHandle_ = newPSOHandle;

	// RootSignatureをセット
	rootSignature_ = psoFactory_.getRootSignatureFactory()->Make(keys, directXDriver_).Get();
	commandList_->SetGraphicsRootSignature(rootSignature_);
}

void PSOManager::LasyCreatePSO() {
	
	/// 読み取り
	psoRecorder_.Load(psoCacheFilePath_);
	
	/// PSOを作成する
	// まずはPSOKeyのリストを取る
	std::vector<PSOKey> keys = psoRecorder_.GetAllKeys();

	// 実際生成
	for(auto& key : keys) {
		auto pso = psoFactory_.createPSO(key);

		int handle = (int)psoMap_.size();
		psoMap_[handle] = pso;
		psoKeyToHandleMap_[key] = handle;
	}
}

void PSOManager::CreateAllPSO() {

	int handle = 0;
	for (int LightNB = 0; LightNB < (int)LightModelType::NumLightModels; LightNB++) {
		for (int RenderNB = 0; RenderNB < (int)RenderModelType::NumOfRenderModels; RenderNB++) {
			for (int RasterNB = 0; RasterNB < (int)RasterizerMode::NumRasterizerModes; RasterNB++) {
				for (int BlendNB = 0; BlendNB < (int)BlendModeType::NumOfBlends; BlendNB++) {
					for (int DepthStencilNB = 0; DepthStencilNB < (int)DepthStencilType::NumberOfDepthStencilStates; DepthStencilNB++) {
						for (int PrimitiveNB = 0; PrimitiveNB < (int)PrimitiveType::NumOfPrimitive; PrimitiveNB++) {
							if (PrimitiveNB == (int)PrimitiveType::UNDEFINED)continue;
							/// このループの組み合わせのPSOKeyを作る
							PSOKey key;
							key.lightModelType = (LightModelType)LightNB;
							key.renderModelType = (RenderModelType)RenderNB;
							key.rasterizerMode = (RasterizerMode)RasterNB;
							key.blendModeType = (BlendModeType)BlendNB;
							key.depthStencilType = (DepthStencilType)DepthStencilNB;
							key.primitiveType = (PrimitiveType)PrimitiveNB;
							key.renderTargetFormatType = (RenderTargetFormatType)config::default_RenderTargetFormatType_;

							/// PSOKeyとハンドルのマップに追加
							psoMap_[handle] = psoFactory_.createPSO(key);

							/// 探す用のマップに追加
							psoKeyToHandleMap_[key] = handle;

							handle++;
						}
					}
				}
			}
		}
	}

}

int PSOManager::GetPSOHandle(PSOKey keys) {

	auto it = psoKeyToHandleMap_.find(keys);
	if (it != psoKeyToHandleMap_.end()) {
		return it->second;
	}
	return -1;

}
