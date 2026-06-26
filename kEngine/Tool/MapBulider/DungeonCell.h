#pragma once
#include "TileMap.h"

/// How to add Rooms
/// RoomID++
/// add RoomSturct(don't forget change the RoomID and type)
/// add RoomSeach


enum RoomType {
	ROOMTYPE_START,
	ROOMTYPE_BOSS,
	ROOMTYPE_TREASURE,
	ROOMTYPE_NORMAL,
	ROOMTYPE_NUMOF
};

enum RoomID {
	ROOMID_TEST,
	ROOMID_BOSS,
	ROOMID_JEWEL,
	ROOMID_NO,
	ROOMID_MONSTER1,
	ROOMID_MONSTERFORMIHARA,
	ROOMID_NUMOF
};


struct roomCell{
	TileMapData roomMap;
	int roomWidth;
	int roomHeight;
	int doorType;
	int roomType;
	int roomID;
	bool isNecess;
};

struct TestRoom:roomCell {
	TestRoom();
};

#pragma region BossRoom

struct BossRoom:roomCell {
	BossRoom();
};

#pragma endregion

#pragma region TreasureRoom

struct JewelRoom:roomCell {
	JewelRoom(int perlin);
};

#pragma endregion

#pragma region NormalRoom

struct NoRoom:roomCell {
	NoRoom();
};

struct MonsterRoom1:roomCell {
	MonsterRoom1();
};

struct MonsterRoomFormMIHARA:roomCell {
	MonsterRoomFormMIHARA();
};

#pragma endregion




roomCell* RoomSeach(int roomID,float perlin);