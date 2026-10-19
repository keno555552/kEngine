#pragma once
#include "DungeonCell.h"

struct DungeonData {
	/// ダンジョンデータ
	int dungeonID{};
	int startRoomID{};
	std::vector<int> bossRoomsID;
	std::vector<int> treasureRoomsID;
	std::vector<int> normalRoomsID;
	float RoomRate[ROOMTYPE_NUMOF][ROOMID_NUMOF]{};
	int MaxWidth{};
	int MaxHeight{};
	int MinWidth{};
	int MinHeight{};
	int EachRoomWidth{};
	int EachRoomHeight{};

	/// 生成関連
	int StartPoint{};
	bool isNecess{};
	bool isOnly{};
	int dungeonSpawnDistance{};

	void NormalizeRates(int RoomType);
	bool SetCheck(int RoomType);
	int RoomSeachByRate(int RoomType, int perlin);
};


struct GBPTestDungeon :DungeonData {
	GBPTestDungeon();
};