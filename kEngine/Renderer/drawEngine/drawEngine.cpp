#include "drawEngine.h"
#include "Transform.h"
#include <math.h>
#include "Vector3.h"
#include "Logger.h"

#include "Queue/RenderData.h"
#include <vector>
#include <Types/PSOType/RenderTargetFormat.h>
#include "PSOManager/RootSlot.h"

void DrawEngine::Initialize(
	DirectXCore* directXDriver,
	DrawDataCollector* drawDataCollector,
	PostProcessRunner* postProcessRunner
) {
	directXDriver_ = directXDriver;
	commandList_ = directXDriver_->GetCommandList();
	srvManager_ = SrvManager::GetInstance();
	resourceManager_ = ResourceManager::GetInstance();
	drawDataCollector_ = drawDataCollector;
	postProcessRunner_ = postProcessRunner;

	///
	kClientWidth_ = config::GetClientWidth();
	kClientHeight_ = config::GetClientHeight();


	/// ================== PSO関連 ================== ///
	psoManager_ = std::make_unique<PSOManager>();
	psoManager_->Initialize(directXDriver_);

	depthStencilResource = resourceManager_->CreateDepthStencilTextureResource(directXDriver_->GetDevice(), kClientWidth_, kClientHeight_);
	MakeDepthStencilView();


	viewport = createViewport(kClientWidth_, kClientHeight_);
	scissorRect = createScissorRect(kClientWidth_, kClientHeight_);

	/// Lighting

	int numOfLight = config::GetMaxLightNum();

	lightBuffer_ = std::make_unique<InstanceBuffer<LightGPU>>(directXDriver_);
	lightListData_ = lightBuffer_->CreateInstanceBuffer(numOfLight);
	lightBuffer_->GetResource()->SetName("LightBuffer");

	/// ===================== Material用InstanceBufferを作成 ======================== ///
	int numOfMaterial = config::GetMaxMaterialNum();
	materialResource_ = std::make_unique<InstanceBuffer<MaterialForGPU>>(directXDriver_);
	MaterialForGPU* instanceListPtrMaterial = materialResource_->CreateInstanceBuffer(numOfMaterial);
	drawDataCollector_->SetInstanceListMaterial(instanceListPtrMaterial);

	materialResource_->GetResource()->SetName("MaterialStructuredBuffer");


	/// ======================= Tile用InstanceBufferを作成 ========================= ///
	/// ============== DebugLine

	int numInstanceDL = config::GetDebugLineNumInstance();

	/// DebugLine用のインスタンスバッファを作成して、DrawDataCollectorに渡す
	debugLineResource_ = std::make_unique<InstanceBuffer<TransformationMatrix>>(directXDriver_);
	TransformationMatrix* instanceListPtrDL = debugLineResource_->CreateInstanceBuffer(numInstanceDL);
	drawDataCollector_->SetInstanceListDL(instanceListPtrDL);

	debugLineResource_->GetResource()->SetName("DebugLineWVPBuffer");

	/// DebugLine用のインスタンスバッファを初期化
	IntializeInstanceTMBuffer(instanceListPtrDL, (size_t)numInstanceDL);


	/// ============== 2Dタイル用WVPバッファ

	int numInstance2D = config::Get2DTileNumInstance();

	/// DebugLine用のインスタンスバッファを作成して、DrawDataCollectorに渡す
	tile2DWVPResource_ = std::make_unique<InstanceBuffer<TransformationMatrix>>(directXDriver_);
	TransformationMatrix* instanceListPtr2D = tile2DWVPResource_->CreateInstanceBuffer(numInstance2D);
	drawDataCollector_->SetInstanceList2D(instanceListPtr2D);

	tile2DWVPResource_->GetResource()->SetName("2DTileWVPBuffer");

	/// DebugLine用のインスタンスバッファを初期化
	IntializeInstanceTMBuffer(instanceListPtr2D, (size_t)numInstance2D);


	/// ============== 3Dタイル用WVPバッファ

	int numInstance3D = config::Get3DTileNumInstance();

	/// DebugLine用のインスタンスバッファを作成して、DrawDataCollectorに渡す
	tile3DWVPResource_ = std::make_unique<InstanceBuffer<TransformationMatrix>>(directXDriver_);
	TransformationMatrix* instanceListPtr3D = tile3DWVPResource_->CreateInstanceBuffer(numInstance3D);
	drawDataCollector_->SetInstanceList3D(instanceListPtr3D);

	tile3DWVPResource_->GetResource()->SetName("3DTileWVPBuffer");

	/// DebugLine用のインスタンスバッファを初期化
	IntializeInstanceTMBuffer(instanceListPtr3D, (size_t)numInstance3D);

	/// ============== Particleタイル用WVPバッファ

	int numInstancePC = config::GetParticleCPUNumInstance();

	/// DebugLine用のインスタンスバッファを作成して、DrawDataCollectorに渡す
	tilePCWVPResource_ = std::make_unique<InstanceBuffer<TransformationMatrix>>(directXDriver_);
	TransformationMatrix* instanceListPtrPC = tilePCWVPResource_->CreateInstanceBuffer(numInstancePC);
	drawDataCollector_->SetInstanceListParticle(instanceListPtrPC);
	tilePCWVPResource_->GetResource()->SetName("ParticleWVPBuffer");

	/// DebugLine用のインスタンスバッファを初期化
	IntializeInstanceTMBuffer(instanceListPtrPC, (size_t)numInstancePC);


	/// ====================== InstanceOffset用バッファを作成 ======================= ///

	for (int i = 0; i < config::GetMaxOffsetNum(); i++) {

		/// BasicResourceを作って、OffsetDataのサイズに使って初期化する
		std::unique_ptr<BasicResource> offsetResource = std::make_unique<BasicResource>();
		offsetResource_.push_back(std::make_unique<BasicResource>());
		offsetResource_.back()->CreateResourceClass_(directXDriver_->GetDevice(), sizeof(OffsetData));

		/// OffsetDataを作って、mapします
		OffsetData* ptr = nullptr;
		offsetResource_.back()->GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&ptr));

		/// MapされたOffsetDataをリストに入れる
		instanceOffsetData_.push_back(ptr);

		/// OffsetDataを初期化
		ptr->Offset_WVP = {};
		ptr->Offset_MaterialIndexList = {};

		/// Resourceに名前をつく
		offsetResource_.back()->SetName("InstanceOffsetBuffer" + std::to_string(i));
	}

	/// ====================== MaterialIndexList用バッファを作成 ======================= ///
	materialIndexListResource_ = std::make_unique<InstanceBuffer<int>>(directXDriver_);
	materialIndexList_ = materialIndexListResource_->CreateInstanceBuffer(config::GetMaxMaterialNum());
	materialIndexListResource_->GetResource()->SetName("MaterialListIndexBuffer");


	/// =========================== OffscreenRT初期化 =========================== ///

	kernelDataResource_ = std::make_unique<InstanceBuffer<KernelDataGPU>>(directXDriver_);
	KernelDataGPU* instanceListBlurData = kernelDataResource_->CreateInstanceBuffer(5);
	postProcessRunner_->SetInstanceListBlurData(instanceListBlurData);
	kernelDataResource_->GetResource()->SetName("BlurDataBuffer");


	/// =========================== カメラバッファの初期化 =========================== ///
	cameraBuffer_ = std::make_unique<BasicResource>();
	cameraBuffer_->CreateResourceClass_(directXDriver_->GetDevice(), sizeof(CameraForGPU));
	cameraBuffer_->GetResource()->Map(0, nullptr, reinterpret_cast<void**>(&cameraPtr_));
	cameraBuffer_->SetName("CameraBuffer");

	/// ========================== デフォルトのモデルを設定 ========================== ///
	defaultTextureHandle_ = resourceManager_->LoadModelTexture("./kEngine/EngineAssets/TemplateResource/texture/white5x5.png");

}

