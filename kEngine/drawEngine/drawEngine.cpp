#include "drawEngine.h"
#include "Transform.h"
#include <math.h>
#include <tool/mathsUnit/Vector3.h>
#define M_PI 3.1415926f


DrawEngine::~DrawEngine() {
	for (auto& ptr : psoList_) {
		ptr->Release();
		ptr = nullptr;
	}
	psoList_.clear();

	delete pso_;

	delete tile2DWVPResource_;
	delete tile3DWVPResource_;

	delete defaultCamera_;
	delete cameraBuffer_;

	for (auto& ptr : instanceOffsetData_) {
		delete ptr->instanceOffsetResource;
		ptr->instanceOffsetResource = nullptr;
		ptr->instanceOffset = nullptr;
		delete ptr;
		ptr = nullptr;
	}

	if (depthStencilResource) {
		depthStencilResource->Release();
		depthStencilResource = nullptr;
	}


}

void DrawEngine::Initialize
(const char* kClientTitle, int kClientWidth, int kClientHeight, DirectXCore* directXDriver, SrvManager* srvManager, ResourceManager* resourceManager) {
	directXDriver_ = directXDriver;
	commandList_ = directXDriver_->GetCommandList();
	srvManager_ = srvManager;
	resourceManager_ = resourceManager;
	///
	kClientWidth_ = kClientWidth;
	kClientHeight_ = kClientHeight;

	///
	pso_->Initialize(directXDriver_);

	///================== PSO関連 ==================
	// PSOつくり
	for (int i = 0; i < (int)LightModelType::NumLightModels; i++) {
		ID3D12PipelineState* graphicsPipelineState_ = pso_->createPSO((LightModelType)i);
		psoList_.push_back(graphicsPipelineState_);
	}

	rootSignature_ = pso_->getRootSignature((int)psoType::defaultPSO);

	depthStencilResource = CreateDepthStencilTextureResource(directXDriver_->GetDevice(), kClientWidth, kClientHeight);
	MakeDepthStencilView();

	///
	viewport = createViewport(kClientWidth, kClientHeight);
	scissorRect = createScissorRect(kClientWidth, kClientHeight);



	///Lighting
	InitializeLighting();

	directionalLightData = new DirectionalLight{
			.color{1.0f,1.0f,1.0f,1.0f},
			.direction{-0.5f,-0.5f,0.4f},
			.intensity{1.0f}
	};


	/// Tile用wvpBufferを作成
	tile2DWVPResource_->CreateResourceClass_(directXDriver_->GetDevice(), sizeof(TransformationMatrix) * config::Get2DTileNumInstance());
	tile2DWVPResource_->GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&tile2DInstancingData_));
	for (int index = 0; index < config::Get2DTileNumInstance(); ++index) {
		tile2DInstancingData_[index].WVP = Identity();
		tile2DInstancingData_[index].world = Identity();
	}
	int srvHandleIndex = srvManager_->Allocate();
	srvManager_->CreateSRVforStructuredBuffer(srvHandleIndex, tile2DWVPResource_->GetResource().Get(), config::Get2DTileNumInstance(), sizeof(TransformationMatrix));  // 修正：使用正確的 tile2DWVPResource_

	Tile2DSrvHandleGPU_ = srvManager_->GetGPUDescriptorHandle(srvHandleIndex);

	tile3DWVPResource_->CreateResourceClass_(directXDriver_->GetDevice(), sizeof(TransformationMatrix) * config::Get3DTileNumInstance());
	tile3DWVPResource_->GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&tile3DInstancingData_));
	for (int index = 0; index < config::Get3DTileNumInstance(); ++index) {
		tile3DInstancingData_[index].WVP = Identity();
		tile3DInstancingData_[index].world = Identity();
	}
	srvHandleIndex = srvManager_->Allocate();
	srvManager_->CreateSRVforStructuredBuffer(srvHandleIndex, tile3DWVPResource_->GetResource().Get(), config::Get3DTileNumInstance(), sizeof(TransformationMatrix));

	Tile3DSrvHandleGPU_ = srvManager_->GetGPUDescriptorHandle(srvHandleIndex);

	/// InstanceOffset用バッファを作成
	for (int i = 0; i < config::GetMaxMaterialNum(); i++) {
		OffsetData* offsetData = new OffsetData;
		UINT* offset = nullptr;
		offsetData->instanceOffsetResource->CreateResourceClass_(directXDriver_->GetDevice(), 256);
		offsetData->instanceOffsetResource->GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&offset));
		offsetData->instanceOffset = offset;
		offsetData->state = 0;
		instanceOffsetData_.push_back(offsetData);
	}

	//Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
	//D3D12GetDebugInterface(IID_PPV_ARGS(&debugController));
	//debugController->EnableDebugLayer();

	//Microsoft::WRL::ComPtr<ID3D12Debug1> debugController1;
	//debugController.As(&debugController1);
	//debugController1->SetEnableGPUBasedValidation(TRUE);

	/// カメラのスベア
	defaultCamera_ = new Camera;

	cameraBuffer_->CreateResourceClass_(directXDriver_->GetDevice(), sizeof(CameraForGPU));
	cameraBuffer_->GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&cameraPtr_));
	

	/// デフォルトのモデルを設定
	SetModel("resources/TemplateResource/object/plane/plane.obj");
	defaultTextureHandle_ = LoadModelTexture("resources/TemplateResource/texture/white5x5.png");

}

void DrawEngine::PreDraw() {
	// 描画用のDescriptorHeapの設定
	ID3D12DescriptorHeap* descriptorHeaps[] = { srvManager_->GetDescriptorHeap().Get() };  // 現在使用正確的 getter 函式
	commandList_->SetDescriptorHeaps(1, descriptorHeaps);

	commandList_->RSSetViewports(1, &viewport);  // Viewportを設定
	commandList_->RSSetScissorRects(1, &scissorRect);  // Scissorを設定
	/// RootSignatureを設定。PSOに設定しているけど別途設定が必要
	commandList_->SetPipelineState(psoList_[(int)defaultLightModel_]);  // PSOを設定
	rootSignature_ = pso_->getRootSignature((int)psoType::defaultPSO);
	commandList_->SetGraphicsRootSignature(rootSignature_);
	if (currentPSO_ != psoType::defaultPSO) {
		currentPSO_ = psoType::defaultPSO;
	}
	/// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	/// 各種のリソースを設定
	resourceManager_->CreateTurnResource();

	/// InstanceCounterReset
	instance2DCounter_ = 0;
	instance3DCounter_ = 0;
	offsetDataCounter_ = 0;

	/// Lighting
	SetLighting(directionalLightData);

	/// Set Camera
	SetCameraForGPU();

}

void DrawEngine::CommitDraw() {
	/// 集まったデータで描画
	/// Sprite描画
	DrawCall();
	//DrawModel();
	//DrawCube();
	//Draw3DTile();
	//DrawSprite();
	//Draw2DTile();
}

void DrawEngine::EndDraw() {

	/// 各種のリソースを解放
	resourceManager_->ClearTurnResource();

	for (auto& ptr : instanceOffsetData_) {
		if (ptr->state == 1) { ptr->state = 2; }
		if (ptr->state == 2) { ptr->state = 0; }
	}
}

void DrawEngine::SetDirectionalLight(DirectionalLight* light) {
	if (light) {
		if (!directionalLightData) {
			directionalLightData = new DirectionalLight;
		}
		*directionalLightData = *light;
	}
}

