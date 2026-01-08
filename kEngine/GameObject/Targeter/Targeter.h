#pragma once
#include "Object\Object.h"

class Player;
class Targeter :public Object
{
public:
	Targeter(kEngine* system, Player* player);
	~Targeter();
	void Update(Camera* camera) override;

	Vector3 GetMouseOnPlane() const { return mouseOnPlane_; }

private:
	Player* player_ = nullptr;  // 借り
	// mouse projected onto player's plane (z same as player)
	Vector3 mouseOnPlane_{};
};
