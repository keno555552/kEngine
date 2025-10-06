#include "drawEngine.h"
#include "Transform.h"
#include <math.h>
#define M_PI 3.1415926f


DrawEngine::~DrawEngine() {
	delete shader_compile_;
	delete pso_;

	for (auto& i : PSO_) {
		if (i) {
			i->Release();
			i = nullptr;
		}
	}
	PSO_.clear();

	//delete directXDriver_;            /*借り*/

	//dxcUtils->Release();				/*借り*/
	//dxcCompiler->Release();			/*借り*/
	//includeHandler->Release();		/*借り*/

	//rootSignature_->Release();		/*借り*/

	//delete directionalLightData;      /*借り*/

	delete resourceManager_;

	//materialResource_->Release();
	//materialResourceSprite_->Release();

	depthStencilResource->Release();

}

void DrawEngine::Initialize
(const char* kClientTitle, int kClientWidth, int kClientHeight, DirectXBase* directXDriver) {
	directXDriver_ = directXDriver;
	commandList_ = directXDriver_->GetCommandList();
	resourceManager_ = new ResourceManager(directXDriver_->GetDriver());
	///
	kClientWidth_ = kClientWidth;
	kClientHeight_ = kClientHeight;

	///
	shader_compile_->Initialize();
	dxcUtils = shader_compile_->getDxcUtils();
	dxcCompiler = shader_compile_->getDxcCompiler();
	includeHandler = shader_compile_->getIncludeHandler();
	///
	pso_->Initialize(directXDriver_);
	for (int i = 0; i < (int)LightModelType::NumLightModels; i++) {
		ID3D12PipelineState* graphicsPipelineState_ = pso_->createPSO(dxcUtils, dxcCompiler, includeHandler, (LightModelType)i);
		PSO_.push_back(graphicsPipelineState_);
	}
	rootSignature_ = pso_->getRootSignature();

	depthStencilResource = CreateDepthStencilTextureResource(directXDriver_->GetDriver(), kClientWidth, kClientHeight);
	MakeDepthStencilView();
	///
	viewport = createViewport(kClientWidth, kClientHeight);
	scissorRect = createScissorRect(kClientWidth, kClientHeight);

	/// デフォルトのモデルを設定
	SetModel("resources/object/plane/plane.obj");
	defaultTextureHandle_ = LoadModelTexture("resources/white5x5.png");
}

void DrawEngine::PreDraw() {
	commandList_->RSSetViewports(1, &viewport);  // Viewportを設定
	commandList_->RSSetScissorRects(1, &scissorRect);  // Scissorを設定
	/// RootSignatureを設定。PSOに設定しているけど別途設定が必要
	commandList_->SetGraphicsRootSignature(rootSignature_);
	if (currentLightModel_ == defaultLightModel_) {
		commandList_->SetPipelineState(PSO_[(int)defaultLightModel_]);  // PSOを設定
		currentLightModel_ = defaultLightModel_;
	} else {
		commandList_->SetPipelineState(PSO_[(int)currentLightModel_]);
	}
	/// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	/// 各種のリソースを設定
	resourceManager_->CreateTurnResource();

	/// Lighting
	SetLighting(resourceManager_->lightingResource_, directionalLightData);
}

void DrawEngine::SetDirectionalLight(DirectionalLight* light) {
	if (light)directionalLightData = light;
}

void DrawEngine::DrawTriangle(TransformationMatrix* wvpData, MaterialConfig material) {
	/// SetPSO
	if (material.lightModelType != currentLightModel_) {
		commandList_->SetPipelineState(PSO_[(int)material.lightModelType]);
		currentLightModel_ = material.lightModelType;
	}

	/// SetMaterial
	resourceManager_->materialResourceTriangle_->CreateResource_(directXDriver_->GetDriver(), sizeof(Material));
	SetMaterial(resourceManager_->materialResourceTriangle_->GetResource(), material.uvTransformMatrix, material.textureColor, material.enableLighting);
	commandList_->SetGraphicsRootConstantBufferView(0, resourceManager_->materialResourceTriangle_->GetResource()->GetGPUVirtualAddress());
	/// textrue設定
	int materialHandle = readCommonTextureHandle(material.textureHandle);
	textureSrvHandleGPU_ = directXDriver_->GetGPUDescriptorHandle(directXDriver_->GetSrvDescriptorHeap(), directXDriver_->GetDesriptorSizeSRV(), materialHandle);
	commandList_->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);
	/// VBVを設定
	D3D12_VERTEX_BUFFER_VIEW BufferView = resourceManager_->vertexResourceTriangle_->GetVertexBufferView();
	commandList_->IASetVertexBuffers(0, 1, &BufferView);
	/// wvp用のCBufferの場所を設定
	resourceManager_->vertexResourceTriangle_->SetWVPResource_(directXDriver_->GetDriver(), wvpData);
	commandList_->SetGraphicsRootConstantBufferView(1, resourceManager_->vertexResourceTriangle_->GetWVPResource_()->GetGPUVirtualAddress());
	/// 描画！(DrawCall/ドローコール)。
	commandList_->SetGraphicsRootConstantBufferView(3, resourceManager_->lightingResource_->GetGPUVirtualAddress());
	commandList_->DrawInstanced(6, 1, 0, 0);
}

