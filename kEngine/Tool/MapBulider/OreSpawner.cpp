#include "OreSpawner.h"

void OreSpawner::SetOreSetting() {
}

void OreSpawner::SetOreSpawner(int maxRow, int maxCol,
	float attenuation, int threshold, int seed) {
	maxRow_ = maxRow;
	maxCol_ = maxCol;
	attenuation_ = attenuation;
	threshold_ = threshold;
	seed_ = seed;
}

void OreSpawner::SetOreSpawner(int maxRow, int maxCol) {
	maxRow_ = maxRow;
	maxCol_ = maxCol;
}

void OreSpawner::MakeFormMap(TileMapData* targetMap, TileMapData* refenceMap,  int stopper) {
	for (int i = 0; i < OreType::maxType; i++) {
		for (int y = oreSetting[i].startLevel; y <= maxRow_ && y < oreSetting[i].endLevel; y++) {
			for (int x = 0; x < maxCol_; x++) {
				if (refenceMap->Row[y][x] == oreSetting[i].startPoint) {
					if (targetMap->Row[y][x] != oreSetting[i].type) {
						if (targetMap->Row[y][x] != stopper) {
							bool isSkip = false;
							for (int dy = -oreSetting[i].minGap; dy <= oreSetting[i].minGap; dy++) {
								for (int dx = -oreSetting[i].minGap; dx <= oreSetting[i].minGap; dx++) {
									int nx = x + dx;
									int ny = y + dy;
									if (nx < 0 || nx >= maxCol_ || ny < 0 || ny >= maxRow_)continue;
									if (dx * dx + dy * dy > oreSetting[i].minGap * oreSetting[i].minGap) continue;
									if (targetMap->Row[ny][nx] == oreSetting[i].type) {
										isSkip = true;
										break;
									}
								}
								if (isSkip)break;
							}
							if (isSkip)break;
							runOreSpawner(oreSetting[i].radius, targetMap, x, y, *refenceMap, oreSetting[i].minVolume, oreSetting[i].maxVolume, oreSetting[i].type, stopper);
						}
					}
				}
			}
		}
	}
}

void OreSpawner::MakeOverWriteMap(TileMapData* targetMap, int stopper) {
	for (int i = 0; i < OreType::maxType; i++) {
		int seed = seed_ + oreSetting[i].seedPlus;
		perlinNoise->SetSeed(seed);
		TileMapData mapData;
		mapData.Row.resize(maxRow_);
		for (int j = 0; j < maxRow_; j++) {
			mapData.Row[j].resize(maxCol_);
		}
		for (int y = 0; y < maxRow_; y++) {
			for (int x = 0; x < maxCol_; x++) {
				mapData.Row[y][x] = int(perlinNoise->fBm(
					Vector3{ float(x),float(y),1.0f } *Vector3{ 0.056f, 0.145f, 0.1f },
					1, 1.0f, 1.0f, 1.0f) * 255);
			}
		}
		for (int y = oreSetting[i].startLevel; y <= maxRow_ && y < oreSetting[i].endLevel; y++) {
			for (int x = 0; x < maxCol_; x++) {
				if (mapData.Row[y][x] == oreSetting[i].startPoint) {
					if (targetMap->Row[y][x] != oreSetting[i].type) {
						if (targetMap->Row[y][x] != stopper) {
							bool isSkip = false;
							for (int dy = -oreSetting[i].minGap; dy <= oreSetting[i].minGap; dy++) {
								for (int dx = -oreSetting[i].minGap; dx <= oreSetting[i].minGap; dx++) {
									int nx = x + dx;
									int ny = y + dy;
									if (nx < 0 || nx >= maxCol_ || ny < 0 || ny >= maxRow_)continue;
									if (dx * dx + dy * dy > oreSetting[i].minGap * oreSetting[i].minGap) continue;
									if (targetMap->Row[ny][nx] == oreSetting[i].type) {
										isSkip = true;
										break;
									}
								}
								if (isSkip)break;
							}
							if (isSkip)break;
							runOreSpawner(oreSetting[i].radius, targetMap, x, y, mapData, oreSetting[i].minVolume, oreSetting[i].maxVolume, oreSetting[i].type, stopper);
						}
					}
				}
			}
		}

	}
}


