#pragma once
#include "Object/Object.h"
#include "Object/Sprite.h"

class DebugObject :public Object
{
public:
	DebugObject(kEngine* kEngine);
	~DebugObject();

	void Update(Camera* camera) override;
	void Draw() override;

	void SetFollowObject(ObjectPart* followObject);
	void ClearFollowObject();

	void SetShowCenterPoint(bool isShow);

private:
	/// ============= デバッグ用更新関数 ============= ///
	/// ///////全体更新///////// ///
	void updateDebugParts();
	/// 中心点更新 ///
	void updateCenterPoint();

private:
	
	/// ============= カメラ ============= ///
	Camera* camera_ = nullptr;	/// 借り

	/// ============= ハンドル ============= ///
	/// モデル ///

	/// テキスチャ ///
	int TH_centerPoint{};


	/// ============= デバッグ用パーツ ============= ///
	SimpleSprite* centerPoint_ = nullptr;


	/// ============= フラグ ============= ///
	bool isShowCenterPoint_ = false;


};