void DrawEngine::PSODecision(MaterialConfig& material) {
	bool psoChanged = false;

	LightModelType lightModelType = (LightModelType)(int)material.lightModelType;

	switch (lightModelType) {
	case LightModelType::Sprite2D:
		if (currentPSO_ != psoType::Sprite2D) {
			commandList_->SetPipelineState(psoList_[(int)LightModelType::Sprite2D]);
			currentPSO_ = psoType::Sprite2D;
			psoChanged = true;
		}
		break;
	case LightModelType::Lambert:
		if (currentPSO_ != psoType::Lambert) {
			commandList_->SetPipelineState(psoList_[(int)LightModelType::Lambert]);
			currentPSO_ = psoType::Lambert;
			psoChanged = true;
		}
		break;
	case LightModelType::HalfLambert:
		if (currentPSO_ != psoType::HalfLambert) {
			commandList_->SetPipelineState(psoList_[(int)LightModelType::HalfLambert]);
			currentPSO_ = psoType::HalfLambert;
			psoChanged = true;
		}
		break;
	case LightModelType::PhongReflection:
		if (currentPSO_ != psoType::PhongReflection) {
			commandList_->SetPipelineState(psoList_[(int)LightModelType::PhongReflection]);
			currentPSO_ = psoType::PhongReflection;
			psoChanged = true;
		}
	case LightModelType::BlinnPhongReflection:
		if (currentPSO_ != psoType::BlinnPhongReflection) {
			commandList_->SetPipelineState(psoList_[(int)LightModelType::BlinnPhongReflection]);
			currentPSO_ = psoType::BlinnPhongReflection;
			psoChanged = true;
		}
	}
	if (psoChanged) {
		rootSignature_ = pso_->getRootSignature((int)currentPSO_);
		commandList_->SetGraphicsRootSignature(rootSignature_);
	}
}

void DrawEngine::DrawTriangle(TransformationMatrix* wvpData, MaterialConfig material) {
	///// SetPSO
	//PSODecision(material);
	//
	///// SetMaterial
	//SetMaterial(material.uvTransformMatrix, material.textureColor, material.enableLighting);
	//
	///// texture設定
	//int materialHandle = readCommonTextureHandle(material.textureHandle);
	//textureSrvHandleGPU_ = directXDriver_->GetGPUDescriptorHandle(directXDriver_->GetSrvDescriptorHeap(), directXDriver_->GetDescriptorSizeSRV(), materialHandle);
	//commandList_->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);
	///// VBVを設定
	//D3D12_VERTEX_BUFFER_VIEW BufferView = resourceManager_->vertexResourceTriangle_->GetVertexBufferView();
	//commandList_->IASetVertexBuffers(0, 1, &BufferView);
	///// wvp用のCBufferの場所を設定
	//resourceManager_->vertexResourceTriangle_->SetWVPResource_(directXDriver_->GetDriver(), wvpData);
	//commandList_->SetGraphicsRootConstantBufferView(1, resourceManager_->vertexResourceTriangle_->GetWVPResource_()->GetGPUVirtualAddress());
	///// 描画！(DrawCall/ドローコール)。
	//commandList_->SetGraphicsRootConstantBufferView(3, resourceManager_->lightingResource_->GetResource()->GetGPUVirtualAddress());
	//commandList_->DrawInstanced(6, 1, 0, 0);
}

void DrawEngine::CollectSprite(Vector2 pos, MaterialConfig material) {
	//	resourceManager_->instanceManager_->AddSpriteInstance(pos, material);
}