void OreSpawner::digCircle(TileMapData* tileMap, int cx, int cy, float radius, int type, int stopper) {
	for (int y = (int)-radius; y <= (int)radius; y++) {
		for (int x = (int)-radius; x <= (int)radius; x++) {
			int nx = cx + x;
			int ny = cy + y;
			if (stopper != -500) {
				if (nx >= 0 && nx < maxCol_ && ny >= 0 && ny < maxRow_ &&
					std::sqrt(float(x * x + y * y)) <= radius && tileMap->Row[ny][nx] != stopper) {
					tileMap->Row[ny][nx] = type;
				}
			} else {
				if (nx >= 0 && nx < maxCol_ && ny >= 0 && ny < maxRow_ &&
					std::sqrt(float(x * x + y * y)) <= radius) {
					tileMap->Row[ny][nx] = type;
				}
			}
		}
	}
}

void OreSpawner::runOreSpawner(float radius, TileMapData* tileMap, int& startX, int& startY, TileMapData& refenceMap, int minVolume, int maxVolume, int type, int stopper) {
	int cx = startX;
	int cy = startY;
	int sx = startX;
	int sy = startY;
	int bx = startX;
	int by = startY;
	int updown = 0;
	int R = 1;
	float volume;
	if (sx + 2 < maxCol_ && sy + 2 < maxRow_) {
		volume = (float)minVolume + float(maxVolume - minVolume) * (refenceMap.Row[sy + 2][sx + 2] / 255.0f);
	} else if (sx - 2 >= 0 && sy - 2 >= 0) {
		volume = (float)minVolume + float(maxVolume - minVolume) * (refenceMap.Row[sy - 2][sx - 2] / 255.0f);
	} else {
		volume = (float)maxVolume; // or something safe
	}

	int timer = 0;
	digCircle(tileMap, sx, sy, radius, type, stopper);
	int handle = 1;
	while (handle < (int)volume) {
		timer++;
		sx = 0;
		sy = 0;
		if (!updown) {
			for (int y = (int)-R; y <= (int)R; y++) {
				for (int x = (int)-R; x <= (int)R; x++) {
					int nx = cx + x;
					int ny = cy + y;
					if (nx < 0 || ny < 0 || nx >= maxCol_ || ny >= maxRow_) continue;
					if (tileMap->Row[ny][nx] == type || tileMap->Row[ny][nx] == stopper) continue;
					if (std::sqrt(float(x * x + y * y)) > R)continue;
					if (refenceMap.Row[ny][nx] > refenceMap.Row[by][bx])continue;
					if (refenceMap.Row[ny][nx] > refenceMap.Row[sy][sx]) {
						sx = nx;
						sy = ny;
					}
				}
			}
			if (sx == 0 && sy == 0) {
				updown = 1;
				continue;
			}
		} else {
			for (int y = (int)-R; y <= (int)R; y++) {
				for (int x = (int)-R; x <= (int)R; x++) {
					int nx = cx + x;
					int ny = cy + y;
					if (nx < 0 || ny < 0 || nx >= maxCol_ || ny >= maxRow_) continue;
					if (tileMap->Row[ny][nx] == type || tileMap->Row[ny][nx] == stopper) continue;
					if (std::sqrt(float(x * x + y * y)) > R)continue;
					//if (refenceMap.Row[ny][nx] < refenceMap.Row[by][bx])continue;
					if (refenceMap.Row[ny][nx] < refenceMap.Row[sy][sx]) {
						sx = nx;
						sy = ny;
					}
				}
			}
		}
		if (refenceMap.Row[sy][sx] != 0) {
			digCircle(tileMap, sx, sy, radius, type, stopper);
			bx = sx;
			by = sy;
			handle++;
		}
		//if ((int)volume % 6 == 0) {
		//	R++;
		//}
		if (timer % 6 == 0) {
			R++;
		}
	}
}