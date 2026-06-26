#include "GBPDungeonGenerater.h"

GBPDungeonGenerater::~GBPDungeonGenerater() {
	delete perlinNoise, perlinNoise = nullptr;
}

void GBPDungeonGenerater::SetDungeonGenerater(int maxRow, int maxCol, int seed) {
	maxRow_ = maxRow;
	maxCol_ = maxCol;
	seed_ = seed + 105;
	perlinNoise->SetSeed(seed_);

	/// 参考用パリーノイズ地図を作成
	mapData.Row.resize(maxRow_);
	for (int i = 0; i < maxRow_; i++) {
		mapData.Row[i].resize(maxCol_);
	}
	for (int y = 0; y < maxRow_; y++) {
		for (int x = 0; x < maxCol_; x++) {
			mapData.Row[y][x] = int(perlinNoise->fBm(
				Vector3{ float(x),float(y),1.0f } *Vector3{ 0.1f, 0.1f, 0.1f },
				1, 1.0f, 1.0f, 1.0f) * 255);
		}
	}
}

void GBPDungeonGenerater::RoomSpawner(TileMapData* targetMap, std::vector<GBPDungeonGenerater::RoomData>& roomData, int stopper) {
	///もしroomDataが空なら、何かの間違いがありがち、もっかいチェック
	if (roomData.empty())assert(false);
	int roomCounter = 0;
	roomCell* referenceRoom{};
	while (roomCounter < (int)roomData.size()) {
		referenceRoom = RoomSeach(roomData[roomCounter].roomID, (float)roomData[roomCounter].perlinValue);
		int LTx = roomData[roomCounter].roomCenterX - referenceRoom->roomWidth / 2;
		int LTy = roomData[roomCounter].roomCenterY - referenceRoom->roomHeight / 2;
		for (int y = 0; y < referenceRoom->roomHeight; y++) {
			for (int x = 0; x < referenceRoom->roomWidth; x++) {
				if ((LTx + x) < 0 || (LTx + x) >= maxCol_ || (LTy + y) < 0 || (LTy + y) >= maxRow_)continue;
				if (targetMap->Row[LTy + y][LTx + x] == stopper)continue;
				if (referenceRoom->roomMap.Row[y][x] == NONE)continue;
				targetMap->Row[LTy + y][LTx + x] = referenceRoom->roomMap.Row[y][x];
			}
		}
		roomCounter++;
		delete referenceRoom; referenceRoom = nullptr;
	}
}


void GBPDungeonGenerater::RoomSort(std::vector<GBPDungeonGenerater::RoomData>& roomData) {
	std::vector<RoomData> result{};
	RoomData handRoom = roomData[0];
	for (int roomCounter = 0; roomCounter < (int)roomData.size(); roomCounter++) {
		handRoom.dataPointX = maxCol_;
		handRoom.dataPointY = maxRow_;
		for (int roomCounter2 = 0; roomCounter2 < (int)roomData.size(); roomCounter2++) {
			bool isSkip = false;
			if (!result.empty()) {
				for (const auto& i : result) {
					if (i.dataPointX == roomData[roomCounter2].dataPointX && i.dataPointY == roomData[roomCounter2].dataPointY) {
						isSkip = true;
						break;
					}
				}
			}
			if (isSkip)continue;
			if (handRoom.dataPointY > roomData[roomCounter2].dataPointY) { handRoom = roomData[roomCounter2]; }
			if (handRoom.dataPointY == roomData[roomCounter2].dataPointY) {
				if (handRoom.dataPointX > roomData[roomCounter2].dataPointX) {
					handRoom = roomData[roomCounter2];
				}
			}
		}
		result.push_back(handRoom);
	}
	roomData = result;
}