void DrawEngine::Finalize() {

	psoManager_->Finalize();
	psoManager_.reset();

	materialResource_.reset();
	materialIndexListResource_.reset();
	debugLineResource_.reset();
	tile2DWVPResource_.reset();
	tile3DWVPResource_.reset();
	tilePCWVPResource_.reset();
	cameraBuffer_.reset();

	offsetResource_.clear();

	depthStencilResource.Reset();

	lightBuffer_.reset();

}

void DrawEngine::StartFrame() {

	/// 描画用のDescriptorHeapの設定
	SetSRVHeap();
	/// Viewportを設定
	commandList_->RSSetViewports(1, &viewport); 
	/// Scissorを設定
	commandList_->RSSetScissorRects(1, &scissorRect);

	/// InstanceCounterReset
	instance2DCounter_ = 0;
	instance3DCounter_ = 0;
	instancePCCounter_ = 0;
	offsetDataCounter_ = 0;
	instanceMaterialIndexCounter_ = 0;
}

void DrawEngine::PreDraw() {

	/// PSOReset
	// デフォルトのPSOをセット
	PSOKey defaultPSOKey = CreateDefaultPSOKey();
	psoManager_->SetPSOStrong(defaultPSOKey);

	/// Lighting
	drawDataCollector_->UpdateLightData();
	UpdateLighting();

	/// Set Camera
	SetCameraForGPU();


	/// ==================== OffscreenRT関連設定 ==================== ///
	postProcessRunner_->SetPreDraw(this);
}

void DrawEngine::CommitDraw() {

	/// 集まったデータで描画
	/// Sprite描画
	DrawCall();

}

void DrawEngine::EndDraw() {

	postProcessRunner_->Execute(this);
}

void DrawEngine::PSODecision(PSOKey& psoKey) {

	PSOKey psoKeyInt = psoKey;
	if (enviromentReflectionTextureHandle_ == -1) {
		psoKeyInt.featureMask &= ~(uint64_t)FeatureFlags::EnvReflection;
	}

	psoManager_->SetPSO(psoKeyInt);

	/// Topology設定
	switch (psoKeyInt.primitiveType) {
	case PrimitiveType::LINE:
		commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
		break;

	case PrimitiveType::TRIANGLE:
		commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		break;
	}
}

/// ======================================== Draw関数 ========================================== ///
/// -------------------------------------- DebugLine関連 --------------------------------------- ///

void DrawEngine::DrawDebugLine() {

	auto& vertices = drawDataCollector_->GetDebugLineVertexBucket();
	UINT lineCount = (UINT)vertices.size();

	if (vertices.empty()) return;

	/// TileSRV
	commandList_->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::TransformMatricesList_SB), debugLineResource_->GetGPUDescriptorHandle());

	/// ========== PSO ==========
	PSOKey psoKey = CreateDebugLinePSOKey();

	PSODecision(psoKey);

	/// ========== Camera CBV (b1) ==========
	SetCameraForGPU();

	/// ========== VertexBuffer ==========
	UpdateDebugLineVertexBuffer(vertices);
	commandList_->IASetVertexBuffers(0, 1, &debugLineVBView_);

	/// ========== Draw ==========
	commandList_->DrawInstanced((UINT)vertices.size(), 1, 0, 0);
}

/// ------------------------------------------ 2D関連 ------------------------------------------ ///

void DrawEngine::Draw2D() {

	if (drawDataCollector_->GetOpaqueBuckets2D().empty() &&
		drawDataCollector_->GetTransparentObjectParts2D().empty())return;

	Draw2DOpaque();
	Draw2DTransparent();

}

