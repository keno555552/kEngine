#include "DungeonData.h"
#include "Logger.h"
#include "cassert"

GBPTestDungeon::GBPTestDungeon() {
	startRoomID = ROOMID_TEST;
	bossRoomsID = { ROOMID_BOSS };
	treasureRoomsID = { ROOMID_JEWEL };
	normalRoomsID = { ROOMID_NO,ROOMID_TEST,ROOMID_MONSTER1,ROOMID_MONSTERFORMIHARA };
	RoomRate[ROOMTYPE_BOSS][ROOMID_BOSS] = 1.0f;
	RoomRate[ROOMTYPE_TREASURE][ROOMID_JEWEL] = 1.0f;
	RoomRate[ROOMTYPE_NORMAL][ROOMID_NO] = 0.2f;
	RoomRate[ROOMTYPE_NORMAL][ROOMID_TEST] = 0.2f;
	RoomRate[ROOMTYPE_NORMAL][ROOMID_MONSTER1] = 0.3f;
	RoomRate[ROOMTYPE_NORMAL][ROOMID_MONSTERFORMIHARA] = 0.3f;
	MaxWidth = 6;
	MaxHeight = 4;
	MinWidth = 3;
	MinHeight = 2;
	EachRoomWidth = 20;
	EachRoomHeight = 10;

	StartPoint = 95;
	isNecess = false;
	isOnly = false;
	dungeonSpawnDistance = 100;

	SetCheck(ROOMTYPE_BOSS);
	SetCheck(ROOMTYPE_TREASURE);
	//NormalizeRates(ROOMTYPE_NORMAL);
	SetCheck(ROOMTYPE_NORMAL);
}

void DungeonData::NormalizeRates(int RoomType) {
	float total = 0.0f;
	int emptyCount = 0;

	for (int id = 0; id < ROOMID_NUMOF; ++id) {
		total += RoomRate[RoomType][id];
		if (RoomRate[RoomType][id] == 0.0f)
			++emptyCount;
	}

	if (total < 1.0f && emptyCount > 0) {
		float remaining = 1.0f - total;
		float addition = remaining / emptyCount;

		for (int id = 0; id < ROOMID_NUMOF; ++id) {
			if (RoomRate[RoomType][id] == 0.0f)
				RoomRate[RoomType][id] = addition;
		}

		char buffer[256];
		sprintf_s(buffer,
			"[Normalize] Type %d -> %d entries filled | Remaining = %.4f | Each = %.4f | Original sum = %.4f\n",
			RoomType, emptyCount, remaining, addition, total);
		Logger::Log(buffer);
	}
}

bool DungeonData::SetCheck(int RoomType) {
	float handle = 0;
	for (int ID = 0; ID < ROOMID_NUMOF; ID++) {
		handle += RoomRate[RoomType][ID];
	}
	if (handle < 0.99f) {
		char buffer[256];
		sprintf_s(buffer, "DungeonData::SetCheck() : Rate total is not enough_%d\n", RoomType);
		Logger::Log(buffer);
		assert(false);
	}
	return false;
}

int DungeonData::RoomSeachByRate(int RoomType, int perlin) {
	float rate = float(perlin % 191) / 191.0f; //perlin
	float increaseRate = rate;
	std::vector<int> RoomIDList;
	if (RoomType == ROOMTYPE_BOSS) { RoomIDList = bossRoomsID; }
	if (RoomType == ROOMTYPE_TREASURE) { RoomIDList = treasureRoomsID; }
	if (RoomType == ROOMTYPE_NORMAL) { RoomIDList = normalRoomsID; }
	for (int i = 0; i < RoomIDList.size(); i++) {
		if (RoomRate[RoomType][RoomIDList[i]] >= increaseRate) {
			return RoomIDList[i];
		} else {
			increaseRate -= RoomRate[RoomType][RoomIDList[i]];
		}
	}
	return 0;
}
