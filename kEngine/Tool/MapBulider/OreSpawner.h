#pragma once
#include "TileMap.h"
#include "PerlinNoise/PerlinNoise.h"

class OreSpawner
{
public:

	void SetOreSetting();

	void SetOreSpawner(int maxRow, int maxCol,
		float attenuation,int threshold, int seed);

	void SetOreSpawner(int maxRow, int maxCol);

	void MakeFormMap(TileMapData* targetMap, TileMapData* refenceMap, int stopper = -500);
	void MakeOverWriteMap(TileMapData* targetMap, int stopper = -500);



private:
	/// OreSetting
	struct OreSetting {
		float radius;
		int seedPlus;
		int startLevel;
		int endLevel;
		int startPoint;
		int minVolume;
		int maxVolume;
		int minGap;
		int type;
	};

	enum OreType {
		iron,
		gold,
		diamond,
		maxType
	};

	OreSetting oreSetting[maxType] = { { 0.5f,0,20, 70,140,8,20, 15,IRON},
									   { 0.5f,1,60,100,200,3, 5, 5,GOLD },
									   { 0.5f,5,80,100,220,3, 7, 5,DIAMOND} };

private:
	PerlinNoise* perlinNoise = new PerlinNoise;
	TileMapData* refenceMap_ = nullptr;
	int maxRow_ = 0;
	int maxCol_ = 0;
	float attenuation_ = 0.9f;
	int threshold_ = 90;
	int seed_ = 0;

private:
	void digCircle(TileMapData* tileMap, int cx, int cy, float radius, int type, int stopper = -500);
	void runOreSpawner(float radius, TileMapData* tileMap, int& x, int& y, TileMapData& mapData, int minVolume,int maxVolume, int type, int stopper = -500);
};