void DrawEngine::DrawSprite() {

#pragma region 自分で作った
	//if (!resourceManager_->spriteList_.empty()) {
	//	int beforeMaterialIndex = -1;
	//
	//	/// SetPSO
	//	for (auto ptr : resourceManager_->spriteList_) {
	//		if (ptr->isDraw)continue;
	//
	//		/// SetPSO
	//		commandList_->SetPipelineState(PSO_[(int)resourceManager_->materialConfigList_[ptr->materialConfigIndex]->lightModelType]);
	//		currentLightModel_ = resourceManager_->materialConfigList_[ptr->materialConfigIndex]->lightModelType;
	//		beforeMaterialIndex = ptr->materialConfigIndex;
	//
	//		for (auto ptr2 : resourceManager_->spriteList_) {
	//			if (ptr2->isDraw)continue;
	//			if (resourceManager_->materialConfigList_[ptr->materialConfigIndex]->lightModelType != resourceManager_->materialConfigList_[ptr2->materialConfigIndex]->lightModelType)continue;
	//
	//			MaterialConfig* nowMaterial = resourceManager_->materialConfigList_[ptr2->materialConfigIndex];
	//			/// SetMaterial
	//			SetMaterial(resourceManager_->materialResourceSprite_->GetResource(), nowMaterial->uvTransformMatrix, nowMaterial->textureColor, false);
	//			commandList_->SetGraphicsRootConstantBufferView(0, resourceManager_->materialResourceSprite_->GetResource()->GetGPUVirtualAddress());
	//
	//			/// texture設定
	//			int materialHandle = readCommonTextureHandle(nowMaterial->textureHandle);
	//			textureSrvHandleGPU_ = directXDriver_->GetGPUDescriptorHandle(directXDriver_->GetSrvDescriptorHeap(), directXDriver_->GetDescriptorSizeSRV(), materialHandle);
	//			commandList_->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);
	//
	//			/// Spriteの描画。変更が必要なものだけ変更する.
	//			D3D12_VERTEX_BUFFER_VIEW VertexBufferView = resourceManager_->vertexResourceSpriteGroup_[0]->GetVertexBufferView();
	//			commandList_->IASetVertexBuffers(0, 1, &VertexBufferView);
	//			/// IBVを設定
	//			D3D12_INDEX_BUFFER_VIEW IndexBufferView = resourceManager_->vertexResourceSpriteGroup_[0]->GetIndexBufferView();
	//			commandList_->IASetIndexBuffer(&IndexBufferView);
	//
	//			for (auto ptr3 : resourceManager_->spriteList_) {
	//				if (ptr3->isDraw)continue;
	//				if (ptr3->materialConfigIndex != beforeMaterialIndex)continue;
	//
	//				/// wvp用のCBufferの場所を設定
	//				resourceManager_->vertexResourceSpriteGroup_[0]->SetWVPResource_(directXDriver_->GetDriver(), ptr3->position);
	//				/// TransformationMatrixCBufferの場所を設定
	//				commandList_->SetGraphicsRootConstantBufferView(1, resourceManager_->vertexResourceSpriteGroup_[0]->GetWVPResource_()->GetGPUVirtualAddress());
	//				/// 描画! (DrawCall)
	//
	//				commandList_->SetGraphicsRootConstantBufferView(3, resourceManager_->lightingResource_->GetGPUVirtualAddress());
	//				commandList_->DrawIndexedInstanced(12, 1, 0, 0, 0);
	//				ptr3->isDraw = true;
	//			}
	//		}
	//
	//	}
	//}
#pragma endregion

	//if (resourceManager_->instanceManager_->spriteList_.empty())return;
	//
	//struct MaterialStateCache {
	//	int materialIndex = -1;
	//	int textureHandle = -1;
	//	LightModelType lightModel = LightModelType::HalfLambert;
	//};
	//
	//MaterialStateCache lastMaterialState_;
	//
	//std::unordered_map<int, std::vector<SpriteInstance*>> groupedSprites;
	//
	//for (auto& ptr : resourceManager_->instanceManager_->spriteList_) {
	//	if (ptr->drawState == InstanceManager::STANDBY) {
	//		groupedSprites[ptr->materialConfigIndex].push_back(ptr);
	//	}
	//}
	//
	//for (auto& [materialIndex, group] : groupedSprites) {
	//	MaterialConfig* material = resourceManager_->instanceManager_->materialConfigList_[materialIndex];
	//
	//	bool needSetMaterial =
	//		materialIndex != lastMaterialState_.materialIndex ||
	//		material->textureHandle != lastMaterialState_.textureHandle ||
	//		material->lightModelType != lastMaterialState_.lightModel;
	//
	//	if (needSetMaterial) {
	//		// Set PSO
	//		commandList_->SetPipelineState(psoList_[(int)psoType::defaultPSO]);
	//		currentPSO_ = psoType::defaultPSO;
	//		rootSignature_ = pso_->getRootSignature((int)psoType::defaultPSO);
	//		commandList_->SetGraphicsRootSignature(rootSignature_);
	//
	//		// Set Material
	//		SetMaterial(material->uvTransformMatrix, material->textureColor, false);
	//
	//		// Set Texture
	//		int materialHandle = readCommonTextureHandle(material->textureHandle);
	//		textureSrvHandleGPU_ = directXDriver_->GetGPUDescriptorHandle(directXDriver_->GetSrvDescriptorHeap(), directXDriver_->GetDescriptorSizeSRV(), materialHandle);
	//		commandList_->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);
	//
	//		// Update cache
	//		lastMaterialState_.materialIndex = materialIndex;
	//		lastMaterialState_.textureHandle = material->textureHandle;
	//		lastMaterialState_.lightModel = material->lightModelType;
	//	}
	//
	//	// Set VB/IB
	//	D3D12_VERTEX_BUFFER_VIEW VertexBufferView = resourceManager_->vertexResourceSpriteGroup_[0]->GetVertexBufferView();
	//	commandList_->IASetVertexBuffers(0, 1, &VertexBufferView);
	//	D3D12_INDEX_BUFFER_VIEW IndexBufferView = resourceManager_->vertexResourceSpriteGroup_[0]->GetIndexBufferView();
	//	commandList_->IASetIndexBuffer(&IndexBufferView);
	//
	//	///// ループの中に入らないように先に計算しとく
		//Matrix4x4 viewMatrixSprite; viewMatrixSprite.Identity();
		//Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(config::GetClientWidth()), float(config::GetClientHeight()), 0.0f, 100.0f);
		//Matrix4x4 viewProj = viewMatrixSprite * projectionMatrixSprite;
		//
		//int spriteCount = (int)group.size();
		//
		//for (int i = 0; i < group.size(); i++) {
		//	// 単位行列を書き込んておく
		//	if (tileInstancingData_[i].WVP != Identity()) tileInstancingData_[i].WVP = Identity();
		//	if (tileInstancingData_[i].world != Identity()) tileInstancingData_[i].world = Identity();
		//
		//	// CPUで動かす用のTransformを作る。
		//	Transform transformSprite = CreateDefaultTransform();
		//	if (group[i] != nullptr) {
		//		transformSprite.translate = { group[i]->position.x,group[i]->position.y,0.0f };
		//	}
		//
		//	// Sprite用のworldViewProjectionMatrixを作る
		//	Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
		//	Matrix4x4 worldViewProjectionMatrixSprite = worldMatrixSprite * viewProj;
		//
		//	group[i]->drawState = InstanceManager::ISDRAW;
		//}
	//
	//	// Draw all sprites with same material
	//	for (auto ptr : group) {
	//		resourceManager_->vertexResourceSpriteGroup_[0]->SetWVPResource_(directXDriver_->GetDriver(), ptr->position);
	//		commandList_->SetGraphicsRootConstantBufferView(1, resourceManager_->vertexResourceSpriteGroup_[0]->GetWVPResource_()->GetGPUVirtualAddress());
	//		commandList_->SetGraphicsRootConstantBufferView(3, resourceManager_->lightingResource_->GetResource()->GetGPUVirtualAddress());
	//		commandList_->DrawIndexedInstanced(12, 1, 0, 0, 0);
	//		ptr->drawState = InstanceManager::ISDRAW;
	//	}
	//}
}

void DrawEngine::DrawSpriteDirect(Vector2 pos, MaterialConfig material) {
	///// SetPSO
	//PSODecision(material);
	//
	///// SetMaterial
	//SetMaterial(material.uvTransformMatrix, material.textureColor, false);
	///// texture設定
	//int materialHandle = readCommonTextureHandle(material.textureHandle);
	//textureSrvHandleGPU_ = directXDriver_->GetGPUDescriptorHandle(directXDriver_->GetSrvDescriptorHeap(), directXDriver_->GetDescriptorSizeSRV(), materialHandle);
	//commandList_->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);
	///// Spriteの描画。変更が必要なものだけ変更する9652.
	//D3D12_VERTEX_BUFFER_VIEW VertexBufferView = resourceManager_->vertexResourceSpriteGroup_[0]->GetVertexBufferView();
	//commandList_->IASetVertexBuffers(0, 1, &VertexBufferView);
	///// IBVを設定
	//D3D12_INDEX_BUFFER_VIEW IndexBufferView = resourceManager_->vertexResourceSpriteGroup_[0]->GetIndexBufferView();
	//commandList_->IASetIndexBuffer(&IndexBufferView);
	///// wvp用のCBufferの場所を設定
	//resourceManager_->vertexResourceSpriteGroup_[0]->SetWVPResource_(directXDriver_->GetDriver(), { pos.x,pos.y,0.0f });
	///// TransformationMatrixCBufferの場所を設定
	//commandList_->SetGraphicsRootConstantBufferView(1, resourceManager_->vertexResourceSpriteGroup_[0]->GetWVPResource_()->GetGPUVirtualAddress());
	///// 描画! (DrawCall)
	//
	//commandList_->SetGraphicsRootConstantBufferView(3, resourceManager_->lightingResource_->GetResource()->GetGPUVirtualAddress());
	//commandList_->DrawIndexedInstanced(12, 1, 0, 0, 0);
}

