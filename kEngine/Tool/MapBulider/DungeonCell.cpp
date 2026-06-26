#include "DungeonCell.h"

TestRoom::TestRoom() {
	roomMap.Column = { 3,3,3,3,3,8,8,3,3,3,3,3 }; roomMap.ColumnPush();
	roomMap.Column = { 3,0,0,0,0,0,0,0,0,0,0,3 }; roomMap.ColumnPush();
	roomMap.Column = { 3,0,0,0,0,0,0,0,0,0,0,3 }; roomMap.ColumnPush();
	roomMap.Column = { 8,0,0,0,0,0,0,0,0,0,0,8 }; roomMap.ColumnPush();
	roomMap.Column = { 8,0,0,0,0,0,0,0,0,0,0,8 }; roomMap.ColumnPush();
	roomMap.Column = { 3,3,3,3,3,3,3,8,8,3,3,3 }; roomMap.ColumnPush();
	roomWidth = static_cast<int>(roomMap.Row[0].size());
	roomHeight = static_cast<int>(roomMap.Row.size());

	for (int y = 0; y < roomHeight; y++) {
		for (int x = 0; x < roomWidth; x++) {
			if (roomMap.Row[y][x] == 0) { roomMap.Row[y][x] = AIR;    continue; }
			if (roomMap.Row[y][x] == 3) { roomMap.Row[y][x] = ROCK;    continue; }
			if (roomMap.Row[y][x] == 8) { roomMap.Row[y][x] = DUNGEONDOOR; continue; }
		}
	}

	doorType = NONE;
	roomType = ROOMTYPE_START;
	roomID = ROOMID_TEST;
	isNecess = true;
}

BossRoom::BossRoom() {
	roomMap.Column = { 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3 }; roomMap.ColumnPush();
	roomMap.Column = { 3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3 }; roomMap.ColumnPush();
	roomMap.Column = { 3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3 }; roomMap.ColumnPush();
	roomMap.Column = { 3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3 }; roomMap.ColumnPush();
	roomMap.Column = { 3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3 }; roomMap.ColumnPush();
	roomMap.Column = { 8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,8 }; roomMap.ColumnPush();
	roomMap.Column = { 8,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,0,8 }; roomMap.ColumnPush();
	roomMap.Column = { 3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3 }; roomMap.ColumnPush();
	roomWidth = static_cast<int>(roomMap.Row[0].size());
	roomHeight = static_cast<int>(roomMap.Row.size());

	for (int y = 0; y < roomHeight; y++) {
		for (int x = 0; x < roomWidth; x++) {
			if (roomMap.Row[y][x] == 0) { roomMap.Row[y][x] = AIR;    continue; }
			if (roomMap.Row[y][x] == 3) { roomMap.Row[y][x] = ROCK;    continue; }
			if (roomMap.Row[y][x] == 8) { roomMap.Row[y][x] = DUNGEONDOOR; continue; }
			if (roomMap.Row[y][x] == 7) { roomMap.Row[y][x] = MONSTERSPAWNER; continue; }
		}
	}

	doorType = NONE;
	roomType = ROOMTYPE_BOSS;
	roomID = ROOMID_BOSS;
	isNecess = true;
}

JewelRoom::JewelRoom(int perlin) {
	int rate = perlin % 3;

	switch(rate){
	case 0:
		roomMap.Column = { 3,3,3,3,3,3,3,3,3,3,3,3 }; roomMap.ColumnPush();
		roomMap.Column = { 3,1,1,1,2,1,1,4,1,1,2,3 }; roomMap.ColumnPush();
		roomMap.Column = { 3,1,2,2,2,2,1,2,2,1,1,3 }; roomMap.ColumnPush();
		roomMap.Column = { 8,1,1,2,1,2,1,1,1,1,1,3 }; roomMap.ColumnPush();
		roomMap.Column = { 8,1,1,1,1,1,1,1,2,1,1,3 }; roomMap.ColumnPush();
		roomMap.Column = { 3,3,3,3,3,3,3,3,3,3,3,3 }; roomMap.ColumnPush();
		roomWidth = static_cast<int>(roomMap.Row[0].size());
		roomHeight = static_cast<int>(roomMap.Row.size());
		break;
	case 1:
		roomMap.Column = { 3,3,3,3,3,3,3,3,3,3,3,3 }; roomMap.ColumnPush();
		roomMap.Column = { 3,1,1,1,2,1,1,1,1,2,2,3 }; roomMap.ColumnPush();
		roomMap.Column = { 3,1,2,4,2,2,1,2,2,1,1,3 }; roomMap.ColumnPush();
		roomMap.Column = { 3,1,2,1,1,1,2,1,2,1,2,8 }; roomMap.ColumnPush();
		roomMap.Column = { 3,1,1,2,1,2,1,1,2,1,1,8 }; roomMap.ColumnPush();
		roomMap.Column = { 3,3,3,3,3,3,3,3,3,3,3,3 }; roomMap.ColumnPush();
		roomWidth = static_cast<int>(roomMap.Row[0].size());
		roomHeight = static_cast<int>(roomMap.Row.size());
		break;
	case 2:
		roomMap.Column = { 3,3,3,3,3,3,3,3,3,3,3,3 }; roomMap.ColumnPush();
		roomMap.Column = { 3,2,1,2,2,1,1,4,1,1,1,3 }; roomMap.ColumnPush();
		roomMap.Column = { 3,1,1,1,4,1,1,1,2,1,1,3 }; roomMap.ColumnPush();
		roomMap.Column = { 3,1,1,4,4,4,1,1,1,1,1,3 }; roomMap.ColumnPush();
		roomMap.Column = { 3,1,1,2,1,1,1,1,1,1,1,3 }; roomMap.ColumnPush();
		roomMap.Column = { 3,3,3,3,3,3,3,8,8,3,3,3 }; roomMap.ColumnPush();
		roomWidth = static_cast<int>(roomMap.Row[0].size());
		roomHeight = static_cast<int>(roomMap.Row.size());
		break;
	}

	for (int y = 0; y < roomHeight; y++) {
		for (int x = 0; x < roomWidth; x++) {
			if (roomMap.Row[y][x] == 1) { roomMap.Row[y][x] = IRON;    continue;}
			if (roomMap.Row[y][x] == 2) { roomMap.Row[y][x] = GOLD;    continue;}
			if (roomMap.Row[y][x] == 4) { roomMap.Row[y][x] = DIAMOND; continue;}
			if (roomMap.Row[y][x] == 3) { roomMap.Row[y][x] = ROCK;    continue;}
			if (roomMap.Row[y][x] == 8) { roomMap.Row[y][x] = DUNGEONDOOR; continue; }
		}
	}

	doorType = NONE;
	roomType = ROOMTYPE_TREASURE;
	roomID = ROOMID_JEWEL;
	isNecess = true;
}

