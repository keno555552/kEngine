#pragma once
#include "tool/TimeManager/TimeManager.h"
#include "Object/Object.h"

/// アニメーション速度
static inline const float kBackPointAnimationTime = 1.0f;

/// 回転速度
static inline const float kBackPointTurnTime = 1.0f;

class BackPoint :public Object
{
public:
	BackPoint(kEngine* system, const Vector3& position = Vector3{ 0,0,0 });
	~BackPoint();
	void Update(Camera* camera) override;
	Vector3 GetPosition() { return mainPosition.transform.translate; }
	void SetPosition(const Vector3& position) { mainPosition.transform.translate = position; basePosition_ = position; }
	bool GetIsBack() const { return isBack_; }
	void SetIsBack(bool isBack) { isBack_ = isBack; }

	AABB GetAABB();


private:

	/// ======== モデルハンドル ======== ///
	int MH_Model_{};


	/// ===== アニメーション関連 ===== ///

	Vector3 basePosition_{};
	Timer animationTimer_;
	bool isTimeUp_ = true;
	
	/// ===== ヒットボックス ===== ///
	AABB hitBox_{};

	/// ======== フラグ ======== ///
	bool isBack_ = false;

};
