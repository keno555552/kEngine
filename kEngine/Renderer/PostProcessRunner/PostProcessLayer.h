#pragma once
#include <vector>
#include <memory>
#include <map>
#include <string>
#include "Data/Render/Types/PostProcessType.h"
#include "Data/Render/CPUData/RenderCommand.h"
#include "Data/Render/GPUData/RenderCommandGPU.h"
#include "Data/Render/CPUData/RenderTexture.h"
#include "Data/Render/GPUData/BlurDataGPU.h"
#include "PostProcessPass.h"

class DrawEngine;
class PostProcessLayer
{
public:
	/// ==================== PassList関連 ==================== ///
	/// 描画チェーンを足す関数
	void AddPass(std::unique_ptr<PostProcessPass> pass);
	/// リスト内の指定したパスを消去する、passIndexがdefault(-1)の場合、該当layerの全パスを消す。
	void ClearPass( int passIndex = -1);
	/// リスト内の指定した層のパスの名前を取得する。
	std::string GetPassName(int passIndex);
	int GetPassCount() { return static_cast<int>(passList_.size()); }

	void Execute(DrawEngine* engine);



public:
	std::vector<std::unique_ptr<PostProcessPass>> passList_;
};