void DrawEngine::DrawSpriteDirect(Vector2 pos, MaterialConfig material, Vector2 LTPos, Vector2 LBPos, Vector2 RTPos, Vector2 RBPos, float TsizeX, float TsizeY, Vector2 TCLTPos, Vector2 TCRBPos) {
	///// SetPSO
	//PSODecision(material);
	//
	//int vertexHandle = -1;
	//if (!resourceManager_->vertexResourceSpriteGroup_.empty()) {
	//	for (int i = 0; i < resourceManager_->vertexResourceSpriteGroup_.size(); i++) {
	//		if (resourceManager_->vertexResourceSpriteGroup_[i]->CheckSize(LTPos, LBPos, RTPos, RBPos, TsizeX, TsizeY, TCLTPos, TCRBPos)) {
	//			resourceManager_->vertexResourceSpriteGroup_[i]->SetKeep(true);
	//			vertexHandle = i;
	//			break;
	//		}
	//	}
	//}
	//if (vertexHandle == -1) {
	//	Sprite2D* sprite = new Sprite2D;
	//	sprite->SetSize(LTPos, LBPos, RTPos, RBPos, TsizeX, TsizeY, TCLTPos, TCRBPos);
	//	sprite->CreateVertexResource_(directXDriver_->GetDriver());
	//	sprite->CreateVertexBufferView_(5);
	//	sprite->CreateIndexResource_(directXDriver_->GetDriver());
	//	sprite->CreateIndexBufferView_(12);
	//	sprite->SetKeep(true);
	//	resourceManager_->vertexResourceSpriteGroup_.push_back(sprite);
	//	vertexHandle = int(resourceManager_->vertexResourceSpriteGroup_.size() - 1);
	//}
	//
	///// SetMaterial
	//SetMaterial(material.uvTransformMatrix, material.textureColor, false);
	///// texture設定
	//int materialHandle = readCommonTextureHandle(material.textureHandle);
	//textureSrvHandleGPU_ = directXDriver_->GetGPUDescriptorHandle(directXDriver_->GetSrvDescriptorHeap(), directXDriver_->GetDescriptorSizeSRV(), materialHandle);
	//commandList_->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);
	///// Spriteの描画。変更が必要なものだけ変更する
	//D3D12_VERTEX_BUFFER_VIEW VertexBufferView = resourceManager_->vertexResourceSpriteGroup_[vertexHandle]->GetVertexBufferView();
	//commandList_->IASetVertexBuffers(0, 1, &VertexBufferView);
	///// IBVを設定
	//D3D12_INDEX_BUFFER_VIEW IndexBufferView = resourceManager_->vertexResourceSpriteGroup_[vertexHandle]->GetIndexBufferView();
	//commandList_->IASetIndexBuffer(&IndexBufferView);
	///// wvp用のCBufferの場所を設定
	//resourceManager_->vertexResourceSpriteGroup_[vertexHandle]->SetWVPResource_(directXDriver_->GetDriver(), { pos.x,pos.y,0.0f });
	///// TransformationMatrixCBufferの場所を設定
	//commandList_->SetGraphicsRootConstantBufferView(1, resourceManager_->vertexResourceSpriteGroup_[vertexHandle]->GetWVPResource_()->GetGPUVirtualAddress());
	///// 描画! (DrawCall)
	//commandList_->SetGraphicsRootConstantBufferView(3, resourceManager_->lightingResource_->GetResource()->GetGPUVirtualAddress());
	//commandList_->DrawIndexedInstanced(12, 1, 0, 0, 0);
}

void DrawEngine::Collect2DTile(Vector2 pos, MaterialConfig material) {
	//	resourceManager_->Collet2DTile(pos, material);
}

void DrawEngine::Draw2DTile() {
	//	if (resourceManager_->instanceManager_->tile2DList_.empty())return;
	//	struct MaterialStateCache {
	//		int materialIndex = -1;
	//		int textureHandle = -1;
	//		LightModelType lightModel = LightModelType::HalfLambert;
	//	};
	//
	//	MaterialStateCache lastMaterialState_;
	//
	//	std::unordered_map<int, std::vector<SpriteInstance*>> groupedTiles;
	//
	//	for (auto& ptr : resourceManager_->instanceManager_->tile2DList_) {
	//		if (ptr->drawState == InstanceManager::STANDBY) {
	//			groupedTiles[ptr->materialConfigIndex].push_back(ptr);
	//		}
	//	}
	//
	//	/// ループの中に入らないように先に計算しとく
	//	Matrix4x4 viewMatrixSprite; viewMatrixSprite.Identity();
	//	Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(config::GetClientWidth()), float(config::GetClientHeight()), 0.0f, 100.0f);
	//	Matrix4x4 viewProj = viewMatrixSprite * projectionMatrixSprite;
	//
	//	for (auto& [materialIndex, group] : groupedTiles) {
	//		MaterialConfig* material = resourceManager_->instanceManager_->materialConfigList_[materialIndex];
	//
	//		bool needSetMaterial =
	//			((materialIndex != lastMaterialState_.materialIndex) ||
	//				(material->textureHandle != lastMaterialState_.textureHandle) ||
	//				(material->lightModelType != lastMaterialState_.lightModel));
	//
	//
	//		if (needSetMaterial) {
	//			// Set PSO
	//			commandList_->SetPipelineState(psoList_[(int)psoType::Tile]);
	//			currentPSO_ = psoType::Tile;
	//			rootSignature_ = pso_->getRootSignature((int)psoType::Tile);
	//			commandList_->SetGraphicsRootSignature(rootSignature_);
	//
	//			// Set Material
	//
	//			SetMaterial(material->materialResourceHandle);
	//
	//			// Set Texture
	//			int materialHandle = readCommonTextureHandle(material->textureHandle);
	//			textureSrvHandleGPU_ = directXDriver_->GetGPUDescriptorHandle(directXDriver_->GetSrvDescriptorHeap(), directXDriver_->GetDescriptorSizeSRV(), materialHandle);
	//			commandList_->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);
	//
	//			// Update cache
	//			lastMaterialState_.materialIndex = materialIndex;
	//			lastMaterialState_.textureHandle = material->textureHandle;
	//			lastMaterialState_.lightModel = material->lightModelType;
	//
	//			material->drawState = InstanceManager::ISDRAW;
	//		}
	//
	//		// Set VB/IB
	//		int MeshBufferHandle = config::default_Sprite2D_MeshBufferHandle_;
	//		D3D12_VERTEX_BUFFER_VIEW VertexBufferView = resourceManager_->meshBufferList_[config::default_Sprite2D_MeshBufferHandle_]->GetVertexBufferView();
	//		commandList_->IASetVertexBuffers(0, 1, &VertexBufferView);
	//		D3D12_INDEX_BUFFER_VIEW IndexBufferView = resourceManager_->meshBufferList_[config::default_Sprite2D_MeshBufferHandle_]->GetIndexBufferView();
	//		commandList_->IASetIndexBuffer(&IndexBufferView);
	//
	//		int tileCount = (int)group.size();
	//
	//		int wvpInstanceStartPoint = instance2DCounter_;
	//
	//		for (int i = 0; i < group.size(); i++) {
	//
	//			///int instanceCounter = max(instance2DCounter - 1,0);
	//			int instanceCounter = instance2DCounter_;
	//
	//			// 単位行列を書き込んておく
	//			if (tile2DInstancingData_[instanceCounter].WVP != Identity()) tile2DInstancingData_[instanceCounter].WVP = Identity();
	//			if (tile2DInstancingData_[instanceCounter].world != Identity()) tile2DInstancingData_[instanceCounter].world = Identity();
	//
	//			// CPUで動かす用のTransformを作る。
	//			Transform transformSprite = CreateDefaultTransform();
	//			if (group[i] != nullptr) {
	//				transformSprite.translate = { group[i]->position.x,group[i]->position.y,group[i]->position.z };
	//			}
	//
	//			// Sprite用のworldViewProjectionMatrixを作る
	//			Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
	//			Matrix4x4 worldViewProjectionMatrixSprite = worldMatrixSprite * viewProj;
	//			tile2DInstancingData_[instanceCounter].WVP = worldViewProjectionMatrixSprite;
	//
	//			group[i]->drawState = InstanceManager::ISDRAW;
	//			instance2DCounter_++;
	//		}
	//
	//		//*instanceOffsetData_ = wvpInstanceStartPoint;
	//		OffsetData* inUse = nullptr;
	//		inUse = instanceOffsetData_[offsetDataCounter_];
	//		*instanceOffsetData_[offsetDataCounter_]->instanceOffset = static_cast<UINT>(wvpInstanceStartPoint);
	//		instanceOffsetData_[offsetDataCounter_]->state = 1;
	//
	//		commandList_->SetGraphicsRootConstantBufferView(4, inUse->instanceOffsetResource->GetResource()->GetGPUVirtualAddress());
	//
	//		commandList_->SetGraphicsRootDescriptorTable(1, Tile2DSrvHandleGPU_);
	//		commandList_->SetGraphicsRootConstantBufferView(3, resourceManager_->lightingResource_->GetResource()->GetGPUVirtualAddress());
	//		commandList_->DrawIndexedInstanced(12, tileCount, 0, 0, 0);
	//		offsetDataCounter_++;
	//	}
}

