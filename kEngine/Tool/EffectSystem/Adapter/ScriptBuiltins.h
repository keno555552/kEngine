#pragma once
#include <functional>
#include <string>
#include <vector>
#include "Tool/MapBulider/Adapter/ProgramAdapter.h"     // RuntimeContext

/// スクリプトから呼べる組み込み関数
///  - ここに 1 つ足せば、実行（文・式の両方）と ReadMe に自動で出る
struct BuiltinFunc {
	std::string name;                 // スクリプトでの名前
	int minArgs = 0;                  // 引数の数（足りなければエラー）
	int maxArgs = 0;
	std::string usage;                // ReadMe 用："rand(min, max)"
	std::string desc;                 // ReadMe 用：説明
	/// 引数は評価済みの値。戻り値は式で使われる時だけ意味がある（文なら捨てる）
	std::function<int(RuntimeContext& ctx, const std::vector<int>& args)> func;
};

class ScriptBuiltins {
public:
	/// 名前で探す（無ければ nullptr）
	static const BuiltinFunc* Find(const std::string& name);
	/// 全部（ReadMe 用、登録した順）
	static const std::vector<BuiltinFunc>& All();
};