void GBPDungeonGenerater::RoadSpawnerHorizon(TileMapData* targetMap, std::vector<GBPDungeonGenerater::RoomData>& roomData, int stopper) {
	///もしroomDataが空なら、何かの間違いがありがち、もっかいチェック
	if (roomData.empty())assert(false);
	roomCell* referenceRoom{};
	for (int roomCounter = 0; roomCounter < (int)roomData.size() - 1; roomCounter++) {
		referenceRoom = RoomSeach(roomData[roomCounter].roomID, (float)roomData[roomCounter].perlinValue);
		/// 右端の門
		{
			// 右端の門を捜索
			int rightDoorPosX{};
			int rightDoorPosY{};
			int HeightOfDoor{};
			bool isDoorFound = false;
			for (int x = roomData[roomCounter].roomCenterX + referenceRoom->roomWidth / 2 - 1; x > roomData[roomCounter].roomCenterX; x--) {
				for (int y = roomData[roomCounter].roomCenterY - referenceRoom->roomHeight / 2; y < roomData[roomCounter].roomCenterY + referenceRoom->roomHeight / 2 - 1; y++) {
					if (targetMap->Row[y][x] == DUNGEONDOOR) {
						if (targetMap->Row[y][x - 1] == DUNGEONDOOR)continue;
						if (targetMap->Row[y][x + 1] == DUNGEONDOOR)continue;
						rightDoorPosX = x + 1;
						rightDoorPosY = y;
						HeightOfDoor = 1;
						int counter = 0;
						while (true) {
							if (y > maxRow_) break;
							if (targetMap->Row[y + counter][x] != DUNGEONDOOR) break;
							HeightOfDoor++;
							counter++;
						}
						isDoorFound = true;
						goto found;
					}
				}
			}
		found:
			if (!isDoorFound)continue;

			// 右端の門の次の門を捜索
			bool isNextDoorFound = false;
			int nextDoorPosX{};
			int nextDoorPosY{};
			int HeightOfNextDoor{};

			//　次に左端に門がある部屋を捜索
			int roomCounter2 = roomCounter + 1;
			// 同じY座標じゃない部屋は無視
			if (roomData[roomCounter].dataPointY != roomData[roomCounter2].dataPointY) continue;
			if (roomCounter2 == (int)roomData.size() - 1 && roomData[roomCounter2].roomID == ROOMID_NO)break;
			// 部屋内の右端の門を捜索
			int skipCounter = 0;
			for (skipCounter = 0; (roomCounter2 + skipCounter) * dungeonData->EachRoomWidth - dungeonData->EachRoomWidth / 2 < (int)roomData.size(); skipCounter++) {
				if (roomData[roomCounter2 + skipCounter].roomID != ROOMID_NO) {
					break;
				}
			}
			roomCounter2 += skipCounter;
			// 捜索開始
			roomCell* referenceRoom2 = RoomSeach(roomData[roomCounter2].roomID, (float)roomData[roomCounter2].perlinValue);
			for (int nx = roomData[roomCounter2].roomCenterX - referenceRoom2->roomWidth / 2; nx < roomData[roomCounter2].roomCenterX; nx++) {
				for (int ny = roomData[roomCounter2].roomCenterY - referenceRoom2->roomHeight / 2; ny < roomData[roomCounter2].roomCenterY + referenceRoom2->roomHeight / 2; ny++) {
					if (targetMap->Row[ny][nx] == DUNGEONDOOR) {
						if (targetMap->Row[ny][nx + 1] == DUNGEONDOOR)continue;
						if (targetMap->Row[ny][nx - 1] == DUNGEONDOOR)continue;

						nextDoorPosX = nx;
						nextDoorPosY = ny;
						HeightOfNextDoor = 1;
						int counter = 0;
						while (true) {
							if (ny > maxRow_) break;
							if (targetMap->Row[ny + counter][nx] != DUNGEONDOOR) break;
							HeightOfNextDoor++;
							counter++;
						}
						isNextDoorFound = true;
						goto found2;
					}
				}
			}
		found2:
			delete referenceRoom2, referenceRoom2 = nullptr;
			//}
			if (!isNextDoorFound)continue;
			// もし条件が合ったら、道を作る
			int c = nextDoorPosX - rightDoorPosX;
			for (int t = 0; t < c; t++) {
				float ratio = float(t) / float(c);

				int nx = rightDoorPosX + t;
				int ny = int(rightDoorPosY + (nextDoorPosY - rightDoorPosY) * ratio);
				int dh = int(HeightOfDoor + (HeightOfDoor - HeightOfNextDoor) * ratio);
				if (targetMap->Row[ny][nx] != AIR) {
					if (targetMap->Row[ny][nx] != stopper) {
						targetMap->Row[ny][nx] = AIR;
						int handle = 0;
						while (handle < dh) {
							handle++;
							if ((ny + 1) < 0 || (ny + 1) >= maxRow_)continue;
							targetMap->Row[ny + 1][nx] = AIR;
						}
					}
				}
			}
		}
	}
	delete referenceRoom; referenceRoom = nullptr;
}

