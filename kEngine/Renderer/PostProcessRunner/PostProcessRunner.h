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
	int SetPassCopy					(int layer, RenderCommand command = {});
	int SetPassColorGrading			(int layer, RenderCommand command);
	int SetPassVignette				(int layer, RenderCommand command);
	int SetPassBlur					(int layer, RenderCommand command);///未完
	int SetPassOutline				(int layer, RenderCommand command);///未完
	int SetPassOutlinePrewittDepth	(int layer, RenderCommand command);
	int SetPassDissolve				(int layer, RenderCommand command);
	int SetPassNoise				(int layer, RenderCommand command);

	void SetPassCommand(int layer, int passIndex, RenderCommand command);
	/// リスト内の指定したパスを消去する、layerがdefault(-1)の場合、全layer消す。
	void ClearLayer(int layer = -1);
	/// リスト内の指定したパスを消去する、passIndexの場合、該当layerの全パスを消す。
	void ClearPass(int layer = -1, int passIndex = -1);
	/// リスト内の指定した層のパスの名前を取得する。
	std::string GetPassName(int layer, int passIndex);

	/// ==================== エンジン側関連 ==================== ///
	/// 描画チェーンを実行する関数
	void Execute(DrawEngine* drawEngine);
	/// 描画チェーンを実行する前に実行する関数、PPRenderTextureをReset、Clear、設定する用
	void SetPreDraw(DrawEngine* drawEngine);

	/// drawEngine用、RenderCommandGPUを設定する関数
	void SetRenderCommand(DrawEngine* drawEngine, const RenderCommandGPU& command);
	/// drawEngine用、PPRenderCommandを描画用に初期化するための関数
	void SetRenderTargetsForDraw(DrawEngine* drawEngine);
	/// drawEngine用、PPRenderCommandを獲得するための関数
	PPRenderTexture& GetRenderTarget();

	/// KernelDataGPUを設定するための関数、使えなくなる予定
	void SetKernelData(const KernelDataGPU& kernelData);
	void SetInstanceListBlurData(KernelDataGPU* instancingListBlurData) { instancingListBlurData_ = instancingListBlurData; }
	KernelDataGPU* GetInstanceListBlurData() { return instancingListBlurData_; }

private:

	/// Main,パースのリスト。中はLayerとして分けらでいる
	std::vector<std::unique_ptr<PostProcessLayer>> layerList_;

	//TODO:shader側のここのデータはまたStructuredBufferとして取られてる、普通のconstantBufferにするべき
	KernelDataGPU* instancingListBlurData_ = nullptr;

	/// RenderTargetの入れ物、PingPong形式で二つあって、処理用な他のもの入っている
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

