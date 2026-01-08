#pragma once
#include "Object/Object.h"


class Bullet :public Object
{
public:
	Bullet(kEngine* system, const Vector3& position = Vector3{ 0,0,0 }, const Vector3& direction = Vector3{ 0,0,1 });
	~Bullet();
	void Update(Camera* camera) override;
	Vector3 GetPosition() { return mainPosition.transform.translate; }
	void SetPosition(const Vector3& position) { mainPosition.transform.translate = position; }

	bool IsAlive() const { return isAlive_; }
	void SetAlive(bool isAlive) { isAlive_ = isAlive; }

private:
	Vector3 direction_{};
	float speed_ = 20.0f;
	float lifeTime_ = 2.0f;
	bool isAlive_ = true;

private:

	int MH_bullet_{};

};