void GBPDungeonGenerater::RoadSpawnerVertical(TileMapData* targetMap, std::vector<GBPDungeonGenerater::RoomData>& roomData, int stopper) {
	///もしroomDataが空なら、何かの間違いがありがち、もっかいチェック
	if (roomData.empty())assert(false);
	roomCell* referenceRoom{};
	for (int roomCounter = 0; roomCounter < (int)roomData.size() - 1; roomCounter++) {
		referenceRoom = RoomSeach(roomData[roomCounter].roomID, (float)roomData[roomCounter].perlinValue);
		/// 下端の門
		{
			// 下端の門を捜索
			int downDoorPosX{};
			int downDoorPosY{};
			int WidthOfDoor{};
			bool isDoorFound = false;
			for (int y = roomData[roomCounter].roomCenterY + referenceRoom->roomHeight / 2; y > roomData[roomCounter].roomCenterY; y--) {
				for (int x = roomData[roomCounter].roomCenterX - referenceRoom->roomWidth / 2; x < roomData[roomCounter].roomCenterX + referenceRoom->roomWidth / 2 - 1; x++) {
					if (targetMap->Row[y][x] == DUNGEONDOOR) {
						if (targetMap->Row[y - 1][x] == DUNGEONDOOR)continue;
						if (targetMap->Row[y + 1][x] == DUNGEONDOOR)continue;
						downDoorPosX = x;
						downDoorPosY = y + 1;
						WidthOfDoor = 1;
						int counter = WidthOfDoor;
						while (true) {
							if (x > maxCol_) break;
							if (targetMap->Row[y][x + counter] != DUNGEONDOOR) break;
							WidthOfDoor++;
							counter++;
						}
						WidthOfDoor--;
						isDoorFound = true;
						goto found;
					}
				}
			}
		found:
			delete referenceRoom; referenceRoom = nullptr;
			if (!isDoorFound)continue;
			// 下端の門の次の門を捜索
			bool isNextDoorFound = false;
			int nextDoorPosX{};
			int nextDoorPosY{};
			int WidthOfNextDoor{};
			//　次に上端に門がある部屋を捜索
			int roomCounter2{};
			// 同じX座標の部屋を捜索
			for (roomCounter2 = roomCounter + 1; roomCounter2 < (int)roomData.size(); roomCounter2++) {
				if (roomData[roomCounter].dataPointX == roomData[roomCounter2].dataPointX)break;
			}
			if (roomCounter2 == (int)roomData.size())continue;
			if (roomCounter2 == (int)roomData.size() - 1 && roomData[roomCounter2].roomID == ROOMID_NO)break;
			// 部屋内の上端の門を捜索
			roomCell* referenceRoom2 = RoomSeach(roomData[roomCounter2].roomID, (float)roomData[roomCounter2].perlinValue);
			for (int ny = roomData[roomCounter2].roomCenterY - referenceRoom2->roomHeight / 2; ny < roomData[roomCounter2].roomCenterY; ny++) {
				for (int nx = roomData[roomCounter2].roomCenterX - referenceRoom2->roomWidth / 2; nx < roomData[roomCounter2].roomCenterX + referenceRoom2->roomWidth / 2; nx++) {
					if (targetMap->Row[ny][nx] == DUNGEONDOOR) {
						if (targetMap->Row[ny + 1][nx] == DUNGEONDOOR)continue;
						if (targetMap->Row[ny - 1][nx] == DUNGEONDOOR)continue;

						nextDoorPosX = nx;
						nextDoorPosY = ny;
						WidthOfNextDoor = 1;
						int counter = 0;
						while (true) {
							if (nx > maxCol_) break;
							if (targetMap->Row[ny + 1][nx + counter] != DUNGEONDOOR) break;
							WidthOfNextDoor++;
							counter++;
						}

						isNextDoorFound = true;
						goto found2;
					}
				}
			}
		found2:
			delete referenceRoom2, referenceRoom2 = nullptr;
			//}
			if (!isNextDoorFound)continue;
			// もし条件が合ったら、道を作る
			int c = nextDoorPosY - downDoorPosY;
			for (int t = 0; t < c; t++) {
				float ratio = float(t) / float(c);

				int nx = int(downDoorPosX + (nextDoorPosX - downDoorPosX) * ratio);
				int ny = downDoorPosY + t;
				int dw = int(WidthOfDoor + (WidthOfDoor - WidthOfNextDoor) * ratio);

				if (targetMap->Row[ny][nx] != AIR) {
					if (targetMap->Row[ny][nx] != stopper) {
						targetMap->Row[ny][nx] = AIR;
						int handle = 0;
						while (handle < dw) {
							handle++;
							if ((nx + handle) < 0 || (nx + handle) >= maxCol_)continue;
							targetMap->Row[ny][nx + handle] = AIR;
						}
					}
				}
			}
		}
	}
	
}