NoRoom::NoRoom() {
	roomMap.Row.resize(0);
	roomWidth = 0;
	roomHeight = 0;
	doorType = NONE;
	roomType = ROOMTYPE_NORMAL;
	roomID = ROOMID_NO;
	isNecess = true;
}

MonsterRoom1::MonsterRoom1() {
	roomMap.Column = { 0,0,3,8,8,8,8,3,0,0,0,0 }; roomMap.ColumnPush();
	roomMap.Column = { 0,0,3,1,1,1,1,3,0,0,0,0 }; roomMap.ColumnPush();
	roomMap.Column = { 3,3,3,1,1,1,1,3,3,3,3,3 }; roomMap.ColumnPush();
	roomMap.Column = { 3,1,3,3,3,1,1,1,1,1,1,8 }; roomMap.ColumnPush();
	roomMap.Column = { 8,1,1,1,1,1,1,1,3,3,3,3 }; roomMap.ColumnPush();
	roomMap.Column = { 8,1,1,1,1,1,1,7,1,1,1,3 }; roomMap.ColumnPush();
	roomMap.Column = { 3,3,3,3,3,3,3,8,8,3,3,3 }; roomMap.ColumnPush();
	roomWidth = static_cast<int>(roomMap.Row[0].size());
	roomHeight = static_cast<int>(roomMap.Row.size());

	for (int y = 0; y < roomHeight; y++) {
		for (int x = 0; x < roomWidth; x++) {
			if (roomMap.Row[y][x] == 0) { roomMap.Row[y][x] = NONE;           continue; }
			if (roomMap.Row[y][x] == 1) { roomMap.Row[y][x] = AIR;            continue; }
			if (roomMap.Row[y][x] == 3) { roomMap.Row[y][x] = ROCK;           continue; }
			if (roomMap.Row[y][x] == 8) { roomMap.Row[y][x] = DUNGEONDOOR;    continue; }
			if (roomMap.Row[y][x] == 7) { roomMap.Row[y][x] = MONSTERSPAWNER; continue; }
		}
	}

	doorType = NONE;
	roomType = ROOMTYPE_NORMAL;
	roomID = ROOMID_MONSTER1;
	isNecess = true;
}

MonsterRoomFormMIHARA::MonsterRoomFormMIHARA() {
	roomMap.Column = { 3,3,3,3,3,8,8,3,3,3,3,3 }; roomMap.ColumnPush();
	roomMap.Column = { 3,3,3,0,0,0,0,0,0,0,0,3 }; roomMap.ColumnPush();
	roomMap.Column = { 3,3,7,0,0,0,0,0,0,0,0,8 }; roomMap.ColumnPush();
	roomMap.Column = { 3,3,3,3,0,0,7,0,0,0,0,8 }; roomMap.ColumnPush();
	roomMap.Column = { 8,0,0,0,0,1,1,1,0,0,1,3 }; roomMap.ColumnPush();
	roomMap.Column = { 8,0,0,0,0,0,0,0,0,0,0,3 }; roomMap.ColumnPush();
	roomMap.Column = { 3,3,3,3,3,3,3,8,8,3,3,3 }; roomMap.ColumnPush();
	roomWidth = static_cast<int>(roomMap.Row[0].size());
	roomHeight = static_cast<int>(roomMap.Row.size());

	for (int y = 0; y < roomHeight; y++) {
		for (int x = 0; x < roomWidth; x++) {
			if (roomMap.Row[y][x] == 0) { roomMap.Row[y][x] = AIR;    continue; }
			if (roomMap.Row[y][x] == 3) { roomMap.Row[y][x] = ROCK;    continue; }
			if (roomMap.Row[y][x] == 8) { roomMap.Row[y][x] = DUNGEONDOOR; continue; }
			if (roomMap.Row[y][x] == 7) { roomMap.Row[y][x] = MONSTERSPAWNER; continue; }
		}
	}

	doorType = NONE;
	roomType = ROOMTYPE_NORMAL;
	roomID = ROOMID_MONSTERFORMIHARA;
	isNecess = true;
}


roomCell* RoomSeach(int roomID, float perlin) {
	perlin;
	roomCell* room{};
	if (roomID == ROOMID_TEST) { room = new TestRoom(); }
	if (roomID == ROOMID_BOSS) { room = new BossRoom(); }
	if (roomID == ROOMID_JEWEL) { room = new JewelRoom((int)perlin); }
	if (roomID == ROOMID_NO) { room = new NoRoom(); }
	if (roomID == ROOMID_MONSTER1) { room = new MonsterRoom1(); }
	if (roomID == ROOMID_MONSTERFORMIHARA) { room = new MonsterRoomFormMIHARA(); }
	return room;
}