void DrawEngine::CollectSprite(Vector2 pos, MaterialConfig material) {
	resourceManager_->AddSpriteInstance(pos, material);
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
	//			/// textrue設定
	//			int materialHandle = readCommonTextureHandle(nowMaterial->textureHandle);
	//			textureSrvHandleGPU_ = directXDriver_->GetGPUDescriptorHandle(directXDriver_->GetSrvDescriptorHeap(), directXDriver_->GetDesriptorSizeSRV(), materialHandle);
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

	struct MaterialStateCache {
		int materialIndex = -1;
		int textureHandle = -1;
		LightModelType lightModel = LightModelType::HalfLambert;
	};

	MaterialStateCache lastMaterialState_;

	std::unordered_map<int, std::vector<ResourceManager::SpriteInstance*>> groupedSprites;

	for (auto ptr : resourceManager_->spriteList_) {
		if (!ptr->isDraw) {
			groupedSprites[ptr->materialConfigIndex].push_back(ptr);
		}
	}

	for (auto& [materialIndex, group] : groupedSprites) {
		MaterialConfig* material = resourceManager_->materialConfigList_[materialIndex];

		bool needSetMaterial =
			materialIndex != lastMaterialState_.materialIndex ||
			material->textureHandle != lastMaterialState_.textureHandle ||
			material->lightModelType != lastMaterialState_.lightModel;

		if (needSetMaterial) {
			// Set PSO
			commandList_->SetPipelineState(PSO_[(int)material->lightModelType]);
			currentLightModel_ = material->lightModelType;

			// Set Material
			SetMaterial(resourceManager_->materialResourceSprite_->GetResource(), material->uvTransformMatrix, material->textureColor, false);
			commandList_->SetGraphicsRootConstantBufferView(0, resourceManager_->materialResourceSprite_->GetResource()->GetGPUVirtualAddress());

			// Set Texture
			int materialHandle = readCommonTextureHandle(material->textureHandle);
			textureSrvHandleGPU_ = directXDriver_->GetGPUDescriptorHandle(directXDriver_->GetSrvDescriptorHeap(), directXDriver_->GetDesriptorSizeSRV(), materialHandle);
			commandList_->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);

			// Update cache
			lastMaterialState_.materialIndex = materialIndex;
			lastMaterialState_.textureHandle = material->textureHandle;
			lastMaterialState_.lightModel = material->lightModelType;
		}

		// Set VB/IB
		D3D12_VERTEX_BUFFER_VIEW VertexBufferView = resourceManager_->vertexResourceSpriteGroup_[0]->GetVertexBufferView();
		commandList_->IASetVertexBuffers(0, 1, &VertexBufferView);
		D3D12_INDEX_BUFFER_VIEW IndexBufferView = resourceManager_->vertexResourceSpriteGroup_[0]->GetIndexBufferView();
		commandList_->IASetIndexBuffer(&IndexBufferView);

		// Draw all sprites with same material
		for (auto ptr : group) {
			resourceManager_->vertexResourceSpriteGroup_[0]->SetWVPResource_(directXDriver_->GetDriver(), ptr->position);
			commandList_->SetGraphicsRootConstantBufferView(1, resourceManager_->vertexResourceSpriteGroup_[0]->GetWVPResource_()->GetGPUVirtualAddress());
			commandList_->SetGraphicsRootConstantBufferView(3, resourceManager_->lightingResource_->GetGPUVirtualAddress());
			commandList_->DrawIndexedInstanced(12, 1, 0, 0, 0);
			ptr->isDraw = true;
		}
	}
}

void DrawEngine::DrawSpriteDirect(Vector2 pos, MaterialConfig material) {
	/// SetPSO
	if (material.lightModelType != currentLightModel_) {
		commandList_->SetPipelineState(PSO_[(int)material.lightModelType]);
		currentLightModel_ = material.lightModelType;
	}

	/// SetMaterial
	//resourceManager_->materialResourceSprite_->CreateResource_(directXDriver_->GetDriver(), sizeof(Material));
	SetMaterial(resourceManager_->materialResourceSprite_->GetResource(), material.uvTransformMatrix, material.textureColor, false);
	commandList_->SetGraphicsRootConstantBufferView(0, resourceManager_->materialResourceSprite_->GetResource()->GetGPUVirtualAddress());
	/// textrue設定
	int materialHandle = readCommonTextureHandle(material.textureHandle);
	textureSrvHandleGPU_ = directXDriver_->GetGPUDescriptorHandle(directXDriver_->GetSrvDescriptorHeap(), directXDriver_->GetDesriptorSizeSRV(), materialHandle);
	commandList_->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);
	/// Spriteの描画。変更が必要なものだけ変更する9652.
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView = resourceManager_->vertexResourceSpriteGroup_[0]->GetVertexBufferView();
	commandList_->IASetVertexBuffers(0, 1, &VertexBufferView);
	/// IBVを設定
	D3D12_INDEX_BUFFER_VIEW IndexBufferView = resourceManager_->vertexResourceSpriteGroup_[0]->GetIndexBufferView();
	commandList_->IASetIndexBuffer(&IndexBufferView);
	/// wvp用のCBufferの場所を設定
	resourceManager_->vertexResourceSpriteGroup_[0]->SetWVPResource_(directXDriver_->GetDriver(), pos);
	/// TransformationMatrixCBufferの場所を設定
	commandList_->SetGraphicsRootConstantBufferView(1, resourceManager_->vertexResourceSpriteGroup_[0]->GetWVPResource_()->GetGPUVirtualAddress());
	/// 描画! (DrawCall)

	commandList_->SetGraphicsRootConstantBufferView(3, resourceManager_->lightingResource_->GetGPUVirtualAddress());
	commandList_->DrawIndexedInstanced(12, 1, 0, 0, 0);
}