void GBPDungeonGenerater::RoadSpawner(TileMapData* targetMap, std::vector<GBPDungeonGenerater::RoomData>& roomData, int stopper) {
	RoadSpawnerHorizon(targetMap, roomData, stopper);
	RoadSpawnerVertical(targetMap, roomData, stopper);
}


Vector2 GBPDungeonGenerater::CenterTunner(int cx, int cy, int dungeonWidth, int dungeonHeight) {
	Vector2 resuit = { (float)cx,(float)cy };
	if ((cx + dungeonWidth / 2) > maxCol_) { resuit.x = float(maxCol_ - dungeonWidth); }
	if ((cy + dungeonHeight / 2) > maxRow_) { resuit.y = float(maxRow_ - dungeonHeight); }
	if ((cx - dungeonWidth / 2) < 0) { resuit.x = float(dungeonWidth); }
	if ((cy - dungeonHeight / 2) < 0) { resuit.y = float(dungeonHeight); }
	return resuit;
}

void GBPDungeonGenerater::DungeonSpawner(TileMapData* targetMap, int cx, int cy, int stopper) {

	int nx = cx + 1;
	int ny = cy + 1;
	if (nx < 0 || nx >= maxCol_ || ny < 0 || ny >= maxRow_) {
		nx = cx - 1;
		ny = cy - 1;
	};
	float spawnRate = float(mapData.Row[ny][nx] % 10) / 10.0f;
	int dungeonWidthRN = int(dungeonData->MinWidth + (dungeonData->MaxWidth - dungeonData->MinWidth) * spawnRate);
	int dungeonHeightRN = int(dungeonData->MinHeight + (dungeonData->MaxHeight - dungeonData->MinHeight) * spawnRate);
	int dungeonWidth = dungeonWidthRN * dungeonData->EachRoomWidth;
	int dungeonHeight = dungeonHeightRN * dungeonData->EachRoomHeight;

	/// 中心修正
	int adjustedcx = 0;
	int adjustedcy = 0;
	{
		Vector2 handle = CenterTunner(cx, cy, dungeonWidth, dungeonHeight);
		adjustedcx = (int)handle.x;
		adjustedcy = (int)handle.y;
	}

	///// 部屋定義
	//int numberOfRooms = dungeonWidthRN * dungeonHeightRN;
	std::vector<RoomData>roomData;

	/// Boss部屋判断
	bossRoomDirecter(adjustedcx, adjustedcy, dungeonHeightRN, dungeonWidthRN, dungeonWidth, dungeonHeight, roomData);

	/// 宝部屋判断
	int treasureRoomNumber = 1;
	treasureRoomDirecter(adjustedcx, adjustedcy, dungeonHeightRN, dungeonWidthRN, dungeonWidth, dungeonHeight, treasureRoomNumber, roomData);

	/// 普通部屋判断
	normalRoomDirecter(adjustedcx, adjustedcy, dungeonHeightRN, dungeonWidthRN, dungeonWidth, dungeonHeight, roomData);

	RoomSort(roomData);

	RoomSpawner(targetMap, roomData, stopper);


	RoadSpawner(targetMap, roomData, stopper);

	roomData.clear();
}



