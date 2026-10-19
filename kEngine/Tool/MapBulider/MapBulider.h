#pragma once
#include "TileMap.h"
#include "PerlinNoise/PerlinNoise.h"
#include "TileRunner.h"
#include "OreSpawner.h"
#include "GBPDungeonGenerater.h"

class MapBulider
{
public:
	~MapBulider();
	
	void SetSeed(int seed);

	void SetMap(int maxRow, int maxCol,
				int earthLevel,int earthRange,
				int rockLevel,int rockRange);

	void CreateMap(TileMapData* tileMap);


private:
	void CreateEarth(TileMapData* tileMap);
	void CreateRock(TileMapData* tileMap);
	void CreateGrass(TileMapData* tileMap);
	void CreateBurrow(TileMapData* tileMap);
	void CreateOre(TileMapData* tileMap);
	void CreateGBPDungeon(TileMapData* tileMap);

private:
	//enum Type {
	//	AIR,
	//	GRASS,
	//	EARTH,
	//	ROCK,
	//	IRON,
	//	GOLD,
	//	DIAMOND
	//};

private:
	PerlinNoise* perlinNoise = new PerlinNoise;

	int maxRow_ = 0;
	int maxCol_ = 0;
	int earthLevel_ = 0;
	int earthRange_ = 10;
	int rockLevel_ = 6;
	int rockRange_ = 27;

private:
	TileRunnerP* tileRunner = new TileRunnerP;
	int burrowStartPoint_ = 138;
	int burrowSplitPoint_ = 173;

	float attenuation_ = 0.99f;
	int step_ = 100;
	int threshold_ = 90;

public:
	OreSpawner* oreSpawner = new OreSpawner;
	GBPDungeonGenerater* gbpDungeonGenerater = new GBPDungeonGenerater;



	int seed_ = NULL;
};