void DrawEngine::DrawSpriteDirect(Vector2 pos, MaterialConfig material, Vector2 LTpos, Vector2 LBpos, Vector2 RTpos, Vector2 RBpos, float TsizeX, float TsizeY, Vector2 TCLTPos, Vector2 TCRBPos) {
	/// SetPSO
	if (material.lightModelType != currentLightModel_) {
		commandList_->SetPipelineState(PSO_[(int)material.lightModelType]);
		currentLightModel_ = material.lightModelType;
	}

	int vertexHandle = -1;
	if (!resourceManager_->vertexResourceSpriteGroup_.empty()) {
		for (int i = 0; i < resourceManager_->vertexResourceSpriteGroup_.size(); i++) {
			if (resourceManager_->vertexResourceSpriteGroup_[i]->CheckSize(LTpos, LBpos, RTpos, RBpos, TsizeX, TsizeY, TCLTPos, TCRBPos)) {
				resourceManager_->vertexResourceSpriteGroup_[i]->SetKeep(true);
				vertexHandle = i;
				break;
			}
		}
	}
	if (vertexHandle == -1) {
		Sprite2D* sprite = new Sprite2D;
		sprite->SetSize(LTpos, LBpos, RTpos, RBpos, TsizeX, TsizeY, TCLTPos, TCRBPos);
		sprite->CreateVertexResource_(directXDriver_->GetDriver());
		sprite->CreateVertexBufferView_(5);
		sprite->CreateIndexResource_(directXDriver_->GetDriver());
		sprite->CreateIndexBufferView_(12);
		sprite->SetKeep(true);
		resourceManager_->vertexResourceSpriteGroup_.push_back(sprite);
		vertexHandle = int(resourceManager_->vertexResourceSpriteGroup_.size() - 1);
	}

	/// SetMaterial
	//resourceManager_->materialResourceSprite_->CreateResource_(directXDriver_->GetDriver(), sizeof(Material));
	SetMaterial(resourceManager_->materialResourceSprite_->GetResource(), material.uvTransformMatrix, material.textureColor, false);
	commandList_->SetGraphicsRootConstantBufferView(0, resourceManager_->materialResourceSprite_->GetResource()->GetGPUVirtualAddress());
	/// textrue設定
	int materialHandle = readCommonTextureHandle(material.textureHandle);
	textureSrvHandleGPU_ = directXDriver_->GetGPUDescriptorHandle(directXDriver_->GetSrvDescriptorHeap(), directXDriver_->GetDesriptorSizeSRV(), materialHandle);
	commandList_->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);
	/// Spriteの描画。変更が必要なものだけ変更する
	D3D12_VERTEX_BUFFER_VIEW VertexBufferView = resourceManager_->vertexResourceSpriteGroup_[vertexHandle]->GetVertexBufferView();
	commandList_->IASetVertexBuffers(0, 1, &VertexBufferView);
	/// IBVを設定
	D3D12_INDEX_BUFFER_VIEW IndexBufferView = resourceManager_->vertexResourceSpriteGroup_[vertexHandle]->GetIndexBufferView();
	commandList_->IASetIndexBuffer(&IndexBufferView);
	/// wvp用のCBufferの場所を設定
	resourceManager_->vertexResourceSpriteGroup_[vertexHandle]->SetWVPResource_(directXDriver_->GetDriver(), pos);
	/// TransformationMatrixCBufferの場所を設定
	commandList_->SetGraphicsRootConstantBufferView(1, resourceManager_->vertexResourceSpriteGroup_[vertexHandle]->GetWVPResource_()->GetGPUVirtualAddress());
	/// 描画! (DrawCall)
	commandList_->SetGraphicsRootConstantBufferView(3, resourceManager_->lightingResource_->GetGPUVirtualAddress());
	commandList_->DrawIndexedInstanced(12, 1, 0, 0, 0);
}

void DrawEngine::CollectTile(Vector2* pos, MaterialConfig material) {
	//resourceManager_->AddSpriteInstance(pos, material);
}

