#pragma once
#include "mathsIncluder.h"
#include "DungeonData.h"
#include "DungeonCell.h"
#include "TileMap.h"
#include "PerlinNoise/PerlinNoise.h"
#include "Logger.h"

struct DirectionDoorStatus {
	bool isXPlus = false;
	bool isXMinus = false;
	bool isYPlus = false;
	bool isYMinus = false;

};



//Grid - Based Partitioning
class GBPDungeonGenerater
{
public:
	~GBPDungeonGenerater();

	void SetDungeonGenerater(int maxRow, int maxCol, int seed);

	void SpawnTestDungeon(TileMapData* targetMap, int times);

private:
	DungeonData* dungeonData{};
	TileMapData mapData;

private:
	struct RoomData {
		int dataPointX;
		int dataPointY;
		int roomCenterX;
		int roomCenterY;
		int roomType;
		int roomID;
		int perlinValue;
	};
private:
	PerlinNoise* perlinNoise = new PerlinNoise;
	std::vector<Vector2> spawnPoints_{};

	int maxRow_ = 0;
	int maxCol_ = 0;
	int seed_ = 0;

private:
	void DungeonSpawner(TileMapData* targetMap, int cx, int cy, int stopper = -500);
	void bossRoomDirecter(int adjustedcx, int adjustedcy,
						  int dungeonHeightRN, int dungeonWidthRN, 
						  int dungeonWidth,  int dungeonHeight,
						  std::vector<RoomData>& roomData);
	void startRoomDirecter(int adjustedcx, int adjustedcy,
							  int dungeonHeightRN, int dungeonWidthRN,
							  int dungeonWidth, int dungeonHeight,
							  std::vector<GBPDungeonGenerater::RoomData>& roomData);
	void treasureRoomDirecter(int adjustedcx, int adjustedcy,
							  int dungeonHeightRN, int dungeonWidthRN,
							  int dungeonWidth, int dungeonHeight,
							  int treasureRoomNumber,
							  std::vector<GBPDungeonGenerater::RoomData>& roomData);
	void normalRoomDirecter(int adjustedcx, int adjustedcy,
							int dungeonHeightRN, int dungeonWidthRN,
							int dungeonWidth, int dungeonHeight,
							std::vector<GBPDungeonGenerater::RoomData>& roomData);
	void RoomSort( std::vector<GBPDungeonGenerater::RoomData>& roomData);

	void RoomSpawner( TileMapData* targetMap,
					  std::vector<GBPDungeonGenerater::RoomData>& roomData,
					  int stopper = -500);
	void RoadSpawner( TileMapData* targetMap,
					  std::vector<GBPDungeonGenerater::RoomData>& roomData,
					  int stopper = -500);
	void RoadSpawnerHorizon( TileMapData* targetMap,
							  std::vector<GBPDungeonGenerater::RoomData>& roomData,
							  int stopper =-500);
	void RoadSpawnerVertical( TileMapData* targetMap,
							 std::vector<GBPDungeonGenerater::RoomData>& roomData,
							 int stopper =-500);
	Vector2 CenterTunner(int cx, int cy, int dungeonWidth, int dungeonHeight);
};
