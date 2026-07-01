#pragma once
#include <vector>
#include <map>
#include <string>
#include "Data/Render/Types/PostProcessType.h"
#include "Renderer/PostProcessRunner/PostProcessLayer.h"


class DrawEngine;
class PostProcessRunner {
public:
	PostProcessRunner();

	/// ==================== PassList関連 ==================== ///
	/// 描画チェーンを一つずつ設定する関数
	void SetPassCopy					(int layer, RenderCommand command = {});
	void SetPassColorGrading			(int layer, RenderCommand command);
	//void SetPassVignette				(int layer, RenderCommand command);
	//void SetPassBlur					(int layer, RenderCommand command);
	//void SetPassOutline				(int layer, RenderCommand command);
	//void SetPassOutlinePrewittDepth	(int layer, RenderCommand command);
	//void SetPassDissolve				(int layer, RenderCommand command);
	//void SetPassNoise					(int layer, RenderCommand command);
	/// リスト内の指定したパスを消去する、layerがdefault(-1)の場合、全layer消す,passIndexの場合、該当layerの全パスを消す。
	void ClearLayer(int layer = -1);
	/// リスト内の指定したパスを消去する、layerがdefault(-1)の場合、全layer消す,passIndexの場合、該当layerの全パスを消す。
	void ClearPass(int layer = -1, int passIndex = -1);
	/// リスト内の指定した層のパスの名前を取得する。
	std::string GetPassName(int layer, int passIndex);

	void Execute(DrawEngine* drawEngine);
	void SetPreDraw(DrawEngine* drawEngine);

	void SetRenderCommand(DrawEngine* drawEngine, const RenderCommandGPU& command);
	void SetRenderTargetsForDraw(DrawEngine* drawEngine);
	PPRenderTexture& GetRenderTarget();

	/// 描画システムを設定用関数
	void SetBlurlData(const RenderCommand& renderCommand, int instanceIndex);
	void SetOutlinelData(const RenderCommand& renderCommand, int instanceIndex);
	void SetInstanceListBlurData(KernelDataGPU* instancingListBlurData) { instancingListBlurData_ = instancingListBlurData; }
	KernelDataGPU* GetInstanceListBlurData() { return instancingListBlurData_; }

private:
	std::vector<std::unique_ptr<PostProcessLayer>> layerList_;

	RenderCommandGPU renderCommandGPU_{};

	KernelDataGPU* instancingListBlurData_ = nullptr;
	int instanceCounterBlurData_ = 0;

	PPRenderTexture renderTarget_{};


private:
	/// Check,もし問題があればログを出し、trueを返す
	void DrawEngineCheck(DrawEngine* drawEngine);
	bool LayerCheck(int layer = 0);
	bool PassCheck(int layer, int passIndex = 0);

	/// ======================= リセット系 ===================== ///
	void ResetRenderTargets(DrawEngine* drawEngine);
	void ClearRenderTargets(DrawEngine* drawEngine);
};