void DrawEngine::Draw2DTransparent() {

	auto& transparent2D_ = drawDataCollector_->GetTransparentObjectParts2D();

	/// TileSRV
	commandList_->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::TransformMatricesList_SB), tile2DWVPResource_->GetGPUDescriptorHandle());
	commandList_->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::MaterialList_SB), materialResource_->GetGPUDescriptorHandle());
	commandList_->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::MaterialIndexList_SB), materialIndexListResource_->GetGPUDescriptorHandle());

	for (auto& object : transparent2D_) {

		/// Set PSO
		PSODecision(object.psoKey);

		SetLightingGPU();

		commandList_->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::MaterialList_SB), materialResource_->GetGPUDescriptorHandle());

		SetTexture(object.textureHandle);

		/// MeshIndex 數量
		int meshIndexCount = object.mesh->GetIndexNum();

		/// VBV/IBV 設定 
		D3D12_VERTEX_BUFFER_VIEW vbv = object.mesh->GetVertexBufferView();
		D3D12_INDEX_BUFFER_VIEW ibv = object.mesh->GetIndexBufferView();
		commandList_->IASetVertexBuffers(0, 1, &vbv);
		commandList_->IASetIndexBuffer(&ibv);

		/// MaterialIndexList 設定
		if (instanceMaterialIndexCounter_ >= config::GetMaxMaterialNum())assert(false);
		int materialIndex = instanceMaterialIndexCounter_;
		materialIndexList_[materialIndex] = object.materialID;
		++instanceMaterialIndexCounter_;

		/// WVP 設定
		int instIdx = instance2DCounter_;
		++instance2DCounter_;

		/// OffsetData 設定
		if (offsetDataCounter_ >= offsetResource_.size()) assert(false);
		auto& resource = offsetResource_[offsetDataCounter_];
		auto& inUse = instanceOffsetData_[offsetDataCounter_];
		inUse->Offset_WVP = static_cast<UINT>(instIdx);
		inUse->Offset_MaterialIndexList = static_cast<UINT>(materialIndex);
		commandList_->SetGraphicsRootConstantBufferView(static_cast<UINT>(RootSlot::InstanceOffset_CB), resource->GetResource()->GetGPUVirtualAddress());


		/// Draw
		if (meshIndexCount != 0) {
			commandList_->DrawIndexedInstanced(meshIndexCount, 1, 0, 0, 0);
		} else {
			int meshVertexCount = object.mesh->GetVertexNum();
			commandList_->DrawInstanced(meshVertexCount, 1, 0, 0);
		}

		offsetDataCounter_++;
	}
}

void DrawEngine::Draw2DOpaque() {
	auto& transparentObjectParts2D_ = drawDataCollector_->GetOpaqueBuckets2D();

	/// TileSRV
	commandList_->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::TransformMatricesList_SB), tile2DWVPResource_->GetGPUDescriptorHandle());

	for (auto& [psoKey, materialBuckets] : transparentObjectParts2D_) {

		/// Set PSO
		PSOKey psoKeyInt = psoKey;
		PSODecision(psoKeyInt);

		for (auto& [meshBuffer, TextureHandleGroup] : materialBuckets) {

			if (TextureHandleGroup.empty()) continue;

			SetLightingGPU();

			/// インスタンスの開始位置を保存
			int instIdx = instance2DCounter_;


			/// VBV/IBV 設定
			D3D12_VERTEX_BUFFER_VIEW vbv = meshBuffer->GetVertexBufferView();
			D3D12_INDEX_BUFFER_VIEW ibv = meshBuffer->GetIndexBufferView();
			commandList_->IASetVertexBuffers(0, 1, &vbv);
			commandList_->IASetIndexBuffer(&ibv);

			for (auto& [TextureHandle, RenderDataGroup] : TextureHandleGroup) {
				commandList_->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::MaterialList_SB), materialResource_->GetGPUDescriptorHandle());

				SetTexture((int)TextureHandle);

				/// Instancing 用のデータを準備
				int instancesCounter = 0;

				/// MeshIndex數量
				int meshIndexCount = meshBuffer->GetIndexNum();

				/// 設定 offset
				int materialIndex = instanceMaterialIndexCounter_;

				if (offsetDataCounter_ >= offsetResource_.size()) assert(false);
				auto& resource = offsetResource_[offsetDataCounter_];
				auto& inUse = instanceOffsetData_[offsetDataCounter_];
				inUse->Offset_WVP = static_cast<UINT>(instIdx);
				inUse->Offset_MaterialIndexList = static_cast<UINT>(materialIndex);
				commandList_->SetGraphicsRootConstantBufferView(static_cast<UINT>(RootSlot::InstanceOffset_CB), resource->GetResource()->GetGPUVirtualAddress());

				for (auto& RenderData : RenderDataGroup) {


					/// MaterialIndexList 設定
					if (instanceMaterialIndexCounter_ >= config::GetMaxMaterialNum())assert(false);
					materialIndexList_[instanceMaterialIndexCounter_] = RenderData.materialID;
					++instanceMaterialIndexCounter_;

					/// WVP計算
					instancesCounter++;
					instance2DCounter_++;


				}
				offsetDataCounter_++;
				if (meshIndexCount != 0) {
					commandList_->DrawIndexedInstanced(meshIndexCount, instancesCounter, 0, 0, 0);
				} else {
					int meshVertexCount = meshBuffer->GetVertexNum();
					commandList_->DrawInstanced(meshVertexCount, instancesCounter, 0, 0);
				}
			}
		}
	}
}

/// ------------------------------------------ 3D関連 ------------------------------------------ ///

void DrawEngine::Draw3D() {

	if (drawDataCollector_->GetOpaqueBuckets3D().empty() &&
		drawDataCollector_->GetTransparentObjectParts3D().empty())return;

	Draw3DOpaque();
	Draw3DTransparent();

}

