#include "TileRunner.h"

void TileRunnerP::SetTileRunner(int maxRow, int maxCol,
	float attenuation, int step,
	int threshold, int seed) {
	maxRow_ = maxRow;
	maxCol_ = maxCol;
	attenuation_ = attenuation;
	step_ = step;
	threshold_ = threshold;
	seed_ = seed;
}

void TileRunnerP::SetTileRunner(int maxRow, int maxCol) {
	maxRow_ = maxRow;
	maxCol_ = maxCol;
}

void TileRunnerP::MakeFormMap(TileMapData* targetMap, TileMapData* refenceMap, int type,int startRadius, int stopper) {
	float radius = (float)startRadius;
	for (int y = 0; y < maxRow_; y++) {
		for (int x = 0; x < maxCol_; x++) {
			if (refenceMap->Row[y][x] == startPoint_) {
				bool isSkip = false;
				for (int dy = -30; dy <= 30; dy++) {
					for (int dx = -30; dx <= 30; dx++) {
						int nx = x + dx;
						int ny = y + dy;
						if (nx < 0 || nx >= maxCol_ || ny < 0 || ny >= maxRow_)continue;
						if (dx * dx + dy * dy > 30 * 30) continue;
						if (refenceMap->Row[ny][nx] == startPoint_ && targetMap->Row[ny][nx] == AIR) {
							isSkip = true;
							break;
						}
					}
					if (isSkip)break;
				}
				if (isSkip)break;
				runTileRunner(radius, targetMap, x, y, *refenceMap, 2, type,stopper);
			}
		}
	}
}

void TileRunnerP::MakeOverWriteMap(TileMapData* targetMap, int type, int startRadius, int stopper) {
	perlinNoise->SetSeed(seed_);
	TileMapData mapData;
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
	float radius = (float)startRadius;
	for (int y = 0; y < maxRow_; y++) {
		for (int x = 0; x < maxCol_; x++) {
			if (mapData.Row[y][x] == startPoint_) {
				bool isSkip = false;
				for (int dy = -30; dy <= 30; dy++) {
					for (int dx = -30; dx <= 30; dx++) {
						int nx = x + dx;
						int ny = y + dy;
						if (nx < 0 || nx >= maxCol_ || ny < 0 || ny >= maxRow_)continue;
						if (dx * dx + dy * dy > 30 * 30) continue;
						if (mapData.Row[ny][nx] == startPoint_ && targetMap->Row[ny][nx] == AIR) {
							isSkip = true;
							break;
						}
					}
					if (isSkip)break;
				}
				if (isSkip)break;
				runTileRunner(radius, targetMap, x, y, mapData, 2, type,stopper);
			}
		}
	}
}


void TileRunnerP::digCircle(TileMapData* tileMap, int cx, int cy, float radius, int type,int stopper) {
	for (int y = (int)-radius; y <= (int)radius; y++) {
		for (int x = (int)-radius; x <= (int)radius; x++) {
			int nx = cx + x;
			int ny = cy + y;
			if(stopper != -500){
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

float TileRunnerP::runTileRunner(float radius, TileMapData* tileMap, int& x, int& y, TileMapData& refenceMap, int splitTime, int type,int stopper) {
	float radiuss = radius;
	int px = x;
	int py = y;
	int sx = x;
	int sy = y;
	int cx = x;
	int cy = y;
	int dx[5] = { -1, -1,  0, 1, 1 };
	int dy[5] = { 0,  1,  1, 1, 0 };
	int velocity = 0;
	if (splitTime > 0) {
		for (int i = 0; i < step_ && radiuss > 0.5f; i++) {
			//if (sx < 0 || sy < 0 || sx >= maxCol_ || sy >= maxRow_) break;
			if (tileMap->Row[sy][sx] != 0) {
				if(stopper != -500){
					digCircle(tileMap, sx, sy, radiuss, type, stopper);
				} else {
					digCircle(tileMap, sx, sy, radiuss, type);
				}
			}
			int handle = 0;
			//if (i % int(radiuss * 2) == 0) {
			if (i % 10 == 0) {
				for (int j = 0; j < 5; ++j) {
					int nx = cx + dx[j];
					int ny = cy + dy[j];
					if (nx < 0 || ny < 0 || nx >= maxCol_ || ny >= maxRow_) continue;
					int handle2 = refenceMap.Row[ny][nx];
					if (handle2 > handle && handle2 != splitPoint_) {
						handle = handle2;
						sx = nx;
						sy = ny;
						velocity = j;
					}
				}
				if (handle == 0) {
					sx = cx + dx[2];
					sy = cy + dy[2];
					velocity = 2;
				}
			} else {
				sx = cx + dx[velocity];
				sy = cy + dy[velocity];
			}
			if (i % 2 == 0) {
				px = cx;
				py = cy;
			}
			if (sx == px && sy == py) {
				sx = cx + dx[2];
				sy = cy + dy[2];
				velocity = 2;
			}
			cx = sx;
			cy = sy;
			radiuss *= attenuation_;

			if (cx < 0 || cy < 0 || cx >= maxCol_ || cy >= maxRow_) break;
			if (refenceMap.Row[cy][cx] == splitPoint_) {
				int handle2 = 0;
				int ssx = cx;
				int ssy = cy;
				for (int j = 0; j < 5; ++j) {
					int nx = cx + dx[j];
					int ny = cy + dy[j];
					if (nx < 0 || ny < 0 || nx >= maxCol_ || ny >= maxRow_) continue;
					int handle3 = refenceMap.Row[ny][nx];
					if (handle3 > handle2 && handle3 != handle && handle3 != splitPoint_) {
						handle2 = handle3;
						ssx = nx;
						ssy = ny;
					}
				}
				if (ssx == px && ssy == py) {
					ssx = cx + dx[2];
					ssy = cy + dy[2];
				}
				if (ssx < 0 || ssy < 0 || ssx >= maxCol_ || ssy >= maxRow_) continue;
				runTileRunner(radiuss, tileMap, ssx, ssy, refenceMap, splitTime - 1,type,stopper);
				continue;
			}
		}
	}
	return radiuss;
}