void GBPDungeonGenerater::bossRoomDirecter(int adjustedcx, int adjustedcy,
	int dungeonHeightRN, int dungeonWidthRN,
	int dungeonWidth, int dungeonHeight,
	std::vector<RoomData>& roomData) {
	int nx = 0;
	int ny = 0;
	int maxPerlinValue = 0;
	int hx = 0;
	int hy = 0;
	/// 部屋になるところの中心のPerlin値を使って、一番高いやつをBoss部屋に特定
	for (int y = 0; y < dungeonHeightRN; y++) {
		for (int x = 0; x < dungeonWidthRN; x++) {
			bool isSkip = false;
			nx = adjustedcx - dungeonWidth / 2 + x * dungeonData->EachRoomWidth + dungeonData->EachRoomWidth / 2;
			ny = adjustedcy - dungeonHeight / 2 + y * dungeonData->EachRoomHeight + dungeonData->EachRoomHeight / 2;
			int nowPerlinValue = mapData.Row[ny][nx];
			/// すでに部屋があるならスキップ
			if (!roomData.empty()) {
				for (const auto& i : roomData) {
					if (i.dataPointX == nx && i.dataPointY == ny) {
						isSkip = true;
						break;
					}
				}
			}
			if (maxPerlinValue < nowPerlinValue) {
				maxPerlinValue = nowPerlinValue;
				hx = nx;
				hy = ny;
			}
		}
	}
	/// 見つからないなら何か変、checkしよう
	if (hx == 0 && hy == 0) {
		char buffer[256];
		sprintf_s(buffer, "GBPDG::bossRoomDirecter() : Wrong Position\n");
		OutputDebugStringA(buffer);
		assert(false);
	}

	///　部屋決定、Boss部屋種類決定
	nx = hx + 1;
	ny = hy + 1;
	if (nx < 0 || nx >= maxCol_ || ny < 0 || ny >= maxRow_) {
		nx = hx - 1;
		ny = hy - 1;
	};
	int numOfAllBossRoom = (int)dungeonData->bossRoomsID.size();
	roomCell* bossRoom = RoomSeach(dungeonData->bossRoomsID[mapData.Row[ny][nx] % numOfAllBossRoom], (float)mapData.Row[hy][hx]);
	/// 部屋ずらす
	int adjNx = 0;
	int adjNy = 0;
	nx = hx - (dungeonData->EachRoomWidth / 2 - bossRoom->roomWidth / 2 - 1);
	ny = hy - (dungeonData->EachRoomHeight / 2 - bossRoom->roomHeight / 2 - 1);

	int ex = hx + (dungeonData->EachRoomWidth / 2 - bossRoom->roomWidth / 2 - 1);
	int ey = hy + (dungeonData->EachRoomHeight / 2 - bossRoom->roomHeight / 2 - 1);

	if (nx == ex) { adjNx = ex; }
	if (ny == ey) { adjNy = ey; }

	maxPerlinValue = 0;
	for (int y = ny; y < ey; y++) {
		for (int x = nx; x < ex; x++) {
			int nowPerlinValue = mapData.Row[y][x];
			if (maxPerlinValue < nowPerlinValue) {
				maxPerlinValue = nowPerlinValue;
				adjNx = x;
				adjNy = y;
			}
		}
	}


	/// データ保存
	RoomData bossRoomData = {
		hx,hy,
		adjNx,adjNy,
		ROOMTYPE_BOSS,
		bossRoom->roomID,
		mapData.Row[hy][hx] };
	roomData.push_back(bossRoomData);
	delete bossRoom;
}