void DrawEngine::Draw3DTransparent() {

	auto& transparent3D_ = drawDataCollector_->GetTransparentObjectParts3D();

	/// TileSRV
	commandList_->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::TransformMatricesList_SB), tile3DWVPResource_->GetGPUDescriptorHandle());
	commandList_->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::MaterialList_SB), materialResource_->GetGPUDescriptorHandle());
	commandList_->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::MaterialIndexList_SB), materialIndexListResource_->GetGPUDescriptorHandle());


	for (auto& object : transparent3D_) {

		/// Set PSO
		PSOKey psoKeyInt = object.psoKey;
		psoKeyInt.depthStencilType = DepthStencilType::ReadOnly;
		PSODecision(psoKeyInt);

		if (enviromentReflectionTextureHandle_ != -1) {
			SetEnviromentReflectionGPU();
		}

		SetLightingGPU();

		SetTexture(object.textureHandle);

		/// MeshIndex 數量
		int meshIndexCount = object.mesh->GetIndexNum();

		/// MaterialIndexList 設定
		if (instanceMaterialIndexCounter_ >= config::GetMaxMaterialNum())assert(false);
		int materialIndex = instanceMaterialIndexCounter_;
		materialIndexList_[materialIndex] = object.materialID;
		++instanceMaterialIndexCounter_;

		/// VBV/IBV 設定
		D3D12_VERTEX_BUFFER_VIEW vbv = object.mesh->GetVertexBufferView();
		D3D12_INDEX_BUFFER_VIEW ibv = object.mesh->GetIndexBufferView();
		commandList_->IASetVertexBuffers(0, 1, &vbv);
		commandList_->IASetIndexBuffer(&ibv);

		/// WVP 設定
		int instIdx = instance3DCounter_;
		++instance3DCounter_;

		/// 設定 offset
		if (offsetDataCounter_ >= offsetResource_.size()) assert(false);
		auto& resource = offsetResource_[offsetDataCounter_];
		auto& inUse = instanceOffsetData_[offsetDataCounter_];
		inUse->Offset_WVP = static_cast<UINT>(instIdx);
		inUse->Offset_MaterialIndexList = static_cast<UINT>(materialIndex);
		commandList_->SetGraphicsRootConstantBufferView(static_cast<UINT>(RootSlot::InstanceOffset_CB), resource->GetResource()->GetGPUVirtualAddress());


		/// Draw
		if (meshIndexCount != 0) {
			commandList_->DrawIndexedInstanced(meshIndexCount, 1, 0, 0, 0);
		} else {
			int meshVertexCount = object.mesh->GetVertexNum();
			commandList_->DrawInstanced(meshVertexCount, 1, 0, 0);
		}

		offsetDataCounter_++;
	}
}

void DrawEngine::Draw3DOpaque() {
	auto& transparentObjectParts3D_ = drawDataCollector_->GetOpaqueBuckets3D();

	/// TileSRV
	commandList_->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::TransformMatricesList_SB), tile3DWVPResource_->GetGPUDescriptorHandle());
	commandList_->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::MaterialList_SB), materialResource_->GetGPUDescriptorHandle());
	commandList_->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::MaterialIndexList_SB), materialIndexListResource_->GetGPUDescriptorHandle());

	for (auto& [psoKey, materialBuckets] : transparentObjectParts3D_) {

		/// Set PSO
		PSOKey psoKeyInt = psoKey;
		PSODecision(psoKeyInt);

		if (enviromentReflectionTextureHandle_ != -1) {
			SetEnviromentReflectionGPU();
		}

		for (auto& [meshBuffer, TextureHandleGroup] : materialBuckets) {

			if (TextureHandleGroup.empty()) continue;

			SetLightingGPU();

			/// VBV/IBV 設定
			D3D12_VERTEX_BUFFER_VIEW vbv = meshBuffer->GetVertexBufferView();
			D3D12_INDEX_BUFFER_VIEW ibv = meshBuffer->GetIndexBufferView();
			commandList_->IASetVertexBuffers(0, 1, &vbv);
			commandList_->IASetIndexBuffer(&ibv);

			for (auto& [TextureHandle, RenderDataGroup] : TextureHandleGroup) {

				SetTexture((int)TextureHandle);

				/// Instancing 用のデータを準備
				int instancesCounter = 0;

				/// 設定 offset
				int instIdx = instance3DCounter_;
				int materialIndex = instanceMaterialIndexCounter_;

				if (offsetDataCounter_ >= offsetResource_.size()) assert(false);
				auto& resource = offsetResource_[offsetDataCounter_];
				auto& inUse = instanceOffsetData_[offsetDataCounter_];
				inUse->Offset_WVP = static_cast<UINT>(instIdx);
				inUse->Offset_MaterialIndexList = static_cast<UINT>(materialIndex);
				commandList_->SetGraphicsRootConstantBufferView(static_cast<UINT>(RootSlot::InstanceOffset_CB), resource->GetResource()->GetGPUVirtualAddress());

				/// MeshIndex數量
				int meshIndexCount = meshBuffer->GetIndexNum();

				for (auto& RenderData : RenderDataGroup) {

					/// MaterialIndexList 設定
					if (instanceMaterialIndexCounter_ >= config::GetMaxMaterialNum())assert(false);
					materialIndexList_[instanceMaterialIndexCounter_] = RenderData.materialID;
					++instanceMaterialIndexCounter_;

					/// WVPInstance校正
					instancesCounter++;
					instance3DCounter_++;
				}
				offsetDataCounter_++;


				if (meshIndexCount != 0) {
					commandList_->DrawIndexedInstanced(meshIndexCount, instancesCounter, 0, 0, 0);
				} else {
					int meshVertexCount_ = meshBuffer->GetVertexNum();
					commandList_->DrawInstanced(meshVertexCount_, instancesCounter, 0, 0);
				}
			}
		}
	}
}

/// --------------------------------------- Particle関連 --------------------------------------- ///