void DrawEngine::CollectCube(TransformationMatrix* wvpData, MaterialConfig material) {
	//resourceManager_->ColletModel(wvpData, { material }, resourceManager_->default_Cube_MeshBufferHandle_, true);
//	resourceManager_->Collet3DTile(wvpData, { material }, config::default_Cube_MeshBufferHandle_, true);
}

void DrawEngine::DrawCube() {

	//	if (resourceManager_->instanceManager_->modelList_.empty())return;
	//	struct MaterialStateCache {
	//		int materialIndex = -1;
	//		int textureHandle = -1;
	//		LightModelType lightModel = LightModelType::HalfLambert;
	//	};
	//
	//	MaterialStateCache lastMaterialState_;
	//
	//	std::unordered_map<int, std::vector<ModelInstance*>> groupedModelTile;
	//
	//	for (auto& ptr : resourceManager_->instanceManager_->modelList_) {
	//		if (ptr->drawState == InstanceManager::STANDBY) {
	//			groupedModelTile[ptr->materialConfigIndex].push_back(ptr);
	//		}
	//	}
	//
	//	for (auto& [materialIndex, group] : groupedModelTile) {
	//
	//		MaterialConfig* material = resourceManager_->instanceManager_->materialConfigList_[materialIndex];
	//
	//		bool needSetMaterial =
	//			((materialIndex != lastMaterialState_.materialIndex) ||
	//				(material->textureHandle != lastMaterialState_.textureHandle) ||
	//				(material->lightModelType != lastMaterialState_.lightModel));
	//
	//		if (needSetMaterial) {
	//			// Set PSO
	//			PSODecision(*material);
	//
	//			// Set Material
	//			SetMaterial(material->materialResourceHandle);
	//
	//			// Set Texture
	//			int materialHandle;
	//			if (material->useOriginalTexture)materialHandle = readModelTextureHandle(material->textureHandle);
	//			if (!material->useOriginalTexture)materialHandle = readCommonTextureHandle(material->textureHandle);
	//			textureSrvHandleGPU_ = directXDriver_->GetGPUDescriptorHandle(directXDriver_->GetSrvDescriptorHeap(), directXDriver_->GetDescriptorSizeSRV(), materialHandle);
	//			commandList_->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);
	//
	//			// Update cache
	//			lastMaterialState_.materialIndex = materialIndex;
	//			lastMaterialState_.textureHandle = material->textureHandle;
	//			lastMaterialState_.lightModel = material->lightModelType;
	//
	//			material->drawState = InstanceManager::ISDRAW;
	//		}
	//
	//		bool isDraw = false;
	//
	//		for (auto& ptr : group) {
	//			if (ptr->useDefaultModel && ptr->modelHandle == config::default_Cube_MeshBufferHandle_) {
	//				/// wvp用のCBufferの場所を設定
	//				ID3D12Resource* wvpResource = resourceManager_->wvpResource_->CreateWVPResource_(directXDriver_->GetDriver(), ptr->transformData);
	//				commandList_->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
	//
	//
	//				/// VBVを設定
	//				D3D12_VERTEX_BUFFER_VIEW VertexBufferView = resourceManager_->meshBufferList_[config::default_Cube_MeshBufferHandle_]->GetVertexBufferView();
	//				commandList_->IASetVertexBuffers(0, 1, &VertexBufferView);
	//
	//				/// IBVを設定
	//				D3D12_INDEX_BUFFER_VIEW IndexBufferView = resourceManager_->meshBufferList_[config::default_Cube_MeshBufferHandle_]->GetIndexBufferView();
	//				commandList_->IASetIndexBuffer(&IndexBufferView);
	//
	//				/// 描画
	//				commandList_->SetGraphicsRootConstantBufferView(3, resourceManager_->lightingResource_->GetResource()->GetGPUVirtualAddress());
	//				commandList_->DrawIndexedInstanced(ptr->vertexNum, 1, 0, 0, 0);
	//				ptr->drawState = InstanceManager::ISDRAW;
	//				isDraw = true;
	//			}
	//		}
	//
	//		if (!isDraw)material->drawState = InstanceManager::STANDBY;
	//	}
}

void DrawEngine::DrawSphere(TransformationMatrix* wvpData, MaterialConfig material, int subdivision) {
	//subdivision;
	///// SetPSO
	//PSODecision(material);
	//
	///// SetMaterial
	//SetMaterial(material.uvTransformMatrix, material.textureColor, material.enableLighting);
	///// texture設定
	//int materialHandle = readCommonTextureHandle(material.textureHandle);
	//textureSrvHandleGPU_ = directXDriver_->GetGPUDescriptorHandle(directXDriver_->GetSrvDescriptorHeap(), directXDriver_->GetDescriptorSizeSRV(), materialHandle);
	//commandList_->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);
	//
	///// VBVを設定
	//resourceManager_->vertexResourceSphere_->CreateVertexResource_(directXDriver_->GetDriver());
	//
	//D3D12_VERTEX_BUFFER_VIEW BufferView = resourceManager_->vertexResourceSphere_->GetVertexBufferView();
	//commandList_->IASetVertexBuffers(0, 1, &BufferView);  // VBVを設定
	//
	///// IBVを設定
	//resourceManager_->vertexResourceSphere_->CreateIndexResource_(directXDriver_->GetDriver());
	//D3D12_INDEX_BUFFER_VIEW IndexBufferView = resourceManager_->vertexResourceSphere_->GetIndexBufferView();
	//commandList_->IASetIndexBuffer(&IndexBufferView);
	//
	///// wvp用のCBufferの場所を設定
	//resourceManager_->vertexResourceSphere_->SetWVPResource_(directXDriver_->GetDriver(), wvpData);
	//commandList_->SetGraphicsRootConstantBufferView(1, resourceManager_->vertexResourceSphere_->GetWVPResource_()->GetGPUVirtualAddress());
	//
	///// 描画
	//commandList_->SetGraphicsRootConstantBufferView(3, resourceManager_->lightingResource_->GetResource()->GetGPUVirtualAddress());
	//commandList_->DrawIndexedInstanced(resourceManager_->vertexResourceSphere_->GetVertexNum(), 1, 0, 0, 0);
}

void DrawEngine::CollectModel(TransformationMatrix* wvpData, std::vector<MaterialConfig> material, int modelHandle) {
	//resourceManager_->ColletModel(wvpData, material, modelHandle);
//	resourceManager_->Collet3DTile(wvpData, material, modelHandle);
}