void GBPDungeonGenerater::startRoomDirecter(int adjustedcx, int adjustedcy,
	int dungeonHeightRN, int dungeonWidthRN,
	int dungeonWidth, int dungeonHeight,
	std::vector<GBPDungeonGenerater::RoomData>& roomData) {
	int nx = 0;
	int ny = 0;
	int minPerlinValue = 256;
	int lx = 0;
	int ly = 0;
	/// 部屋になるところの中心のPerlin値を使って、一番低いやつをstart部屋に特定
	for (int y = 0; y < dungeonHeightRN; y++) {
		for (int x = 0; x < dungeonWidthRN; x++) {
			bool isSkip = false;
			nx = adjustedcx - dungeonWidth / 2 + x * dungeonData->EachRoomWidth + dungeonData->EachRoomWidth / 2;
			ny = adjustedcy - dungeonHeight / 2 + y * dungeonData->EachRoomHeight + dungeonData->EachRoomHeight / 2;
			int nowPerlinValue = mapData.Row[ny][nx];
			/// すでに部屋があるならスキップ
			if (!roomData.empty()) {
				for (const auto& i : roomData) {
					if (i.dataPointX == nx && i.dataPointY == ny) {
						isSkip = true;
						break;
					}
				}
			}
			if (isSkip)continue;
			if (minPerlinValue > nowPerlinValue) {
				minPerlinValue = nowPerlinValue;
				lx = nx;
				ly = ny;
			}
		}
	}
	/// 見つからないなら何か変、checkしよう
	if (lx == 0 && ly == 0) {
		char buffer[256];
		sprintf_s(buffer, "GBPDG::treasureRoomDirecter() : Wrong Position\n");
		OutputDebugStringA(buffer);
		assert(false);
	}
	///　スタート部屋決定
	nx = lx + 1;
	ny = ly + 1;
	if (nx < 0 || nx >= maxCol_ || ny < 0 || ny >= maxRow_) {
		nx = lx - 1;
		ny = ly - 1;
	};
	int startRoomID = dungeonData->startRoomID;
	roomCell* startRoom = RoomSeach(startRoomID, (float)mapData.Row[ly][lx]);
	/// 部屋ずらす
	int adjNx = 0;
	int adjNy = 0;
	nx = lx - (dungeonData->EachRoomWidth / 2 - startRoom->roomWidth / 2 - 1);
	ny = ly - (dungeonData->EachRoomHeight / 2 - startRoom->roomHeight / 2 - 1);

	int ex = lx + (dungeonData->EachRoomWidth / 2 - startRoom->roomWidth / 2 - 1);
	int ey = ly + (dungeonData->EachRoomHeight / 2 - startRoom->roomHeight / 2 - 1);

	minPerlinValue = 256;
	for (int y = ny; y < ey; y++) {
		for (int x = nx; x < ex; x++) {
			int nowPerlinValue = mapData.Row[y][x];
			if (minPerlinValue > nowPerlinValue) {
				minPerlinValue = nowPerlinValue;
				adjNx = x;
				adjNy = y;
			}
		}
	}
	/// データ保存
	RoomData treasureRoomData = {
		lx,ly,
		adjNx,adjNy,
		ROOMTYPE_START,
		startRoomID,
		mapData.Row[ly][lx] };
	roomData.push_back(treasureRoomData);
	delete startRoom;
}