void DrawEngine::DrawTile(Vector2 pos, MaterialConfig material) {
	if (kParticleNumInstance_ == 0)return;

	kParticleNumInstance_ = 0;

	struct MaterialStateCache {
		int materialIndex = -1;
		int textureHandle = -1;
		LightModelType lightModel = LightModelType::HalfLambert;
	};

	MaterialStateCache lastMaterialState_;

	std::unordered_map<int, std::vector<ResourceManager::SpriteInstance*>> groupedSprites;

	for (auto ptr : resourceManager_->spriteList_) {
		if (!ptr->isDraw) {
			groupedSprites[ptr->materialConfigIndex].push_back(ptr);
		}
	}

	for (auto& [materialIndex, group] : groupedSprites) {
		MaterialConfig* material = resourceManager_->materialConfigList_[materialIndex];

		bool needSetMaterial =
			materialIndex != lastMaterialState_.materialIndex ||
			material->textureHandle != lastMaterialState_.textureHandle ||
			material->lightModelType != lastMaterialState_.lightModel;

		if (needSetMaterial) {
			// Set PSO
			commandList_->SetPipelineState(PSO_[(int)material->lightModelType]);
			currentLightModel_ = material->lightModelType;

			// Set Material
			SetMaterial(resourceManager_->materialResourceSprite_->GetResource(), material->uvTransformMatrix, material->textureColor, false);
			commandList_->SetGraphicsRootConstantBufferView(0, resourceManager_->materialResourceSprite_->GetResource()->GetGPUVirtualAddress());

			// Set Texture
			int materialHandle = readCommonTextureHandle(material->textureHandle);
			textureSrvHandleGPU_ = directXDriver_->GetGPUDescriptorHandle(directXDriver_->GetSrvDescriptorHeap(), directXDriver_->GetDesriptorSizeSRV(), materialHandle);
			commandList_->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);

			// Update cache
			lastMaterialState_.materialIndex = materialIndex;
			lastMaterialState_.textureHandle = material->textureHandle;
			lastMaterialState_.lightModel = material->lightModelType;
		}

		// Set VB/IB
		D3D12_VERTEX_BUFFER_VIEW VertexBufferView = resourceManager_->vertexResourceSpriteGroup_[0]->GetVertexBufferView();
		commandList_->IASetVertexBuffers(0, 1, &VertexBufferView);
		D3D12_INDEX_BUFFER_VIEW IndexBufferView = resourceManager_->vertexResourceSpriteGroup_[0]->GetIndexBufferView();
		commandList_->IASetIndexBuffer(&IndexBufferView);

		// Draw all sprites with same material
		for (auto ptr : group) {
			resourceManager_->vertexResourceSpriteGroup_[0]->SetWVPResource_(directXDriver_->GetDriver(), ptr->position);
			commandList_->SetGraphicsRootConstantBufferView(1, resourceManager_->vertexResourceSpriteGroup_[0]->GetWVPResource_()->GetGPUVirtualAddress());
			commandList_->SetGraphicsRootConstantBufferView(3, resourceManager_->lightingResource_->GetGPUVirtualAddress());
			commandList_->DrawIndexedInstanced(12, 1, 0, 0, 0);
			ptr->isDraw = true;
		}
	}
}

void DrawEngine::DrawCube(TransformationMatrix* wvpData, MaterialConfig material) {
	/// SetPSO
	if (material.lightModelType != currentLightModel_) {
		commandList_->SetPipelineState(PSO_[(int)material.lightModelType]);
		currentLightModel_ = material.lightModelType;
	}

	/// SetMaterial
	resourceManager_->materialResourceCube_->CreateResource_(directXDriver_->GetDriver(), sizeof(Material));
	SetMaterial(resourceManager_->materialResourceCube_->GetResource(), material.uvTransformMatrix, material.textureColor, material.enableLighting);
	commandList_->SetGraphicsRootConstantBufferView(0, resourceManager_->materialResourceCube_->GetResource()->GetGPUVirtualAddress());
	/// textrue設定
	int materialHandle = readCommonTextureHandle(material.textureHandle);
	textureSrvHandleGPU_ = directXDriver_->GetGPUDescriptorHandle(directXDriver_->GetSrvDescriptorHeap(), directXDriver_->GetDesriptorSizeSRV(), materialHandle);
	commandList_->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);
	/// VBVを設定
	D3D12_VERTEX_BUFFER_VIEW BufferView = resourceManager_->vertexResourceCube_->GetVertexBufferView();
	commandList_->IASetVertexBuffers(0, 1, &BufferView);
	/// IBVを設定
	D3D12_INDEX_BUFFER_VIEW IndexBufferView = resourceManager_->vertexResourceCube_->GetIndexBufferView();
	commandList_->IASetIndexBuffer(&IndexBufferView);
	/// wvp用のCBufferの場所を設定
	resourceManager_->vertexResourceCube_->SetWVPResource_(directXDriver_->GetDriver(), wvpData);
	commandList_->SetGraphicsRootConstantBufferView(1, resourceManager_->vertexResourceCube_->GetWVPResource_()->GetGPUVirtualAddress());
	/// 描画！(DrawCall/ドローコール)。
	commandList_->SetGraphicsRootConstantBufferView(3, resourceManager_->lightingResource_->GetGPUVirtualAddress());
	commandList_->DrawIndexedInstanced(36, 1, 0, 0, 0);
}