void DrawEngine::DrawParticle() {

	auto& objectBucket_ = drawDataCollector_->GetParticleBucket();

	/// TileSRV
	commandList_->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::TransformMatricesList_SB), tilePCWVPResource_->GetGPUDescriptorHandle());
	commandList_->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::MaterialList_SB), materialResource_->GetGPUDescriptorHandle());
	commandList_->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::MaterialIndexList_SB), materialIndexListResource_->GetGPUDescriptorHandle());

	for (auto& [psoKey, materialBuckets] : objectBucket_) {

		/// Set PSO
		PSOKey psoKeyInt = psoKey;
		PSODecision(psoKeyInt);

		if (enviromentReflectionTextureHandle_ != -1) {
			SetEnviromentReflectionGPU();
		}

		for (auto& [MeshBuffer, TextureHandleGroup] : materialBuckets) {

			/// Meshを呼び出す
			auto mesh = MeshBuffer;

			if (enviromentReflectionTextureHandle_ != -1) {
				SetEnviromentReflectionGPU();
			}

			if (TextureHandleGroup.empty()) continue;

			SetLightingGPU();

			/// VBV/IBV 設定
			D3D12_VERTEX_BUFFER_VIEW vbv = mesh->GetVertexBufferView();
			D3D12_INDEX_BUFFER_VIEW ibv = mesh->GetIndexBufferView();
			commandList_->IASetVertexBuffers(0, 1, &vbv);
			commandList_->IASetIndexBuffer(&ibv);


			for (auto& [TextureHandle, RenderDataGroup] : TextureHandleGroup) {


				SetTexture(TextureHandle);

				/// Instancing 用のデータを準備
				int instancesCounter = 0;

				/// MeshIndex 数設定
				int meshIndexCount = mesh->GetIndexNum();

				/// 設定 offset
				if (offsetDataCounter_ >= offsetResource_.size()) assert(false);
				auto& resource = offsetResource_[offsetDataCounter_];
				auto& inUse = instanceOffsetData_[offsetDataCounter_];
				inUse->Offset_WVP = static_cast<UINT>(instancePCCounter_);
				inUse->Offset_MaterialIndexList = static_cast<UINT>(instanceMaterialIndexCounter_);
				commandList_->SetGraphicsRootConstantBufferView(static_cast<UINT>(RootSlot::InstanceOffset_CB), resource->GetResource()->GetGPUVirtualAddress());

				for (auto& [materialID, TransformMatirx] : RenderDataGroup) {

					/// MaterialIndexList 設定
					if (instanceMaterialIndexCounter_ >= config::GetMaxMaterialNum())assert(false);
					materialIndexList_[instanceMaterialIndexCounter_] = materialID;
					++instanceMaterialIndexCounter_;

					/// Counter計算
					instancesCounter++;
					instancePCCounter_++;
				}
				offsetDataCounter_++;


				if (meshIndexCount != 0) {
					commandList_->DrawIndexedInstanced(meshIndexCount, instancesCounter, 0, 0, 0);
				} else {
					int meshVertexCount_ = mesh->GetVertexNum();
					commandList_->DrawInstanced(meshVertexCount_, instancesCounter, 0, 0);
				}
			}
		}
	}
}

/// ------------------------------------- まとめのDrawCell -------------------------------------- ///

void DrawEngine::DrawCall() {

	/// 非透明、透明順で
	/// 3D、Sprite、Particle、DebugLineの順
	/// で描画する
	Draw3D();
	Draw2D();
	DrawParticle();
	DrawDebugLine();
}

/// ------------------------------------- PostProcess関連 -----------------------------===------ ///

void DrawEngine::TransitionRenderTarget(
	RenderTexture& renderTexture,
	D3D12_RESOURCE_STATES toState
) {
	if (renderTexture.currentState == toState)
		return;

	CD3DX12_RESOURCE_BARRIER offscreenToRT =
		CD3DX12_RESOURCE_BARRIER::Transition(
			renderTexture.resource.Get(),
			renderTexture.currentState,
			toState
		);
	commandList_->ResourceBarrier(1, &offscreenToRT);
	renderTexture.currentState = toState;
}

void DrawEngine::TransitionDepthStencil(
	RenderTexture& renderTexture,
	D3D12_RESOURCE_STATES toState
) {

	if (renderTexture.depthState == toState)
		return;

	CD3DX12_RESOURCE_BARRIER depthToSRV =
		CD3DX12_RESOURCE_BARRIER::Transition(
			renderTexture.depthResource.Get(),
			renderTexture.depthState,
			toState,
			0
		);
	commandList_->ResourceBarrier(1, &depthToSRV);
	renderTexture.depthState = toState;
}

void DrawEngine::SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE renderTarget) {
	auto dsv = directXDriver_->GetDsvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart();
	commandList_->OMSetRenderTargets(1, &renderTarget, FALSE, &dsv);
}

void DrawEngine::SetSRVHeap() {
	ID3D12DescriptorHeap* srvHeaps[] = { SrvManager::GetInstance()->GetDescriptorHeap() };
	commandList_->SetDescriptorHeaps(1, srvHeaps);
}

void DrawEngine::SetRootDescriptorTable(UINT rootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE descriptorHandle) {
	commandList_->SetGraphicsRootDescriptorTable(rootParameterIndex, descriptorHandle);
}

void DrawEngine::DrawColorGrading(RenderCommandGPU& renderCommandGPU) {

	/// RenderTargetを切り替える
	postProcessRunner_->SetRenderTargetsForDraw(this);

	SetRenderTarget(postProcessRunner_->GetRenderTarget().outputRT.rtvHandleCPU);

	/// PSOを設定
	PSOKey psoKey = CreateColorGradingPSOKey();
	psoManager_->SetPSO(psoKey);

	/// RenderCommandをPostProcessRunnerにセット
	postProcessRunner_->SetRenderCommand(this, renderCommandGPU);

	/// SRV Heapを設定
	SetSRVHeap();

	/// RenderTargetを設定
	SetRootDescriptorTable(static_cast<UINT>(RootSlotPostProcess::SourceTexture_SB), postProcessRunner_->GetRenderTarget().inputRT.srvHandleGPU);

	/// 最終のドロー
	DrawFullscreenQuad();

	/// OffscreenRTを入れ替える
	std::swap(postProcessRunner_->GetRenderTarget().inputRT, postProcessRunner_->GetRenderTarget().outputRT);

}