void GBPDungeonGenerater::treasureRoomDirecter(int adjustedcx, int adjustedcy,
	int dungeonHeightRN, int dungeonWidthRN,
	int dungeonWidth, int dungeonHeight,
	int treasureRoomNumber,
	std::vector<GBPDungeonGenerater::RoomData>& roomData) {

	/// 部屋になるところの中心のPerlin値を使って、一番低いやつからを宝部屋に特定
	for (int treasureRoomCounter = 0; treasureRoomCounter < treasureRoomNumber; treasureRoomCounter++) {
		int nx = 0;
		int ny = 0;
		int minPerlinValue = 256;
		int lx = 0;
		int ly = 0;
		for (int y = 0; y < dungeonHeightRN; y++) {
			for (int x = 0; x < dungeonWidthRN; x++) {
				bool isSkip = false;
				nx = adjustedcx - dungeonWidth / 2 + x * dungeonData->EachRoomWidth + dungeonData->EachRoomWidth / 2;
				ny = adjustedcy - dungeonHeight / 2 + y * dungeonData->EachRoomHeight + dungeonData->EachRoomHeight / 2;
				/// すでに部屋があるならスキップ
				if (!roomData.empty()) {
					for (const auto& i : roomData) {
						if (i.dataPointX == nx && i.dataPointY == ny) {
							isSkip = true;
							break;
						}
					}
				}
				if (isSkip)continue;
				int nowPerlinValue = mapData.Row[ny][nx];
				if (minPerlinValue > nowPerlinValue) {
					minPerlinValue = nowPerlinValue;
					lx = nx;
					ly = ny;
				}
			}
		}

		/// 見つからないなら何か変、checkしよう
		if (lx == 0 && ly == 0) {
			char buffer[256];
			sprintf_s(buffer, "GBPDG::treasureRoomDirecter() : Wrong Position\n");
			OutputDebugStringA(buffer);
			assert(false);
		}
		///　スタート部屋決定
		nx = lx + 1;
		ny = ly + 1;
		if (nx < 0 || nx >= maxCol_ || ny < 0 || ny >= maxRow_) {
			nx = lx - 1;
			ny = ly - 1;
		};
		int numOfTreasureRoom = (int)dungeonData->treasureRoomsID.size();
		roomCell* treasureRoom = RoomSeach(dungeonData->treasureRoomsID[mapData.Row[ny][nx] % numOfTreasureRoom], (float)mapData.Row[ly][lx]);

		/// 部屋ずらす
		int adjNx = 0;
		int adjNy = 0;
		nx = lx - (dungeonData->EachRoomWidth / 2 - treasureRoom->roomWidth / 2 - 1);
		ny = ly - (dungeonData->EachRoomHeight / 2 - treasureRoom->roomHeight / 2 - 1);

		int ex = lx + (dungeonData->EachRoomWidth / 2 - treasureRoom->roomWidth / 2 - 1);
		int ey = ly + (dungeonData->EachRoomHeight / 2 - treasureRoom->roomHeight / 2 - 1);

		minPerlinValue = 256;
		for (int y = ny; y < ey; y++) {
			for (int x = nx; x < ex; x++) {
				int nowPerlinValue = mapData.Row[y][x];
				if (minPerlinValue > nowPerlinValue) {
					minPerlinValue = nowPerlinValue;
					adjNx = x;
					adjNy = y;
				}
			}
		}
		/// データ保存
		RoomData treasureRoomData = {
			lx,ly,
			adjNx,adjNy,
			ROOMTYPE_TREASURE,
			treasureRoom->roomID,
			mapData.Row[ly][lx] };
		roomData.push_back(treasureRoomData);
		delete treasureRoom;
	}
}

