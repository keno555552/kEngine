#pragma once
#include "TileMap.h"
#include "PerlinNoise/PerlinNoise.h"

class TileRunnerP
{
public:

	void SetTileRunner(int maxRow, int maxCol,
		float attenuation, int step,
		int threshold, int seed);

	void SetTileRunner(int maxRow, int maxCol);

	void SetStartPoint(int startPoint) { startPoint_ = startPoint; }
	void SetSplitPoint(int splitPoint) { splitPoint_ = splitPoint; }

	void MakeFormMap(TileMapData* targetMap, TileMapData* refenceMap, int type, int startRadius, int stopper = -500);
	void MakeOverWriteMap(TileMapData* targetMap, int type, int startRadius,int stopper = -500);



private:
	int startPoint_ = 138;
	int splitPoint_ = 173;
private:
	PerlinNoise* perlinNoise = new PerlinNoise;
	TileMapData* refenceMap_ = nullptr;
	int maxRow_ = 0;
	int maxCol_ = 0;
	float attenuation_ = 0.95f;
	int step_ = 300;
	int threshold_ = 90;
	int seed_ = 0;

private:
	void digCircle(TileMapData* tileMap, int cx, int cy, float radius, int type, int stopper = -500);
	float runTileRunner(float radius, TileMapData* tileMap, int& x, int& y, TileMapData& mapData, int splitTime, int type, int stopper = -500);
};