void DrawEngine::DrawVignette(RenderCommandGPU& renderCommandGPU) {

	/// RenderTargetを切り替える
	postProcessRunner_->SetRenderTargetsForDraw(this);

	SetRenderTarget(postProcessRunner_->GetRenderTarget().outputRT.rtvHandleCPU);

	/// PSOを設定
	PSOKey psoKey = CreateVignettePSOKey();
	psoManager_->SetPSO(psoKey);

	/// RenderCommandをPostProcessRunnerにセット
	postProcessRunner_->SetRenderCommand(this, renderCommandGPU);

	/// SRV Heapを設定
	SetSRVHeap();

	/// DescriptorTableを設定
	SetRootDescriptorTable(static_cast<UINT>(RootSlotPostProcess::SourceTexture_SB), postProcessRunner_->GetRenderTarget().inputRT.srvHandleGPU);

	/// 最終のドロー
	DrawFullscreenQuad();

	/// OffscreenRTを入れ替える
	std::swap(postProcessRunner_->GetRenderTarget().inputRT, postProcessRunner_->GetRenderTarget().outputRT);

}

void DrawEngine::DrawBlur(RenderCommandGPU& renderCommandGPU, KernelDataGPU& kernelData) {

	/// RenderTargetを切り替える
	postProcessRunner_->SetRenderTargetsForDraw(this);

	SetRenderTarget(postProcessRunner_->GetRenderTarget().outputRT.rtvHandleCPU);

	/// PSOを設定
	PSOKey psoKey = CreateBlurPSOKey();
	psoManager_->SetPSO(psoKey);

	/// RenderCommandをPostProcessRunnerにセット
	postProcessRunner_->SetRenderCommand(this, renderCommandGPU);

	/// BlurDataを設定(TODO:変える予定)
	postProcessRunner_->SetKernelData(kernelData);
	SetRootDescriptorTable(static_cast<UINT>(RootSlotPostProcess::KernelData_SB), kernelDataResource_->GetGPUDescriptorHandle());

	/// SRV Heapを設定
	SetSRVHeap();

	/// DescriptorTableを設定
	SetRootDescriptorTable(static_cast<UINT>(RootSlotPostProcess::SourceTexture_SB), postProcessRunner_->GetRenderTarget().inputRT.srvHandleGPU);

	/// 最終のドロー
	DrawFullscreenQuad();

	/// OffscreenRTを入れ替える
	std::swap(postProcessRunner_->GetRenderTarget().inputRT, postProcessRunner_->GetRenderTarget().outputRT);

}

void DrawEngine::DrawOutline(RenderCommandGPU& renderCommandGPU) {

	/// RenderTargetを切り替える
	postProcessRunner_->SetRenderTargetsForDraw(this);

	SetRenderTarget(postProcessRunner_->GetRenderTarget().outputRT.rtvHandleCPU);

	/// PSOを設定
	PSOKey psoKey = CreateOutlinePSOKey();
	psoManager_->SetPSO(psoKey);

	/// RenderCommandをPostProcessRunnerにセット
	postProcessRunner_->SetRenderCommand(this, renderCommandGPU);

	/// OutlineDataを設定
	SetRootDescriptorTable(static_cast<UINT>(RootSlotPostProcess::KernelData_SB), kernelDataResource_->GetGPUDescriptorHandle());

	/// SRV Heapを設定
	SetSRVHeap();

	/// DescriptorTableを設定
	SetRootDescriptorTable(static_cast<UINT>(RootSlotPostProcess::SourceTexture_SB), postProcessRunner_->GetRenderTarget().inputRT.srvHandleGPU);

	/// 最終のドロー
	DrawFullscreenQuad();

	/// OffscreenRTを入れ替える
	std::swap(postProcessRunner_->GetRenderTarget().inputRT, postProcessRunner_->GetRenderTarget().outputRT);

}

void DrawEngine::DrawOutlinePrewittDepth(RenderCommandGPU& renderCommandGPU) {

	/// RenderTargetを切り替える
	postProcessRunner_->SetRenderTargetsForDraw(this);

	/// DepthStencilを切り替える
	TransitionDepthStencil(
		postProcessRunner_->GetRenderTarget().inputRT,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);

	SetRenderTarget(postProcessRunner_->GetRenderTarget().outputRT.rtvHandleCPU);

	/// PSOを設定
	PSOKey psoKey = CreateOutlinePrewittDepthPSOKey();
	psoManager_->SetPSO(psoKey);

	/// RenderCommandをPostProcessRunnerにセット
	postProcessRunner_->SetRenderCommand(this, renderCommandGPU);

	/// OutlineDataを設定
	SetRootDescriptorTable(static_cast<UINT>(RootSlotPostProcess::KernelData_SB), kernelDataResource_->GetGPUDescriptorHandle());

	/// SRV Heapを設定
	SetSRVHeap();

	/// DescriptorTableを設定
	SetRootDescriptorTable(static_cast<UINT>(RootSlotPostProcess::SourceTexture_SB), postProcessRunner_->GetRenderTarget().inputRT.srvHandleGPU);

	/// DepthSRVを設定
	SetRootDescriptorTable(static_cast<UINT>(RootSlotPostProcess::DepthTexture_SRV), postProcessRunner_->GetRenderTarget().inputRT.depthSrvHandleGPU);

	/// 最終のドロー
	DrawFullscreenQuad();

	/// OffscreenRTを入れ替える
	std::swap(postProcessRunner_->GetRenderTarget().inputRT, postProcessRunner_->GetRenderTarget().outputRT);
}