void GBPDungeonGenerater::normalRoomDirecter(int adjustedcx, int adjustedcy,
	int dungeonHeightRN, int dungeonWidthRN,
	int dungeonWidth, int dungeonHeight,
	std::vector<GBPDungeonGenerater::RoomData>& roomData) {

	/// 部屋になるところの中心のPerlin値を使って、一番低いやつからを宝部屋に特定
	for (int dy = 0; dy < dungeonHeightRN; dy++) {
		for (int dx = 0; dx < dungeonWidthRN; dx++) {
			bool isSkip = false;
			int nx = adjustedcx - dungeonWidth / 2 + dx * dungeonData->EachRoomWidth + dungeonData->EachRoomWidth / 2;
			int ny = adjustedcy - dungeonHeight / 2 + dy * dungeonData->EachRoomHeight + dungeonData->EachRoomHeight / 2;
			if (!roomData.empty()) {
				for (const auto& i : roomData) {
					if (i.dataPointX == nx && i.dataPointY == ny) {
						isSkip = true;
						break;
					}
				}
			}
			if (isSkip)continue;
			/// 部屋の種類を決める
			int nnx = nx + 1;
			int nny = ny + 1;
			if (nnx < 0 || nnx >= maxCol_ || nny < 0 || nny >= maxRow_) {
				nnx = nx - 1;
				nny = ny - 1;
			};
			roomCell* normalRoom = RoomSeach(dungeonData->RoomSeachByRate(ROOMTYPE_NORMAL, mapData.Row[nny][nnx]), (float)mapData.Row[ny][nx]);

			/// 部屋ずらす
			int adjNx = 0;
			int adjNy = 0;
			if (normalRoom->roomID != ROOMID_NO) {
				nnx = nx - (dungeonData->EachRoomWidth / 2 - normalRoom->roomWidth / 2 - 1);
				nny = ny - (dungeonData->EachRoomHeight / 2 - normalRoom->roomHeight / 2 - 1);

				int ex = nx + (dungeonData->EachRoomWidth / 2 - normalRoom->roomWidth / 2 - 1);
				int ey = ny + (dungeonData->EachRoomHeight / 2 - normalRoom->roomHeight / 2 - 1);

				int maxPerlinValue = 0;
				for (int y = nny; y < ey; y++) {
					for (int x = nnx; x < ex; x++) {
						int nowPerlinValue = mapData.Row[y][x];
						if (maxPerlinValue < nowPerlinValue) {
							maxPerlinValue = nowPerlinValue;
							adjNx = x;
							adjNy = y;
						}
					}
				}
			} else {
				adjNx = nx;
				adjNy = ny;
			}

			/// データ保存
			RoomData treasureRoomData = {
				nx,ny,
				adjNx,adjNy,
				ROOMTYPE_NORMAL,
				normalRoom->roomID,
				mapData.Row[ny][nx] };
			roomData.push_back(treasureRoomData);
			delete normalRoom;
		}
	}
}

void GBPDungeonGenerater::SpawnTestDungeon(TileMapData* targetMap, int times) {
	targetMap;
	int spawnCounter = times;
	int turnCounter = 0;

	dungeonData = new GBPTestDungeon;
	int seachDistance = dungeonData->dungeonSpawnDistance * 2;
	while (spawnCounter > 0) {
		for (int y = 0; y < maxRow_; y++) {
			for (int x = 0; x < maxCol_; x++) {
				if (mapData.Row[y][x] == dungeonData->StartPoint) {
					bool isSkip = false;
					for (int dy = -seachDistance; dy <= seachDistance; dy++) {
						for (int dx = -seachDistance; dx <= seachDistance; dx++) {
							int nx = x + dx;
							int ny = y + dy;
							if (nx < 0 || nx >= maxCol_ || ny < 0 || ny >= maxRow_)continue;
							if (dx * dx + dy * dy > seachDistance * seachDistance) continue;
							for (const auto& i : spawnPoints_) {
								if (nx == (int)i.x && ny == (int)i.y) {
									isSkip = true;
									break;
								}
							}
							if (isSkip)break;
						}
						if (isSkip)break;
					}
					if (isSkip)break;
					DungeonSpawner(targetMap, x, y);
					spawnCounter--;
					spawnPoints_.push_back(Vector2{ (float)x,(float)y });
				}
			}
		}
		turnCounter++;
		if (turnCounter > 100) {
			break;
		}
	}
	spawnPoints_.clear();
	turnCounter;
}

