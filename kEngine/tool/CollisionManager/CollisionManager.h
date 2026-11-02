#pragma once
#include "MathsIncluder.h"
#include "Data/Game/MapChip/CollisionResult.h"
#include <vector>
#include <memory>

class Enemy;
class Bullet;
class Player;
class Driller;
class Sign;
class BackPoint;
class MapChipManager;
class CollisionManager
{
public:
	
	void Update(float deltaTime);

	void InputMapChip(MapChipManager* mapChipManager);

	void InputPlayer(Player* player);
	void InputBackPoint(BackPoint* backPoint);
	void InputBulletList(std::vector<Bullet*>* bulletList);
	void InputEnemyList(std::vector<Enemy*>* enemyList);
	void InputSignList(std::vector<std::unique_ptr<Sign>>* signList);
	void InputDriller(Driller* driller);

private:
	//void MapCollisionCircle();
	bool PlayerMapCollision();
	void PlayerEnemyCollision();
	void PlayerBackPointCollision();
	void PlayerSignCollision();
	void PlayerDrillerCollision();
	void BulletMapCollision();
	void BulletPlayerCollision();
	void BulletEnemyCollision();
	void EnemyMapCollision();

private:
	Player* player_ = nullptr;
	MapChipManager* mapChipManager_ = nullptr;
	BackPoint* backPoint_ = nullptr;
	Driller* driller_ = nullptr;
	std::vector<Bullet*>* bulletList_ = nullptr;
	std::vector<Enemy*>* enemyList_ = nullptr;
	std::vector<std::unique_ptr<Sign>>* signList_ = nullptr;

	float totelDeltaTime_ = 0.0f;
};

