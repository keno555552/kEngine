#include "ScriptBuiltins.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>
#include "Tool/MapBulider/TileRunner.h"
#include "Tool/MapBulider/OreSpawner.h"
#include "Tool/MapBulider/GBPDungeonGenerater.h"

namespace {
	/// ========================== 小道具 ========================== ///
	using Args = std::vector<ScriptValue>;
	ScriptValue Int(int v) { return ScriptValue::Int(v); }
	ScriptValue Float(float v) { return ScriptValue::Float(v); }

	/// 引数 i が無ければ def
	int ArgInt(const Args& a, size_t i, int def) { return i < a.size() ? a[i].AsInt() : def; }
	float ArgFloat(const Args& a, size_t i, float def) { return i < a.size() ? a[i].AsFloat() : def; }

	/// min〜max に収める（逆に書いても OK）
	int InRange(uint32_t h, int min, int max) {
		if (max < min) std::swap(min, max);
		return min + static_cast<int>(h % static_cast<uint32_t>(max - min + 1));
	}

	/// srand 用：(地図の seed, n) → 決まった値（呼ぶ順番に関係なし）
	uint32_t Hash(int seed, int n) {
		uint64_t z = static_cast<uint64_t>(static_cast<uint32_t>(seed)) * 0x9E3779B97F4A7C15ull ^ static_cast<uint32_t>(n);
		z += 0x9E3779B97F4A7C15ull;
		z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
		z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
		return static_cast<uint32_t>(z ^ (z >> 31));
	}

	/// rand 用：真隨機（起動時に 1 回だけ種を作る。time() は秒単位なので使わない）
	std::mt19937& TrueRandom() {
		static std::mt19937 engine{ std::random_device{}() };
		return engine;
	}

	/// 旧生成器（TileRunner / OreSpawner / GBPDungeon）を今の地図で動かす
	///  - 旧生成器は Type（AIR=1, ROCK=4 ...）で書くので、作業用の地図で動かして
	///    変わったマスだけ tileType() の対応表で ID に直して戻す
	///  - 作業用の地図：今の空気マスは AIR、それ以外は「触っていない」(-1)
	template<class F>
	void RunOldGenerator(RuntimeContext& ctx, F&& generate) {
		constexpr int kUntouched = -1;
		TileMapData& map = *ctx.map;
		const int h = map.Height();
		const int w = map.Width();
		const int airId = ctx.TypeToId(AIR);

		TileMapData work;
		work.Row.assign(h, std::vector<int>(w, kUntouched));
		for (int y = 0; y < h; ++y) for (int x = 0; x < w; ++x) {
			if (map.Row[y][x] == airId) work.Row[y][x] = AIR;
		}
		const TileMapData before = work;

		generate(work, w, h);

		for (int y = 0; y < h; ++y) for (int x = 0; x < w; ++x) {
			if (work.Row[y][x] != before.Row[y][x]) map.Row[y][x] = ctx.TypeToId(work.Row[y][x]);
		}
	}

	/// 名前付き設定：同じ名前の変数（ローカル → Variables）があればその値、無ければ def
	float Setting(RuntimeContext& ctx, const char* name, float def) {
		ScriptValue v;
		return MapProgramAdapter::ReadVariable(ctx, name, v) ? v.AsFloat() : def;
	}

	/// 乱数（地図 seed の列。同じ seed・同じ腳本なら同じ結果）
	float RandF(RuntimeContext& ctx, float min, float max) { return (max <= min) ? min : ctx.rng.randomFloat(min, max); }
	int RandI(RuntimeContext& ctx, int min, int max) { return (max <= min) ? min : ctx.rng.randomInt(min, max); }

