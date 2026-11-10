#pragma once
#include "tool/TimeManager/TimeManager.h"
#include "Object/Object.h"

static inline const float kSignOPEDTime = 0.5f;
static inline const float kSignInteractRange = 1.0f;
static inline const Vector3 kSignInteractHalfSize = { 1.0f, 1.0f, 1.0f };
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
	void SetIsPlayerNear(bool isNear) { isPlayerNear_ = isNear; }
	bool GetIsPlayerNear() const { return isPlayerNear_; }

	/// ================= 当たり判定関連 ================= ///
	AABB GetAABB();
	/// プレイヤーが近いか（判定はマスの中心で）
	bool IsPlayerNear(const Vector3& playerPos) const;

private:
	/// ======== モデルハンドル ======== ///
	int MH_Model_{};

	/// ===== アニメーション関連 ===== ///
	Vector3 ChangeSmallScale_ = { 0.6f, 0.3f, 0.5f };
	Timer animationTimer_;
	bool isTimeUp_ = true;

	/// ===== ヒットボックス ===== ///
	AABB hitBox_{};
	Vector3 cellPosition_{};
	bool isPlayerNear_ = false;
 
	/// ======== フラグ ======== ///
	bool isInteract_ = false;

};