void DrawEngine::DrawSphere(TransformationMatrix* wvpData, MaterialConfig material, int sudivision) {
	sudivision;
	/// SetPSO
	if (material.lightModelType != currentLightModel_) {
		commandList_->SetPipelineState(PSO_[(int)material.lightModelType]);
		currentLightModel_ = material.lightModelType;
	}

	/// SetMaterial
	resourceManager_->materialResourceSphere_->CreateResource_(directXDriver_->GetDriver(), sizeof(Material));
	SetMaterial(resourceManager_->materialResourceSphere_->GetResource(), material.uvTransformMatrix, material.textureColor, material.enableLighting);
	commandList_->SetGraphicsRootConstantBufferView(0, resourceManager_->materialResourceSphere_->GetResource()->GetGPUVirtualAddress());
	/// textrue設定
	int materialHandle = readCommonTextureHandle(material.textureHandle);
	textureSrvHandleGPU_ = directXDriver_->GetGPUDescriptorHandle(directXDriver_->GetSrvDescriptorHeap(), directXDriver_->GetDesriptorSizeSRV(), materialHandle);
	commandList_->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);

	/// VBVを設定
	resourceManager_->vertexResourceSphere_->CreateVertexResource_(directXDriver_->GetDriver());

	D3D12_VERTEX_BUFFER_VIEW BufferView = resourceManager_->vertexResourceSphere_->GetVertexBufferView();
	commandList_->IASetVertexBuffers(0, 1, &BufferView);  // VBVを設定

	/// IBVを設定
	resourceManager_->vertexResourceSphere_->CreateIndexResource_(directXDriver_->GetDriver());
	D3D12_INDEX_BUFFER_VIEW IndexBufferView = resourceManager_->vertexResourceSphere_->GetIndexBufferView();
	commandList_->IASetIndexBuffer(&IndexBufferView);

	/// wvp用のCBufferの場所を設定
	resourceManager_->vertexResourceSphere_->SetWVPResource_(directXDriver_->GetDriver(), wvpData);
	commandList_->SetGraphicsRootConstantBufferView(1, resourceManager_->vertexResourceSphere_->GetWVPResource_()->GetGPUVirtualAddress());

	/// 描画
	commandList_->SetGraphicsRootConstantBufferView(3, resourceManager_->lightingResource_->GetGPUVirtualAddress());
	commandList_->DrawIndexedInstanced(resourceManager_->vertexResourceSphere_->GetVertexNum(), 1, 0, 0, 0);
}

void DrawEngine::DrawModel(TransformationMatrix* wvpData, std::vector<MaterialConfig> material, int modelHandle) {

	for (int i = 0; i < resourceManager_->vertexResourceModelGroup_[modelHandle]->GetModelNum(); i++) {

		/// SetPSO
		if (material[i].lightModelType != currentLightModel_) {
			commandList_->SetPipelineState(PSO_[(int)material[i].lightModelType]);
			//currentLightModel_ = material[readModelTextureHandle(i)].lightModelType;
			currentLightModel_ = material[i].lightModelType;
		}
		/// SetMaterial
		resourceManager_->materialResourceModelGroup_[modelHandle]->CreateResource_(directXDriver_->GetDriver(), sizeof(Material));
		SetMaterial(resourceManager_->materialResourceModelGroup_[modelHandle]->GetResource(), material[i].uvTransformMatrix, material[i].textureColor, material[i].enableLighting);
		commandList_->SetGraphicsRootConstantBufferView(0, resourceManager_->materialResourceModelGroup_[modelHandle]->GetResource()->GetGPUVirtualAddress());
		/// textrue設定
		int materialHandle;
		if (!material[i].useOriginalTexture)materialHandle = readCommonTextureHandle(material[i].textureHandle);
		else materialHandle = readModelTextureHandle(resourceManager_->vertexResourceModelGroup_[modelHandle]->GetModel(i)->GetTextureHandle());
		textureSrvHandleGPU_ = directXDriver_->GetGPUDescriptorHandle(directXDriver_->GetSrvDescriptorHeap(), directXDriver_->GetDesriptorSizeSRV(), materialHandle);

		//textureSrvHandleGPU_ = directXDriver_->GetGPUDescriptorHandle(directXDriver_->GetSrvDescriptorHeap(), directXDriver_->GetDesriptorSizeSRV(), resourceManager_->vertexResourceModel_[modelHandle]->GetTextureHandle());
		commandList_->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);

		///// VBVを設定
		//resourceManager_->vertexResourceModel_[modelHandle]->CreateVertexResource_(directXDriver_->GetDriver());

		D3D12_VERTEX_BUFFER_VIEW BufferView = resourceManager_->vertexResourceModelGroup_[modelHandle]->GetModel(i)->GetVertexBufferView();
		commandList_->IASetVertexBuffers(0, 1, &BufferView);  // VBVを設定

		/// wvp用のCBufferの場所を設定
		resourceManager_->vertexResourceModelGroup_[modelHandle]->GetModel(i)->SetWVPResource_(directXDriver_->GetDriver(), wvpData);
		commandList_->SetGraphicsRootConstantBufferView(1, resourceManager_->vertexResourceModelGroup_[modelHandle]->GetModel(i)->GetWVPResource_()->GetGPUVirtualAddress());

		/// 描画
		commandList_->SetGraphicsRootConstantBufferView(3, resourceManager_->lightingResource_->GetGPUVirtualAddress());
		commandList_->DrawInstanced(resourceManager_->vertexResourceModelGroup_[modelHandle]->GetModel(i)->GetVertexNum(), 1, 0, 0);
	}
}