void DrawEngine::DrawModel() {

	//	if (resourceManager_->instanceManager_->modelList_.empty())return;
	//	struct MaterialStateCache {
	//		int materialIndex = -1;
	//		int textureHandle = -1;
	//		LightModelType lightModel = LightModelType::HalfLambert;
	//	};
	//
	//	MaterialStateCache lastMaterialState_;
	//
	//	std::unordered_map<int, std::vector<ModelInstance*>> groupedModelTile;
	//
	//	for (auto& ptr : resourceManager_->instanceManager_->modelList_) {
	//		if (ptr->drawState == InstanceManager::STANDBY) {
	//			groupedModelTile[ptr->materialConfigIndex].push_back(ptr);
	//		}
	//	}
	//
	//	for (auto& [materialIndex, group] : groupedModelTile) {
	//
	//		MaterialConfig* material = resourceManager_->instanceManager_->materialConfigList_[materialIndex];
	//
	//		bool needSetMaterial =
	//			((materialIndex != lastMaterialState_.materialIndex) ||
	//				(material->textureHandle != lastMaterialState_.textureHandle) ||
	//				(material->lightModelType != lastMaterialState_.lightModel));
	//
	//		if (needSetMaterial) {
	//			// Set PSO
	//			PSODecision(*material);
	//
	//			// Set Material
	//			SetMaterial(material->materialResourceHandle);
	//
	//			// Set Texture
	//			int materialHandle;
	//			if (material->useOriginalTexture)materialHandle = readModelTextureHandle(material->textureHandle);
	//			if (!material->useOriginalTexture)materialHandle = readCommonTextureHandle(material->textureHandle);
	//			textureSrvHandleGPU_ = directXDriver_->GetGPUDescriptorHandle(directXDriver_->GetSrvDescriptorHeap(), directXDriver_->GetDescriptorSizeSRV(), materialHandle);
	//			commandList_->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);
	//
	//			// Update cache
	//			lastMaterialState_.materialIndex = materialIndex;
	//			lastMaterialState_.textureHandle = material->textureHandle;
	//			lastMaterialState_.lightModel = material->lightModelType;
	//
	//			material->drawState = InstanceManager::ISDRAW;
	//		}
	//
	//		bool isDraw = false;
	//
	//		for (auto& ptr : group) {
	//			if (!ptr->useDefaultModel) {
	//				/// wvp用のCBufferの場所を設定
	//				ID3D12Resource* wvpResource = resourceManager_->wvpResource_->CreateWVPResource_(directXDriver_->GetDriver(), ptr->transformData);
	//				commandList_->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
	//
	//
	//				/// VBVを設定
	//				D3D12_VERTEX_BUFFER_VIEW BufferView = resourceManager_->meshBufferList_[ptr->modelHandle]->GetVertexBufferView();
	//				commandList_->IASetVertexBuffers(0, 1, &BufferView);
	//
	//				/// 描画
	//				commandList_->SetGraphicsRootConstantBufferView(3, resourceManager_->lightingResource_->GetResource()->GetGPUVirtualAddress());
	//				commandList_->DrawInstanced(ptr->vertexNum, 1, 0, 0);
	//				ptr->drawState = InstanceManager::ISDRAW;
	//				isDraw = true;
	//			}
	//		}
	//
	//		if (!isDraw)material->drawState = InstanceManager::STANDBY;
	//	}
}

void DrawEngine::Collect3DTile(TransformationMatrix* wvpData, std::vector<MaterialConfig> material, int modelHandle) {
	//if (modelHandle == -1) {
	//	resourceManager_->Collet3DTile(wvpData, { material }, config::default_Cube_MeshBufferHandle_, true);
	//	return;
	//}
	//resourceManager_->Collet3DTile(wvpData, material, modelHandle);
}

void DrawEngine::Draw3DTile() {

	//if (resourceManager_->instanceManager_->tile3DList_.empty()) return;
	//
	//struct MaterialStateCache {
	//	int materialIndex = -1;
	//	int textureHandle = -1;
	//	LightModelType lightModel = LightModelType::HalfLambert;
	//} lastMaterialState_;
	//
	//std::unordered_map<int, std::vector<ModelInstance*>> groupedByMaterial;
	//for (auto& ptr : resourceManager_->instanceManager_->tile3DList_) {
	//	if (ptr->drawState == InstanceManager::STANDBY) {
	//		groupedByMaterial[ptr->materialConfigIndex].push_back(ptr);
	//	}
	//}
	//
	//int instancesCounter = 0;
	//
	//for (auto& [materialIndex, group] : groupedByMaterial) {
	//	MaterialConfig* material = resourceManager_->instanceManager_->materialConfigList_[materialIndex];
	//
	//	bool needSetMaterial =
	//		(materialIndex != lastMaterialState_.materialIndex) ||
	//		(material->textureHandle != lastMaterialState_.textureHandle) ||
	//		(material->lightModelType != lastMaterialState_.lightModel);
	//
	//	if (needSetMaterial) {
	//		// Set PSO
	//		PSODecision(*material, true);
	//
	//		// Set Material
	//		SetMaterial(material->materialResourceHandle);
	//
	//		// Set Texture
	//		int materialHandle;
	//		if (material->useOriginalTexture)materialHandle = readModelTextureHandle(material->textureHandle);
	//		if (!material->useOriginalTexture)materialHandle = readCommonTextureHandle(material->textureHandle);
	//		textureSrvHandleGPU_ = directXDriver_->GetGPUDescriptorHandle(directXDriver_->GetSrvDescriptorHeap(), directXDriver_->GetDescriptorSizeSRV(), materialHandle);
	//		commandList_->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);
	//
	//		// Update cache
	//		lastMaterialState_.materialIndex = materialIndex;
	//		lastMaterialState_.textureHandle = material->textureHandle;
	//		lastMaterialState_.lightModel = material->lightModelType;
	//		material->drawState = InstanceManager::ISDRAW;
	//	}
	//
	//	std::unordered_map<int, std::vector<ModelInstance*>> groupedByModel;
	//	for (auto& ptr : group) {
	//		if (ptr->drawState == InstanceManager::STANDBY) {
	//			groupedByModel[ptr->modelHandle].push_back(ptr);
	//		}
	//	}
	//
	//	for (auto& [modelHandle, instances] : groupedByModel) {
	//		// 設定 VBV/IBV
	//		int meshHandle;
	//
	//		meshHandle = instances[0]->useDefaultModel
	//			? config::default_Cube_MeshBufferHandle_
	//			: modelHandle;
	//
	//		auto* mesh = resourceManager_->meshBufferList_[meshHandle];
	//		D3D12_VERTEX_BUFFER_VIEW vbv = mesh->GetVertexBufferView();
	//		D3D12_INDEX_BUFFER_VIEW ibv = mesh->GetIndexBufferView();
	//		commandList_->IASetVertexBuffers(0, 1, &vbv);
	//		commandList_->IASetIndexBuffer(&ibv);
	//
	//		// 計算 instance 數量
	//		int tileCount = static_cast<int>(instances.size());
	//		int wvpInstanceStartPoint = instance3DCounter_;
	//
	//		// 寫入 instancing buffer
	//		for (int i = 0; i < tileCount; ++i) {
	//
	//			tile3DInstancingData_[instance3DCounter_].WVP = instances[i]->transformData.WVP;
	//			tile3DInstancingData_[instance3DCounter_].world = instances[i]->transformData.world;
	//
	//			instances[i]->drawState = InstanceManager::ISDRAW;
	//			instance3DCounter_++;
	//		}
	//
	//		// 設定 OffsetData
	//		OffsetData* inUse = instanceOffsetData_[offsetDataCounter_];
	//		*instanceOffsetData_[offsetDataCounter_]->instanceOffset = static_cast<UINT>(wvpInstanceStartPoint);
	//		instanceOffsetData_[offsetDataCounter_]->state = 1;
	//
	//		commandList_->SetGraphicsRootConstantBufferView(4, inUse->instanceOffsetResource->GetResource()->GetGPUVirtualAddress());
	//		commandList_->SetGraphicsRootDescriptorTable(1, Tile3DSrvHandleGPU_);
	//		commandList_->SetGraphicsRootConstantBufferView(3, resourceManager_->lightingResource_->GetResource()->GetGPUVirtualAddress());
	//
	//		// 一次性繪製
	//		commandList_->DrawIndexedInstanced(instances[0]->vertexNum, tileCount, 0, 0, 0);
	//		offsetDataCounter_++;
	//	}
	//
	//}

}

