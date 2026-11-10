#include "ScriptBuiltins.h"
#include <algorithm>

namespace {
	/// ========================== 組み込み関数の一覧 ========================== ///
	/// 新しい関数はここに 1 つ足すだけ
	std::vector<BuiltinFunc> CreateBuiltins() {
		return {
			/// ---------- 出力 ----------
			{ "setTile", 3, 3, "setTile(x, y, v)", "save v to the map at (x, y)  ※範囲外は無視（= result[y][x] = v）",
				[](RuntimeContext& ctx, const std::vector<int>& a) {
					if (ctx.map->InBounds(a[0], a[1])) ctx.map->Row[a[1]][a[0]] = a[2];
					return 0;
				} },

			/// ---------- 読み取り ----------
			{ "getTile", 2, 2, "getTile(x, y)", "value at (x, y)  ※範囲外は blank（= result[y][x]）",
				[](RuntimeContext& ctx, const std::vector<int>& a) {
					return ctx.map->InBounds(a[0], a[1]) ? ctx.map->Row[a[1]][a[0]] : ctx.blankId;
				} },

			/// ---------- 乱数（seed で固定） ----------
			{ "rand", 0, 2, "rand() / rand(n) / rand(min, max)", "0~99 / 0~n-1 / min~max",
				[](RuntimeContext& ctx, const std::vector<int>& a) {
					if (a.size() >= 2) return ctx.rng.randomInt(a[0], a[1]);
					if (a.size() == 1) return ctx.rng.randomInt(0, (std::max)(0, a[0] - 1));
					return ctx.rng.randomInt(0, 99);
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