void DrawEngine::DrawModel(TransformationMatrix* wvpData, std::vector<MaterialConfig> material) {
	/// SetPSO
	if (material[0].lightModelType != currentLightModel_) {
		commandList_->SetPipelineState(PSO_[(int)material[0].lightModelType]);
		currentLightModel_ = material[0].lightModelType;
	}

	/// SetMaterial
	resourceManager_->materialResourceModelGroup_[0]->CreateResource_(directXDriver_->GetDriver(), sizeof(Material));
	SetMaterial(resourceManager_->materialResourceModelGroup_[0]->GetResource(), material[0].uvTransformMatrix, material[0].textureColor, material[0].enableLighting);
	commandList_->SetGraphicsRootConstantBufferView(0, resourceManager_->materialResourceModelGroup_[0]->GetResource()->GetGPUVirtualAddress());
	/// textrue設定
	int materialHandle;
	if (!material[0].useOriginalTexture)materialHandle = readCommonTextureHandle(material[0].textureHandle);
	else materialHandle = readModelTextureHandle(resourceManager_->vertexResourceModelGroup_[0]->GetModel(0)->GetTextureHandle());
	textureSrvHandleGPU_ = directXDriver_->GetGPUDescriptorHandle(directXDriver_->GetSrvDescriptorHeap(), directXDriver_->GetDesriptorSizeSRV(), materialHandle);
	commandList_->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU_);

	D3D12_VERTEX_BUFFER_VIEW BufferView = resourceManager_->vertexResourceModelGroup_[0]->GetModel(0)->GetVertexBufferView();
	commandList_->IASetVertexBuffers(0, 1, &BufferView);  // VBVを設定

	/// wvp用のCBufferの場所を設定
	resourceManager_->vertexResourceModelGroup_[0]->GetModel(0)->SetWVPResource_(directXDriver_->GetDriver(), wvpData);
	commandList_->SetGraphicsRootConstantBufferView(1, resourceManager_->vertexResourceModelGroup_[0]->GetModel(0)->GetWVPResource_()->GetGPUVirtualAddress());

	/// 描画
	commandList_->SetGraphicsRootConstantBufferView(3, resourceManager_->lightingResource_->GetGPUVirtualAddress());
	commandList_->DrawInstanced(resourceManager_->vertexResourceModelGroup_[0]->GetModel(0)->GetVertexNum(), 1, 0, 0);
}

bool DrawEngine::SetModelTexture(Model* model) {
	if (!model->GetTexturePatch().empty()) {
		model->SetTextureHandle(LoadModelTexture(model->GetTexturePatch()));
		return true;
	}
	model->SetTextureHandle(defaultTextureHandle_ + 1);
	return false;
}

bool DrawEngine::SetModelGroupTexture(Model* model) {
	if (!model->GetTexturePatch().empty()) {
		model->SetTextureHandle(LoadModelTexture(model->GetTexturePatch()));
		return true;
	}
	model->SetTextureHandle(defaultTextureHandle_ + 1);
	return false;
}

int DrawEngine::SetModel(std::string Path) {
	ModelGroup* modelGroup = new ModelGroup;
	modelGroup->SetModelObj(Path);

	if (!resourceManager_->vertexResourceModelGroup_.empty()) {
		for (int i = 0; i < resourceManager_->vertexResourceModelGroup_.size(); i++) {
			std::string checkPath = resourceManager_->vertexResourceModelGroup_[i]->GetDirectoryPath();
			std::string checkName = resourceManager_->vertexResourceModelGroup_[i]->GetObjName_();
			if (checkPath == modelGroup->GetDirectoryPath()) {
				if (checkName == modelGroup->GetObjName_()) {
					return i;
				}
			}
		}
	}
	std::vector<ModelData> modelList = LoadMuitObjFile(modelGroup->GetDirectoryPath(), modelGroup->GetObjName_());
	for (auto& ptr : modelList) {
		Model* model = new Model;
		model->GetModelData(ptr);
		model->SetModelObj(Path);
		model->CreateVertexResourceG_(directXDriver_->GetDriver());
		SetModelGroupTexture(model);
		BasicResource* modelResource = new BasicResource;
		resourceManager_->materialResourceModelGroup_.push_back(modelResource);
		modelGroup->pushModel(model);
		modelGroup->PushModelHandle(modelHandle_);
		modelHandle_++;
	}
	resourceManager_->vertexResourceModelGroup_.push_back(modelGroup);
	modelGroupHandle_++;
	return modelGroupHandle_ - 1;
}

int DrawEngine::readCommonTextureHandle(int handle) {
	return commonTextureSRVMap_[handle];
}

int DrawEngine::readModelTextureHandle(int handle) {
	return modelTextureSRVMap_[handle];
}

int DrawEngine::GetMuitModelNum(int modelHandle) {
	return resourceManager_->vertexResourceModelGroup_[modelHandle]->GetModelNum();
}