	/// 円を塗る（roughness > 0 なら縁をギザギザに）
	void FillCircle(RuntimeContext& ctx, float cx, float cy, float r, int id, float roughness) {
		TileMapData& map = *ctx.map;
		const int reach = static_cast<int>(std::ceil(r + 1.0f));
		const int ix = static_cast<int>(std::floor(cx));
		const int iy = static_cast<int>(std::floor(cy));
		for (int y = iy - reach; y <= iy + reach; ++y) {
			for (int x = ix - reach; x <= ix + reach; ++x) {
				if (!map.InBounds(x, y)) continue;
				const float dx = x + 0.5f - cx, dy = y + 0.5f - cy;
				const float edge = r + (roughness > 0.0f ? RandF(ctx, -roughness, roughness) * r * 0.5f : 0.0f);
				if (dx * dx + dy * dy <= edge * edge) map.Row[y][x] = id;
			}
		}
	}

	/// 泰拉式トンネル：浮動小数の位置・向きで歩きながら円を掘る
	struct TunnelStyle {
		int turnEvery;   // 何歩ごとに向きを変えるか
		float turnAngle; // 1 回の最大の曲がり（度）
		int branchMin;   // 枝の数
		int branchMax;
		float roughness; // 縁の粗さ（0 = 滑らか）
		float endRatio;  // 最後の太さ（1 = 細くならない）
	};

	void Walk(RuntimeContext& ctx, float x, float y, int id, float radius, int length, float angleDeg, const TunnelStyle& st, int depth) {
		constexpr float kDeg = 3.14159265f / 180.0f;
		const int w = ctx.map->Width(), h = ctx.map->Height();

		/// 枝を出す歩数（主線だけ。枝からは出さない）
		std::vector<int> branchAt;
		if (depth == 0 && length >= 10) {
			const int count = RandI(ctx, st.branchMin, st.branchMax);
			for (int i = 0; i < count; ++i) branchAt.push_back(RandI(ctx, length / 5, length * 4 / 5));
		}

		for (int step = 0; step < length; ++step) {
			const float t = static_cast<float>(step) / static_cast<float>(length);
			const float r = radius * (1.0f + (st.endRatio - 1.0f) * t);   // だんだん細く
			FillCircle(ctx, x, y, r, id, st.roughness);

			/// 枝
			for (int at : branchAt) {
				if (at != step) continue;
				const float side = (RandI(ctx, 0, 1) == 0) ? -1.0f : 1.0f;
				Walk(ctx, x, y, id, r * 0.7f, length / 2, angleDeg + side * RandF(ctx, 30.0f, 90.0f), st, depth + 1);
			}

			/// 曲がる
			if (st.turnEvery > 0 && step > 0 && step % st.turnEvery == 0) angleDeg += RandF(ctx, -st.turnAngle, st.turnAngle);
			x += std::cos(angleDeg * kDeg);
			y += std::sin(angleDeg * kDeg);

			/// 地図から大きく出たら終わり
			if (x < -r || y < -r || x > w + r || y > h + r) break;
		}
	}

