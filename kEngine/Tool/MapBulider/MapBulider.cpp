#include "MapBulider.h"

MapBulider::~MapBulider() {
	delete perlinNoise, perlinNoise = nullptr;
	delete oreSpawner, oreSpawner = nullptr;
}

void MapBulider::SetSeed(int seed) {
	seed_ = seed;
}

void MapBulider::SetMap(int maxRow, int maxCol,
	int earthLevel, int earthRange,
	int rockLevel, int rockRange) {
	maxRow_ = maxRow;
	maxCol_ = maxCol;
	earthLevel_ = earthLevel;
	earthRange_ = earthRange;
	rockLevel_ = rockLevel;
	rockRange_ = rockRange;

}

void MapBulider::CreateEarth(TileMapData* tileMap) {
	for (int y = 0; y < maxRow_; y++) {
		for (int x = 0; x < maxCol_; x++) {
			if (y > int(perlinNoise->fBm(Vector3{ float(x),float(y),1.0f } *Vector3{ 0.1f, 0.1f, 0.1f }, 1, 1.0f, 1.0f, 1.0f) * earthRange_) + earthLevel_) {
				tileMap->Row[y][x] = EARTH;
			}

		}
	}
}

void MapBulider::CreateRock(TileMapData* tileMap) {
	for (int y = 0; y < maxRow_; y++) {
		for (int x = 0; x < maxCol_; x++) {
			if (y > int(perlinNoise->fBm(Vector3{ float(x),float(y),1.0f } *Vector3{ 0.1f, 0.1f, 0.1f }, 1, 1.0f, 1.0f, 1.0f) * rockRange_) + rockLevel_) {
				tileMap->Row[y][x] = ROCK;
			}

		}
	}
}

void MapBulider::CreateGrass(TileMapData* tileMap) {
	for (int x = 0; x < maxCol_; x++) {
		for (int y = 0; y < maxRow_; y++) {
			if (tileMap->Row[y][x] == EARTH) {
				tileMap->Row[y][x] = GRASS;
				break;
			}
		}
	}
}

void MapBulider::CreateBurrow(TileMapData* tileMap) {
	tileRunner->SetTileRunner(maxRow_, maxCol_,
		attenuation_, step_, threshold_, seed_);
	tileRunner->SetStartPoint(burrowStartPoint_);
	tileRunner->SetSplitPoint(burrowSplitPoint_);
	tileRunner->MakeOverWriteMap(tileMap,AIR,5);
}

void MapBulider::CreateOre(TileMapData* tileMap) {
	oreSpawner->SetOreSpawner(maxRow_, maxCol_,
		attenuation_, threshold_, seed_);
	oreSpawner->MakeOverWriteMap(tileMap, AIR);
}

void MapBulider::CreateGBPDungeon(TileMapData* tileMap) {
	gbpDungeonGenerater->SetDungeonGenerater(maxRow_, maxCol_, seed_);
	gbpDungeonGenerater->SpawnTestDungeon(tileMap, 1);
}

void MapBulider::CreateMap(TileMapData* tileMap) {
	tileMap;
	///初期化
	perlinNoise->SetRepeat(0);
	for (int y = 0; y < maxRow_; y++) {
		for (int x = 0; x < maxCol_; x++) {
			tileMap->Row[y][x] = AIR;
		}
	}
	if (seed_ == NULL) {
		srand((unsigned int)time(NULL));
		seed_ = rand() % 244;
	}
	perlinNoise->SetSeed(seed_);
	/// 実作
	CreateEarth(tileMap);
	CreateRock(tileMap);
	CreateBurrow(tileMap);
	CreateGrass(tileMap);
	CreateOre(tileMap);
	CreateGBPDungeon(tileMap);
}