int DrawEngine::LoadTexture(const std::string& filePath) {
	DirectX::ScratchImage mipImage = LoadTextrueLow(filePath);
	if (!resourceManager_->commonTextureFilePath_.empty()) {
		for (int i = 0; i < resourceManager_->commonTextureFilePath_.size(); i++) {
			if (resourceManager_->commonTextureFilePath_[i] == filePath) {
				return i;
			}
		}
	}
	const DirectX::TexMetadata& metadata = mipImage.GetMetadata();
	ID3D12Resource* textrueResourceN = CreateTextureResource(directXDriver_->GetDriver(), metadata);
	ID3D12Resource* intermediateResourceN = UploadTextureData(textrueResourceN, mipImage, directXDriver_->GetDriver(), commandList_);
	resourceManager_->intermediateResource_->SaveResource_(intermediateResourceN);
	resourceManager_->textureResource_->SaveResource_(textrueResourceN);

	int handle = MakeTextureShaderResourceView(metadata, textrueResourceN);

	resourceManager_->commonTextureFilePath_.push_back(filePath);
	return handle;
}

int DrawEngine::LoadModelTexture(const std::string& filePath) {
	DirectX::ScratchImage mipImage = LoadTextrueLow(filePath);
	const DirectX::TexMetadata& metadata = mipImage.GetMetadata();
	ID3D12Resource* textrueResourceN = CreateTextureResource(directXDriver_->GetDriver(), metadata);
	ID3D12Resource* intermediateResourceN = UploadTextureData(textrueResourceN, mipImage, directXDriver_->GetDriver(), commandList_);
	resourceManager_->intermediateResource_->SaveResource_(intermediateResourceN);
	resourceManager_->textureResource_->SaveResource_(textrueResourceN);

	int handle = MakeModelShaderResourceView(metadata, textrueResourceN);
	return handle;
}

void DrawEngine::CompoDraw() {

	/// Sprite描画
	DrawSprite();

}

void DrawEngine::EndDraw() {


	/// 各種のリソースを解放
	resourceManager_->ClearTurnResource();
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

void DrawEngine::SetMaterial(ID3D12Resource* resource, Matrix4x4 uvTransform, Vector4 color, int isLighting) {
	// マテリアルにデータを書き込む
	Material* materialData{};

	// 書き込むためのアドレスを取得
	resource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));

	// materialの色
	materialData->color = color;
	materialData->enableLighting = isLighting;
	materialData->uvTransform = uvTransform;

	resource->Unmap(0, nullptr);
}

void DrawEngine::SetLighting(ID3D12Resource* resource, DirectionalLight* directionalLight) {
	// マテリアルにデータを書き込む
	DirectionalLight* lightingData{};

	resource->Map(0, nullptr, reinterpret_cast<void**>(&lightingData));

	*lightingData = *directionalLight;
	lightingData->direction = Normalize(lightingData->direction);

	resource->Unmap(0, nullptr);
}

DirectX::ScratchImage DrawEngine::LoadTextrueLow(const std::string& filePath) {
	// テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr)); // 読み込めなかったらエラー

	// ミップマップを生成する
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(),
		DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr)); // 生成できなかったらエラー

	// ミップマップ付きのデータを返す
	return mipImages;
}

ID3D12Resource* DrawEngine::CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata) {

	/// 1. metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width); // Textureの幅
	resourceDesc.Height = UINT(metadata.height); // Textureの高さ
	resourceDesc.MipLevels = UINT16(metadata.mipLevels); // MipLevelの数
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize); // 奥行きor配列Textureの配列数
	resourceDesc.Format = metadata.format; // TextureのFormat
	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント。1固定。
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension); // Textureの次元数。普段使ってるのは2次元

	/// 2.利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // VRAM上に作る
	//heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK; // WriteBackポリシーでCPUアクセス可能
	//heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0; //プロセッサの近くに配置

	/// 3.Resourceを生成する
	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,					// Heapの設定
		D3D12_HEAP_FLAG_NONE,				// Heapの特殊な設定。特になし。
		&resourceDesc,						// Resourceの設定
		D3D12_RESOURCE_STATE_COPY_DEST,		// 初回のResourceState。Textureは基本読むだけ
		nullptr,							// Clear最適値。使わないのでnullptr
		IID_PPV_ARGS(&resource));			// 作成するResourceポインタへのポインタ
	assert(SUCCEEDED(hr));

	return resource;
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
		D3D12_RESOURCE_STATE_DEPTH_WRITE,	// 深度値を書き込む状態にしておく
		&depthClearValue,					// Clear最適値
		IID_PPV_ARGS(&resource));			// 作成するResourceポインタへのポインタ
	assert(SUCCEEDED(hr));

	return resource;
}