	/// ========================== 組み込み関数の一覧 ========================== ///
	/// 新しい関数はここに 1 つ足すだけ
	std::vector<BuiltinFunc> CreateBuiltins() {
		return {
			/// ---------- 出力 ----------
			{ "setTile", 3, 3, "setTile(x, y, v)", "save v to the map at (x, y). ignored outside the map (= result[y][x] = v)",
				[](RuntimeContext& ctx, const Args& a) {
					const int x = a[0].AsInt(), y = a[1].AsInt();
					if (ctx.map->InBounds(x, y)) ctx.map->Row[y][x] = a[2].AsInt();
					return Int(0);
				} },

			/// ---------- 読み取り ----------
			{ "getTile", 2, 2, "getTile(x, y)", "value at (x, y). blank outside the map (= result[y][x])",
				[](RuntimeContext& ctx, const Args& a) {
					const int x = a[0].AsInt(), y = a[1].AsInt();
					return Int(ctx.map->InBounds(x, y) ? ctx.map->Row[y][x] : ctx.blankId);
				} },

			/// ---------- 乱数 ----------
			{ "rand", 0, 2, "rand() / rand(n) / rand(min, max)", "true random (different every call): 0-99 / 0 to n-1 / min to max",
				[](RuntimeContext&, const Args& a) {
					std::uniform_int_distribution<uint32_t> dist;
					const uint32_t h = dist(TrueRandom());
					if (a.size() >= 2) return Int(InRange(h, a[0].AsInt(), a[1].AsInt()));
					if (a.size() == 1) return Int(InRange(h, 0, (std::max)(0, a[0].AsInt() - 1)));
					return Int(InRange(h, 0, 99));
				} },
			{ "srand", 1, 3, "srand(n) / srand(n, min, max)", "seeded: same map seed + same n = same value (0-99 / min to max). e.g. srand(x * 1000 + y, 0, 9)",
				[](RuntimeContext& ctx, const Args& a) {
					const uint32_t h = Hash(ctx.seed, a[0].AsInt());
					if (a.size() >= 3) return Int(InRange(h, a[1].AsInt(), a[2].AsInt()));
					return Int(InRange(h, 0, 99));
				} },

			/// ---------- ノイズ ----------
			{ "noise", 2, 5, "noise(x, y [, scale, octaves, persistence])", "Perlin noise with the map seed -> float 0.0 to 1.0. default scale 0.1 / octaves 1 / persistence 0.5",
				[](RuntimeContext& ctx, const Args& a) {
					const float scale = ArgFloat(a, 2, 0.1f);
					const int octaves = (std::max)(1, ArgInt(a, 3, 1));
					const float persistence = ArgFloat(a, 4, 0.5f);
					float total = 0.0f, maxAmp = 0.0f, amp = 1.0f, freq = 1.0f;
					for (int o = 0; o < octaves; ++o) {
						/// 旧 CreateEarth と同じ：(x, y, 1) * scale
						const Vector3 pos{ a[0].AsFloat() * scale * freq, a[1].AsFloat() * scale * freq, scale * freq };
						total += ctx.perlin.perlin(pos) * amp;
						maxAmp += amp;
						amp *= persistence;
						freq *= 2.0f;
					}
					return Float(total / maxAmp);
				} },

			/// ---------- 形 ----------
			{ "circle", 4, 4, "circle(x, y, r, id)", "fill a circle of radius r at (x, y) with id (float OK)",
				[](RuntimeContext& ctx, const Args& a) {
					FillCircle(ctx, a[0].AsFloat() + 0.5f, a[1].AsFloat() + 0.5f, a[2].AsFloat(), a[3].AsInt(), 0.0f);
					return Int(0);
				} },
			{ "tunnel", 5, 5, "tunnel(x, y, id, radius, length)",
				"dig a cave (Terraria-like). settings = variables with the same name: turnEvery 6 / turnAngle 30 / branchMin 0 / branchMax 2 / roughness 0.3 / endRatio 0.4 / dir -1 (degrees, 0 = right, 90 = down, -1 = random)",
				[](RuntimeContext& ctx, const Args& a) {
					TunnelStyle st;
					st.turnEvery = static_cast<int>(Setting(ctx, "turnEvery", 6.0f));
					st.turnAngle = Setting(ctx, "turnAngle", 30.0f);
					st.branchMin = static_cast<int>(Setting(ctx, "branchMin", 0.0f));
					st.branchMax = (std::max)(st.branchMin, static_cast<int>(Setting(ctx, "branchMax", 2.0f)));
					st.roughness = (std::max)(0.0f, Setting(ctx, "roughness", 0.3f));
					st.endRatio = (std::max)(0.0f, Setting(ctx, "endRatio", 0.4f));
					float dir = Setting(ctx, "dir", -1.0f);
					if (dir < 0.0f) dir = RandF(ctx, 0.0f, 360.0f);

					const int length = (std::min)(a[4].AsInt(), 100000);   // 念のため上限
					Walk(ctx, a[0].AsFloat() + 0.5f, a[1].AsFloat() + 0.5f, a[2].AsInt(), a[3].AsFloat(), length, dir, st, 0);
					return Int(0);
				} },

			/// ---------- 旧生成器 ----------
			{ "tileType", 2, 2, "tileType(type, id)", "old generator type -> id of this map (same number if not set). 1 AIR 2 GRASS 3 EARTH 4 ROCK 5 IRON 6 GOLD 7 DIAMOND 8 SPAWNER 9 DOOR",
				[](RuntimeContext& ctx, const Args& a) {
					ctx.tileTypes[a[0].AsInt()] = a[1].AsInt();
					return Int(0);
				} },
			{ "surface", 2, 2, "surface(from, to)", "in each column, the first 'from' from the top becomes 'to' (grass)",
				[](RuntimeContext& ctx, const Args& a) {
					const int from = a[0].AsInt(), to = a[1].AsInt();
					TileMapData& map = *ctx.map;
					for (int x = 0; x < map.Width(); ++x) {
						for (int y = 0; y < map.Height(); ++y) {
							if (map.Row[y][x] == from) { map.Row[y][x] = to; break; }
						}
					}
					return Int(0);
				} },
			{ "burrow", 0, 5, "burrow([radius, startPoint, splitPoint, attenuation, step])", "old TileRunner caves, dug with AIR. default 5 / 138 / 173 / 0.99 / 100",
				[](RuntimeContext& ctx, const Args& a) {
					RunOldGenerator(ctx, [&](TileMapData& work, int w, int h) {
						TileRunnerP runner;
						runner.SetTileRunner(h, w, ArgFloat(a, 3, 0.99f), ArgInt(a, 4, 100), 90, ctx.seed);
						runner.SetStartPoint(ArgInt(a, 1, 138));
						runner.SetSplitPoint(ArgInt(a, 2, 173));
						runner.MakeOverWriteMap(&work, AIR, ArgInt(a, 0, 5));
					});
					return Int(0);
				} },
			{ "ore", 0, 0, "ore()", "old OreSpawner: IRON / GOLD / DIAMOND, never placed in air",
				[](RuntimeContext& ctx, const Args&) {
					RunOldGenerator(ctx, [&](TileMapData& work, int w, int h) {
						OreSpawner spawner;
						spawner.SetOreSpawner(h, w, 0.99f, 90, ctx.seed);
						spawner.MakeOverWriteMap(&work, AIR);
					});
					return Int(0);
				} },
			{ "dungeon", 0, 1, "dungeon([times])", "old GBPDungeon rooms. map must be at least 120 x 40",
				[](RuntimeContext& ctx, const Args& a) {
				/// ダンジョンの最大の大きさより小さい地図ははみ出して落ちるので止める
				const GBPTestDungeon size;
				const int needW = size.MaxWidth * size.EachRoomWidth;
				const int needH = size.MaxHeight * size.EachRoomHeight;
				if (ctx.map->Width() < needW || ctx.map->Height() < needH) {
					ctx.errors.push_back("dungeon: map must be at least " + std::to_string(needW) + " x " + std::to_string(needH));
					return Int(0);
				}
				RunOldGenerator(ctx, [&](TileMapData& work, int w, int h) {
					GBPDungeonGenerater generater;
					generater.SetDungeonGenerater(h, w, ctx.seed);
					generater.SpawnTestDungeon(&work, (std::max)(1, ArgInt(a, 0, 1)));
				});
				return Int(0);
			} },
		};
	}
}

const std::vector<BuiltinFunc>& ScriptBuiltins::All() {
	static const std::vector<BuiltinFunc> list = CreateBuiltins();   // 最初に使われた時に 1 回だけ作る
	return list;
}

const BuiltinFunc* ScriptBuiltins::Find(const std::string& name) {
	for (const auto& f : All()) {
		if (f.name == name) return &f;
	}
	return nullptr;
}