void DrawEngine::DrawDissolve(RenderCommandGPU& renderCommandGPU, int dissolveTextureIndex) {

	/// RenderTargetを切り替える
	postProcessRunner_->SetRenderTargetsForDraw(this);

	SetRenderTarget(postProcessRunner_->GetRenderTarget().outputRT.rtvHandleCPU);

	// ⭐ 加這一行：清除 OutputRT
	float clearColor[4] = { 0.1f, 0.25f, 0.5f, 1.0f };
	commandList_->ClearRenderTargetView(postProcessRunner_->GetRenderTarget().outputRT.rtvHandleCPU, clearColor, 0, nullptr);


	/// PSOを設定
	PSOKey psoKey = CreateDissolvePSOKey();
	psoManager_->SetPSO(psoKey);

	/// RenderCommandをPostProcessRunnerにセット
	postProcessRunner_->SetRenderCommand(this, renderCommandGPU);

	/// SRV Heapを設定
	SetSRVHeap();

	/// MaskTextureを設定
	int maskTextureHandleFormRenderCommand = dissolveTextureIndex;
	int dissolveTextureHandle = (maskTextureHandleFormRenderCommand == -1) ?
		defaultTextureHandle_ :
		maskTextureHandleFormRenderCommand;
	SetRootDescriptorTable(static_cast<UINT>(RootSlotPostProcess::Texture2_SRV), resourceManager_->GetTextureGPUDescriptorHandle(dissolveTextureHandle));

	/// DescriptorTableを設定
	SetRootDescriptorTable(static_cast<UINT>(RootSlotPostProcess::SourceTexture_SB), postProcessRunner_->GetRenderTarget().inputRT.srvHandleGPU);

	/// 最終のドロー
	DrawFullscreenQuad();

	/// OffscreenRTを入れ替える
	std::swap(postProcessRunner_->GetRenderTarget().inputRT, postProcessRunner_->GetRenderTarget().outputRT);
}

void DrawEngine::DrawNoise(RenderCommandGPU& renderCommandGPU) {

	/// RenderTargetを切り替える
	postProcessRunner_->SetRenderTargetsForDraw(this);

	SetRenderTarget(postProcessRunner_->GetRenderTarget().outputRT.rtvHandleCPU);

	/// PSOを設定
	PSOKey psoKey = CreateNoisePSOKey();
	psoManager_->SetPSO(psoKey);

	/// RenderCommandをPostProcessRunnerにセット
	postProcessRunner_->SetRenderCommand(this, renderCommandGPU);

	/// SRV Heapを設定
	SetSRVHeap();

	/// DescriptorTableを設定
	SetRootDescriptorTable(static_cast<UINT>(RootSlotPostProcess::SourceTexture_SB), postProcessRunner_->GetRenderTarget().inputRT.srvHandleGPU);

	/// 最終のドロー
	DrawFullscreenQuad();

	/// OffscreenRTを入れ替える
	std::swap(postProcessRunner_->GetRenderTarget().inputRT, postProcessRunner_->GetRenderTarget().outputRT);

}

void DrawEngine::DrawRenderCopy() {

	/// RenderTargetを切り替える
	TransitionRenderTarget(
		postProcessRunner_->GetRenderTarget().inputRT,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE
	);

	/// RenderTargetを設定
	auto backRTV = directXDriver_->GetCurrentBackBufferRTV();
	SetRenderTarget(backRTV);

	/// PSOを設定
	PSOKey psoKey = CreateFullscreenPSOKey();
	psoManager_->SetPSO(psoKey);

	/// SRV Heapを設定
	SetSRVHeap();

	/// DescriptorTableを設定
	SetRootDescriptorTable(static_cast<UINT>(RootSlotPostProcess::SourceTexture_SB), postProcessRunner_->GetRenderTarget().inputRT.srvHandleGPU);

	/// 最終のドロー
	DrawFullscreenQuad();

}

void DrawEngine::DrawFullscreenQuad() {

	/// VB/IBがいらないのでnullptrをセット
	commandList_->IASetVertexBuffers(0, 0, nullptr);
	commandList_->IASetIndexBuffer(nullptr);

	/// primitive typeの設定 
	commandList_->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	/// Draw
	commandList_->DrawInstanced(3, 1, 0, 0);
}

void DrawEngine::SetEnviromentReflectionTexture(int textureHandle) {
	enviromentReflectionTextureHandle_ = textureHandle;
}

/// ------------------------------------- Skinning関連 --------------------------------------- ///

void DrawEngine::CreateSkinningBuffer(ObjectData* objectData) {

	ModelGroup* modelGroup = ResourceManager::GetInstance()->modelGroupList_[objectData->modelHandle_].get();

	for (int i = 0; i < modelGroup->GetModelNum(); ++i) {
		/// 必要な資料をとる
		Model* model = modelGroup->GetModel(i);
		std::weak_ptr<ModelData> modelData = model->GetModelData();
		int skeletonJointNum = (int)modelData.lock()->skeleton.jointList.size();
		int vertexInfluenceNum = (int)modelData.lock()->meshDataList[i].vertices.size();

		/// WellForGPUのバッファを作成する
		std::unique_ptr<InstanceBuffer<WellForGPU>> skinningWFGResource_ = std::make_unique<InstanceBuffer<WellForGPU>>(directXDriver_);
		WellForGPU* instanceListPtrDL = skinningWFGResource_->CreateInstanceBuffer(skeletonJointNum);

		/// VertexInfluenceのバッファを集まる
		auto span = model->GetSkinClusterData().GetVertexInfluences();
		VertexInfluence* instanceListPtrVI = span.data();

		/// ListにInstanceBuffer of WellForGPUを追加
		int bufferIndex = -1;

		if (!skinningBufferFreeList_.empty()) {
			bufferIndex = skinningBufferFreeList_.back();
			skinningBufferFreeList_.pop_back();
			skinningWFGResourceList_[bufferIndex] = std::move(skinningWFGResource_);
		} else {
			bufferIndex = (int)skinningWFGResourceList_.size();
			skinningWFGResourceList_.push_back(std::move(skinningWFGResource_));
		}

		/// DrawDataCollectorに渡す
		int handle = drawDataCollector_->SetSkinningData(instanceListPtrDL, skeletonJointNum, instanceListPtrVI, vertexInfluenceNum);

		/// HandleをModelに保存する
		skinningDatDDC2DEaMap_[handle] = bufferIndex;
		objectData->objectParts_[i].wellHandle = handle;
	}
}