[[nodiscard]]//<---c++属性,戻り値が無視されることを防ぐ
ID3D12Resource* DrawEngine::UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages, ID3D12Device* device, ID3D12GraphicsCommandList* commandList) {

	// PrepareUploadを使って、DirectX12用のSubresource配列を作成する
	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	DirectX::PrepareUpload(device, mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresources);
	// Subresource配列を使って、コピー元のIntermediateResourceに必要のサイズを計算する
	uint64_t intermediateSize = GetRequiredIntermediateSize(texture, 0, UINT(subresources.size()));
	// 計算したサイズでIntermediateResourceを作成する
	ID3D12Resource* intermediateResource = CreateResource(device, intermediateSize);
	// UpdataSubresourcesを使って、IntermediateResourceにSubresource配列を書き込み,textureに転送し頭む積む
	UpdateSubresources(commandList, texture, intermediateResource, 0, 0, UINT(subresources.size()), subresources.data());
	// Tetureへの転送後は利用できるよう、D3D12_RESOURCE_STATE_COPY_DESTからD3D12_RESOURCE_STATE_GENERIC_READへResourceStateを変更する
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandList->ResourceBarrier(1, &barrier);
	return intermediateResource;
}

int DrawEngine::MakeTextureShaderResourceView(const DirectX::TexMetadata& metadata, ID3D12Resource* textureResource) {
	//
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; // シェーダーでのコンポーネントマッピング
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels); // 最初のMipLevelを使用

	//
	//textureSrvHandleCPU_ = directXDriver_->GetSrvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	//textureSrvHandleGPU_ = directXDriver_->GetSrvDescriptorHeap()->GetGPUDescriptorHandleForHeapStart();
	textureSrvHandleCPU_ = directXDriver_->GetCPUDescriptorHandle(directXDriver_->GetSrvDescriptorHeap(), directXDriver_->GetDesriptorSizeSRV(), textrueCounter);
	textureSrvHandleGPU_ = directXDriver_->GetGPUDescriptorHandle(directXDriver_->GetSrvDescriptorHeap(), directXDriver_->GetDesriptorSizeSRV(), textrueCounter);

	//
	directXDriver_->GetDriver()->CreateShaderResourceView(
		textureResource,					// Resource
		&srvDesc,							// SRVの設定
		textureSrvHandleCPU_				// CPU用のハンドル
	);

	int counter = textrueCounter;
	commonTextureSRVMap_.push_back(counter);
	textrueCounter++;

	return (int)commonTextureSRVMap_.size() - 1;
}

int DrawEngine::MakeModelShaderResourceView(const DirectX::TexMetadata& metadata, ID3D12Resource* textureResource) {
	//
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING; // シェーダーでのコンポーネントマッピング
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels); // 最初のMipLevelを使用

	//
	//textureSrvHandleCPU_ = directXDriver_->GetSrvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	//textureSrvHandleGPU_ = directXDriver_->GetSrvDescriptorHeap()->GetGPUDescriptorHandleForHeapStart();
	textureSrvHandleCPU_ = directXDriver_->GetCPUDescriptorHandle(directXDriver_->GetSrvDescriptorHeap(), directXDriver_->GetDesriptorSizeSRV(), textrueCounter);
	textureSrvHandleGPU_ = directXDriver_->GetGPUDescriptorHandle(directXDriver_->GetSrvDescriptorHeap(), directXDriver_->GetDesriptorSizeSRV(), textrueCounter);

	//
	directXDriver_->GetDriver()->CreateShaderResourceView(
		textureResource,					// Resource
		&srvDesc,							// SRVの設定
		textureSrvHandleCPU_				// CPU用のハンドル
	);

	int counter = textrueCounter;
	modelTextureSRVMap_.push_back(counter);
	textrueCounter++;

	return (int)modelTextureSRVMap_.size() - 1;
}

int DrawEngine::MakeParticleShaderResourceView(const DirectX::TexMetadata& metadata, ID3D12Resource* textureResource) {
	//
	D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};
	instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	instancingSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	instancingSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	instancingSrvDesc.Buffer.FirstElement = 0;
	instancingSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	instancingSrvDesc.Buffer.NumElements = kParticleNumInstance_;
	instancingSrvDesc.Buffer.StructureByteStride = sizeof(TransformationMatrix);

	//
	//textureSrvHandleCPU_ = directXDriver_->GetSrvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	//textureSrvHandleGPU_ = directXDriver_->GetSrvDescriptorHeap()->GetGPUDescriptorHandleForHeapStart();
	textureSrvHandleCPU_ = directXDriver_->GetCPUDescriptorHandle(directXDriver_->GetSrvDescriptorHeap(), directXDriver_->GetDesriptorSizeSRV(), textrueCounter);
	textureSrvHandleGPU_ = directXDriver_->GetGPUDescriptorHandle(directXDriver_->GetSrvDescriptorHeap(), directXDriver_->GetDesriptorSizeSRV(), textrueCounter);

	//
	directXDriver_->GetDriver()->CreateShaderResourceView(
		textureResource,					// Resource
		&instancingSrvDesc,					// SRVの設定
		textureSrvHandleCPU_				// CPU用のハンドル
	);

	int counter = textrueCounter;
	modelTextureSRVMap_.push_back(counter);
	textrueCounter++;

	return (int)modelTextureSRVMap_.size() - 1;
}

void DrawEngine::MakeDepthStencilView() {
	///DSV構築
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	// DSVHeap先頭にDSVをつくる
	directXDriver_->GetDriver()->CreateDepthStencilView(
		depthStencilResource, &dsvDesc, directXDriver_->GetDsvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart()
	);
}