void DrawEngine::Collect2D(SpriteData* spriteData) {
	resourceManager_->Collet2D(spriteData);
}

void DrawEngine::Collect3D(ObjectData* object) {
	resourceManager_->Collet3D(object);
}

void DrawEngine::Draw2D() {
	if (resourceManager_->instanceManager_->tile2DList_.empty())return;
	struct MaterialStateCache {
		int materialIndex = -1;
		int textureHandle = -1;
		LightModelType lightModel = LightModelType::HalfLambert;
	};

	MaterialStateCache lastMaterialState_;

	std::unordered_map<int, std::vector<SpriteInstance*>> groupedTiles;

	for (auto& ptr : resourceManager_->instanceManager_->tile2DList_) {
		if (ptr->drawState == InstanceManager::STANDBY) {
			groupedTiles[ptr->materialConfigIndex].push_back(ptr);
		}
	}

	/// ループの中に入らないように先に計算しとく
	Matrix4x4 viewMatrixSprite; viewMatrixSprite.Identity();
	Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(config::GetClientWidth()), float(config::GetClientHeight()), 0.0f, 100.0f);
	Matrix4x4 viewProj = viewMatrixSprite * projectionMatrixSprite;

	int simpleSpriteCounter{};
	resourceManager_->CreateSpriteMesh();

	for (auto& groupPair : groupedTiles) {
		int materialIndex = groupPair.first;
		std::vector<SpriteInstance*>& group = groupPair.second;

		MaterialConfig* material = resourceManager_->instanceManager_->materialConfigList_[materialIndex];

		bool needSetMaterial =
			((materialIndex != lastMaterialState_.materialIndex) ||
				(material->textureHandle != lastMaterialState_.textureHandle) ||
				(material->lightModelType != lastMaterialState_.lightModel));

		/// Metadataを準備
		DirectX::TexMetadata metaData{};

		if (needSetMaterial) {
			// Set PSO
			commandList_->SetPipelineState(psoList_[(int)psoType::Sprite2D]);
			currentPSO_ = psoType::Sprite2D;
			rootSignature_ = pso_->getRootSignature((int)psoType::Sprite2D);
			commandList_->SetGraphicsRootSignature(rootSignature_);

			// Set Material
			SetMaterial(material->materialResourceHandle);

			// Set Texture
			int textureHandle = material->textureHandle;
			textureSrvHandleGPU_ = resourceManager_->GetTextureGPUDescriptorHandle(textureHandle);
			commandList_->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);

			/// MataDataを取得
			metaData = resourceManager_->GetTextureMetadata(textureHandle);

			/// Update cache
			lastMaterialState_.materialIndex = materialIndex;
			lastMaterialState_.textureHandle = material->textureHandle;
			lastMaterialState_.lightModel = material->lightModelType;

			material->drawState = InstanceManager::ISDRAW;
		}

		for (int i = 0; i < (int)group.size(); ++i) {
			SpriteInstance* inst = group[i];

			resourceManager_->ResizeSimpleSpriteMesh(
				metaData,
				simpleSpriteCounter,
				inst->cornerData,
				inst->anchorPoint,
				inst->cropLT,
				inst->cropSize
			);

			D3D12_VERTEX_BUFFER_VIEW vbv =
				resourceManager_->simpleSpriteMeshList_[simpleSpriteCounter]->GetVertexBufferView();
			D3D12_INDEX_BUFFER_VIEW ibv =
				resourceManager_->simpleSpriteMeshList_[simpleSpriteCounter]->GetIndexBufferView();

			commandList_->IASetVertexBuffers(0, 1, &vbv);
			commandList_->IASetIndexBuffer(&ibv);

			// 計算 transform
			Transform t = CreateDefaultTransform();
			t.translate = inst->position;
			t.scale = { inst->scale.x, inst->scale.y, 1.0f };
			t.rotate = inst->rotate;

			Matrix4x4 world = MakeAffineMatrix(t.scale, t.rotate, t.translate);
			Matrix4x4 wvp = world * viewProj;

			int instIdx = instance2DCounter_;
			tile2DInstancingData_[instIdx].WVP = wvp;
			tile2DInstancingData_[instIdx].world = world;

			inst->drawState = InstanceManager::ISDRAW;
			++instance2DCounter_;

			// 設定 offset
			OffsetData* inUse = instanceOffsetData_[offsetDataCounter_];
			*inUse->instanceOffset = static_cast<UINT>(instIdx);
			inUse->state = 1;

			commandList_->SetGraphicsRootConstantBufferView(
				4, inUse->instanceOffsetResource->GetResource()->GetGPUVirtualAddress());
			commandList_->SetGraphicsRootDescriptorTable(1, Tile2DSrvHandleGPU_);
			commandList_->SetGraphicsRootConstantBufferView(
				3, resourceManager_->lightingResource_->GetResource()->GetGPUVirtualAddress());

			int vertexNum =
				resourceManager_->simpleSpriteMeshList_[simpleSpriteCounter]->GetVertexNum();

			// 每個 instance 單獨 draw
			commandList_->DrawIndexedInstanced(vertexNum, 1, 0, 0, 0);

			++offsetDataCounter_;
			++simpleSpriteCounter;
		}
	}
}

