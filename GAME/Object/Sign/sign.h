#pragma once
#include "tool/TimeManager/TimeManager.h"
#include "Object/Object.h"

static inline const float kSignOPEDTime = 0.5f;
static inline const float kSignOPEDRate = 0.5f;

class Sign : public Object
{
public:
	Sign(kEngine* system, const Vector3& position = Vector3{ 0,0,0 });
	~Sign();

	void Update(Camera* camera) override;
	Vector3 GetPosition() { return mainPosition.transform.translate; }
	void SetPosition(const Vector3& position) { mainPosition.transform.translate = position; }
	void SetIsInteract(bool isInteract) { isInteract_ = isInteract;}
	bool GetIsInteract() const { return isInteract_; }


	AABB GetAABB();

private:
	/// ======== モデルハンドル ======== ///
	int MH_Model_{};

	/// ===== アニメーション関連 ===== ///
	Vector3 ChangeSmallScale_ = { 0.6f, 0.3f, 0.5f };
	Timer animationTimer_;
	bool isTimeUp_ = true;

	/// ===== ヒットボックス ===== ///
	AABB hitBox_{};
 
	/// ======== フラグ ======== ///
	bool isInteract_ = false;

};