void DrawEngine::ClearSkinningBuffer(ObjectData* objectData) {

	for (auto& part : objectData->objectParts_) {

		int ddcHandle = part.wellHandle;
		if (ddcHandle < 0) continue;

		// 1. 找到 DrawEngine buffer index
		auto it = skinningDatDDC2DEaMap_.find(ddcHandle);
		if (it != skinningDatDDC2DEaMap_.end()) {

			int bufferIndex = it->second;

			// 2. 清除 GPU buffer
			skinningWFGResourceList_[bufferIndex].reset();

			// 3. 放回 free-list
			skinningBufferFreeList_.push_back(bufferIndex);

			// 4. 移除 mapping
			skinningDatDDC2DEaMap_.erase(it);
		} else {
			continue;
		}

		// 5. 清除 DDC 的資料
		drawDataCollector_->ClearSkinningData(ddcHandle);

		// 6. 清除 ObjectPart 的 handle
		part.wellHandle = -1;
	}
}

int DrawEngine::readCommonTextureHandle(int handle) {
	return resourceManager_->GetTextureHandleFromCommonList(handle);
}

int DrawEngine::GetModelTextureHandle(int modelHandle, int part) {
	return resourceManager_->GetTextureHandleFromModelGroup(modelHandle, part);
}

int DrawEngine::readModelTextureHandle(int handle) {
	return resourceManager_->ReadModelTextureHandle(handle);
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

void DrawEngine::IntializeInstanceTMBuffer(TransformationMatrix* bufferPointer, size_t count) {
	for (size_t index = 0; index < count; ++index) {
		bufferPointer[index].WVP = Identity();
		bufferPointer[index].world = Identity();
		bufferPointer[index].WorldInverseTranspose = Identity();
	}
}

//void DrawEngine::SetMaterial(int materialID) {
//
//	auto checker = resourceManager_->idToIndex_.find(materialID);
//
//	if (checker == resourceManager_->idToIndex_.end()) {
//		Logger::Log("[kError]DE:MaterialID not found in ResourceManager!");
//		return;
//	}
//
//	//commandList_->SetGraphicsRootConstantBufferView(static_cast<UINT>(RootSlot::MaterialList_SB), resourceManager_->materialList_[checker->second].gpuMaterial->GetResource()->GetGPUVirtualAddress());
//}

void DrawEngine::SetTexture(int textureHandle) {

	//auto checker = resourceManager_->idToIndex_.find(materialID);
	//
	//if (checker == resourceManager_->idToIndex_.end()) {
	//	Logger::Log("[kError]DE:SetTexture not found in ResourceManager!");
	//	return;
	//}
	//
	//int textureHandle = resourceManager_->materialList_[checker->second].textureHandle;
	textureSrvHandleGPU_ = TextureManager::GetInstance()->GetTextureGPUDescriptorHandle(textureHandle);
	commandList_->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::Texture_SRV), textureSrvHandleGPU_);
}

void DrawEngine::SetCameraForGPU() {

	cameraPtr_->worldPosition = drawDataCollector_->GetCameraPosition();

	commandList_->SetGraphicsRootConstantBufferView(static_cast<UINT>(RootSlot::Camera_CB), cameraBuffer_->GetResource()->GetGPUVirtualAddress());
}

void DrawEngine::UpdateLighting() {

	// Lightingにデータを書き込む
	lightCount_ = drawDataCollector_->GetLightCount();

	for (int i = 0; i < (int)lightCount_; ++i) {
		lightListData_[i] = drawDataCollector_->GetLightGPUBuffer()[i];
	}

}

void DrawEngine::SetLightingGPU() {

	// LightListGPU Set
	commandList_->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::LightList_SB), lightBuffer_->GetGPUDescriptorHandle());
	// LightingCount Set
	commandList_->SetGraphicsRoot32BitConstants(static_cast<UINT>(RootSlot::LightingCount_CB), 1, &lightCount_, 0);
}

void DrawEngine::SetEnviromentReflectionGPU() {

	auto it = resourceManager_->GetTextureGPUDescriptorHandle(enviromentReflectionTextureHandle_);

	// 綁定到 RootSignature 的 slot 7 (t2)
	commandList_->SetGraphicsRootDescriptorTable(static_cast<UINT>(RootSlot::EnvironmentReflection_SRV), it);

}

void DrawEngine::MakeDepthStencilView() {
	///DSV構築
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	// DSVHeap先頭にDSVをつくる
	directXDriver_->GetDevice()->CreateDepthStencilView(
		depthStencilResource.Get(), &dsvDesc, directXDriver_->GetDsvDescriptorHeap()->GetCPUDescriptorHandleForHeapStart()
	);
}

void DrawEngine::UpdateDebugLineVertexBuffer(const std::vector<DebugLineVertexGPU>& vertices) {

	if (vertices.empty()) return;

	size_t bufferSize = sizeof(DebugLineVertexGPU) * vertices.size();

	// 如果 buffer 不夠大 → 重建
	if (!debugLineVB_ || debugLineVertexBufferSize_ < bufferSize) {
		debugLineVertexBufferSize_ = bufferSize;

		// 用你現有的 BasicResource 工具建立 Upload Buffer

		TransformationMatrix* instanceListPtrDL = debugLineResource_->CreateInstanceBuffer((int)bufferSize);
		drawDataCollector_->SetInstanceListDL(instanceListPtrDL);

		IntializeInstanceTMBuffer(instanceListPtrDL, (size_t)bufferSize);

		debugLineVB_ = CreateResource(directXDriver_->GetDevice(), (size_t)bufferSize);
	}

	// Map + Copy
	DebugLineVertexGPU* mapped = nullptr;
	debugLineVB_->Map(0, nullptr, (void**)&mapped);
	memcpy(mapped, vertices.data(), bufferSize);
	debugLineVB_->Unmap(0, nullptr);

	// VBView
	debugLineVBView_.BufferLocation = debugLineVB_->GetGPUVirtualAddress();
	debugLineVBView_.SizeInBytes = (UINT)bufferSize;
	debugLineVBView_.StrideInBytes = sizeof(DebugLineVertexGPU);
}
