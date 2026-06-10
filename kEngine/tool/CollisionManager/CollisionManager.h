#pragma once
#include "MathsIncluder.h"
#include "Data\Game\MapChip\MapChipField.h"
#include <vector>

class Enemy;
class Bullet;
class Player;
class BackPoint;
class CollisionManager
{
public:
	
	void Update(float deltaTime);

	void InputPlayer(Player* player);
	void InputMapChip(MapChipField* mapChipField);
	void InputBackPoint(BackPoint* backPoint);
	void InputBulletList(std::vector<Bullet*>* bulletList);
	void InputEnemyList(std::vector<Enemy*>* enemyList);


private:
	void MapCollisionCircle();
	bool PlayerMapCollision();
	void PlayerEnemyCollision();
	void PlayerBackPointCollision();
	void BulletMapCollision();
	void BulletPlayerCollision();
	void BulletEnemyCollision();
	void EnemyMapCollision();

private:
	Player* player_ = nullptr;
	MapChipField* mapChipField_ = nullptr;
	BackPoint* backPoint_ = nullptr;
	std::vector<Bullet*>* bulletList_ = nullptr;
	std::vector<Enemy*>* enemyList_ = nullptr;

	float deltaTime_ = 0.0f;

private:
	bool CheckIsWall(MapChipType type);
	bool CheckIsNotWall(MapChipType type);
};