void DrawEngine::Draw3D() {
	if (resourceManager_->instanceManager_->tile3DList_.empty()) return;

	struct MaterialStateCache {
		int materialIndex = -1;
		int textureHandle = -1;
		LightModelType lightModel = LightModelType::HalfLambert;
	} lastMaterialState_;

	std::unordered_map<int, std::vector<ModelInstance*>> groupedByMaterial;
	for (auto& ptr : resourceManager_->instanceManager_->tile3DList_) {
		if (ptr->drawState == InstanceManager::STANDBY) {
			groupedByMaterial[ptr->materialConfigIndex].push_back(ptr);
		}
	}

	int instancesCounter = 0;

	for (auto& [materialIndex, group] : groupedByMaterial) {
		MaterialConfig* material = resourceManager_->instanceManager_->materialConfigList_[materialIndex];

		bool needSetMaterial =
			(materialIndex != lastMaterialState_.materialIndex) ||
			(material->textureHandle != lastMaterialState_.textureHandle) ||
			(material->lightModelType != lastMaterialState_.lightModel);

		if (needSetMaterial) {
			// Set PSO
			PSODecision(*material);

			// Set Material
			SetMaterial(material->materialResourceHandle);

			// Set Texture
			int textureHandle = material->textureHandle;
			DirectX::TexMetadata checker = resourceManager_->GetTextureMetadata(textureHandle);
			D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = resourceManager_->GetTextureGPUDescriptorHandle(textureHandle);
			commandList_->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);

			// Update cache
			lastMaterialState_.materialIndex = materialIndex;
			lastMaterialState_.textureHandle = material->textureHandle;
			lastMaterialState_.lightModel = material->lightModelType;
			material->drawState = InstanceManager::ISDRAW;
		}

		std::unordered_map<int, std::vector<ModelInstance*>> groupedByModel;
		for (auto& ptr : group) {
			if (ptr->drawState == InstanceManager::STANDBY) {
				groupedByModel[ptr->modelHandle].push_back(ptr);
			}
		}

		for (auto& [modelHandle, instances] : groupedByModel) {
			// 設定 VBV/IBV
			//int meshHandle{};

			auto* mesh = resourceManager_->meshBufferList_[modelHandle];
			D3D12_VERTEX_BUFFER_VIEW vbv = mesh->GetVertexBufferView();
			D3D12_INDEX_BUFFER_VIEW ibv = mesh->GetIndexBufferView();
			commandList_->IASetVertexBuffers(0, 1, &vbv);
			commandList_->IASetIndexBuffer(&ibv);

			// instance 総数を計算
			int tileCount = static_cast<int>(instances.size());
			int wvpInstanceStartpoint = instance3DCounter_;

			// instancing bufferの書き込み
			for (int i = 0; i < tileCount; ++i) {

				tile3DInstancingData_[instance3DCounter_].WVP = instances[i]->transformData.WVP;
				tile3DInstancingData_[instance3DCounter_].world = instances[i]->transformData.world;
				tile3DInstancingData_[instance3DCounter_].WorldInverseTranspose = instances[i]->transformData.WorldInverseTranspose;

				instances[i]->drawState = InstanceManager::ISDRAW;
				instancesCounter++;
				instance3DCounter_++;
			}

			// OffsetData設定 
			OffsetData* inUse = instanceOffsetData_[offsetDataCounter_];
			*instanceOffsetData_[offsetDataCounter_]->instanceOffset = static_cast<UINT>(wvpInstanceStartpoint);
			instanceOffsetData_[offsetDataCounter_]->state = 1;

			commandList_->SetGraphicsRootConstantBufferView(4, inUse->instanceOffsetResource->GetResource()->GetGPUVirtualAddress());
			commandList_->SetGraphicsRootDescriptorTable(1, Tile3DSrvHandleGPU_);
			commandList_->SetGraphicsRootConstantBufferView(3, resourceManager_->lightingResource_->GetResource()->GetGPUVirtualAddress());

			// 一次性繪製
			if (ibv.SizeInBytes == 0)commandList_->DrawInstanced(instances[0]->vertexNum, tileCount, 0, 0);
			else commandList_->DrawIndexedInstanced(instances[0]->vertexNum, tileCount, 0, 0, 0);
			offsetDataCounter_++;
		}
	}
}

void DrawEngine::DrawCall() {
	Draw3D();
	Draw2D();
}

void DrawEngine::SetCamera(Camera* camera) {
	instanceCamera_ = camera;
}

bool DrawEngine::SetModelTexture(Model* model) {
	if (!model->GetTexturePatch().empty()) {
		model->SetTextureHandle(LoadModelTexture(model->GetTexturePatch()));
		return true;
	}
	model->SetTextureHandle(defaultTextureHandle_ + 1);
	return false;
}

int DrawEngine::SetModel(std::string Path) {

	resourceManager_->TextuerCounterAdjust(descriptorIndex_);
	int handle = resourceManager_->LoadModel(Path);
	descriptorIndex_ = resourceManager_->GetTextureCounter();

	return handle;
}

int DrawEngine::readCommonTextureHandle(int handle) {
	return resourceManager_->GetTextureHandleFromCommonList(handle);
	//return commonTextureSRVMap_[handle];
}

int DrawEngine::GetModelTextureHandle(int modelHandle, int part) {
	return resourceManager_->GetTextureHandleFromModelGroup(modelHandle, part);
}

int DrawEngine::readModelTextureHandle(int handle) {
	return resourceManager_->ReadModelTextureHandle(handle);
	//return modelTextureSRVMap_[handle];
}

int DrawEngine::LoadModelTexture(const std::string& filePath) {
	return resourceManager_->LoadCommonTexture(filePath);
}

D3D12_VIEWPORT DrawEngine::createViewport(int kClientWidth, int kClientHeight) {
	viewport.Width = (float)kClientWidth;
	viewport.Height = (float)kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	return viewport;
}

D3D12_RECT DrawEngine::createScissorRect(int kClientWidth, int kClientHeight) {
	// シザー矩形
	//D3D12_RECT scissorRect{};
	// 基本的にビューポートと同じ矩形が構成されるようにする
	scissorRect.left = 0;
	scissorRect.right = kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = kClientHeight;

	return scissorRect;
}

void DrawEngine::SetMaterial(int MaterialHandle) {
	// materialの色
	if (resourceManager_->materialResourceList_[MaterialHandle] == nullptr) {
		Logger::Log("Material CBuffer is not created");
		return;
	}
	commandList_->SetGraphicsRootConstantBufferView(0, resourceManager_->materialResourceList_[MaterialHandle]->GetResource()->GetGPUVirtualAddress());
}

void DrawEngine::SetCameraForGPU() {

	if (instanceCamera_) {
		cameraPtr_->worldPosition = instanceCamera_->GetTransform().translate;
	} else {
		cameraPtr_->worldPosition = defaultCamera_->GetTransform().translate;
	}

	commandList_->SetGraphicsRootConstantBufferView(5, cameraBuffer_->GetResource()->GetGPUVirtualAddress());
}

void DrawEngine::InitializeLighting() {

	// マテリアルにデータを書き込む
	resourceManager_->lightingResource_->CreateResourceClass_(directXDriver_->GetDevice(), sizeof(DirectionalLight));
	resourceManager_->lightingResource_->GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&lightingData));
}

void DrawEngine::SetLighting(DirectionalLight* directionalLight) {
	// Lightingにデータを書き込む

	*lightingData = *directionalLight;
	lightingData->direction = Normalize(lightingData->direction);
}

ID3D12Resource* DrawEngine::CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height) {

	/// 1.生成するResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;                                      // Textureの幅
	resourceDesc.Height = height;									 // Textureの高さ
	resourceDesc.MipLevels = 1;										 // mipmapの数
	resourceDesc.DepthOrArraySize = 1;								 // 奥行き or 配列Textureの配列数
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;			 // DepthStencilとして利用可能なフォーマット
	resourceDesc.SampleDesc.Count = 1;								 // サンプリングカウント。1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;	 // 2次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;	 // DepthStencilとして使う通知


	/// 2.利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;// VRAM上に作る

	/// EX1.深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f; //1.0f (最大値)でクリア
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;// フォーマット。Resourceと合わせる

	/// 3.Resourceを生成する
	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,					// Heapの設定
		D3D12_HEAP_FLAG_NONE,				// Heapの特殊な設定。特になし。
		&resourceDesc,						// Resourceの設定
		D3D12_RESOURCE_STATE_DEPTH_WRITE,	// 深度値を書き込む状態にしてお
		&depthClearValue,					// Clear最適値
		IID_PPV_ARGS(&resource));			// 作成するResourceポインタへのポインタ
	assert(SUCCEEDED(hr));

	char buffer[128];
	sprintf_s(buffer, "Create resource at %p\n", resource);
	OutputDebugStringA(buffer);

	return resource;
}

void DrawEngine::MakeDepthStencilView() {
	///DSV構築
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	// DSVHeap先頭にDSVをつくる
	directXDriver_->GetDevice()->CreateDepthStencilView(
		depthStencilResource, &dsvDesc, directXDriver_->GetDsvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart()
	);
}



