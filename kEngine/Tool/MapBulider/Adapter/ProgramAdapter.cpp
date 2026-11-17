#include "ProgramAdapter.h"
#include <algorithm>
#include <cmath>
#include <charconv>
#include "Tool/EffectSystem/Adapter/ScriptBuiltins.h"

/// ==================== 実行用の小道具（このファイルだけで使う） ==================== ///
namespace {
	/// 同じエラーは 1 回だけ記録
	void AddError(RuntimeContext& ctx, const std::string& msg) {
		if (std::find(ctx.errors.begin(), ctx.errors.end(), msg) == ctx.errors.end()) ctx.errors.push_back(msg);
	}

	/// "pos.y" → name = "pos", member = "y"（ドットが無ければ member は空）
	void SplitMember(const std::string& full, std::string& name, std::string& member) {
		const size_t dot = full.find('.');
		name = full.substr(0, dot);
		member = (dot == std::string::npos) ? std::string() : full.substr(dot + 1);
	}

	/// ベクトルの成分を選ぶ（成分指定なし → x、知らない成分 → nullptr）
	float* PickMember(const std::string& m, float* x, float* y, float* z = nullptr, float* w = nullptr) {
		if (m.empty() || m == "x") return x;
		if (m == "y") return y;
		if (m == "z") return z;
		if (m == "w") return w;
		return nullptr;
	}

	/// "12" → int / "1.5" や "2.0f" → float / true,false → 1,0
	ScriptValue ParseLiteral(std::string s) {
		if (s == "true")  return ScriptValue::Int(1);
		if (s == "false") return ScriptValue::Int(0);
		if (s.find('.') != std::string::npos) {
			if (!s.empty() && (s.back() == 'f' || s.back() == 'F')) s.pop_back();
			float v = 0.0f;
			std::from_chars(s.data(), s.data() + s.size(), v);   // 数字でなければ 0（例外を投げない）
			return ScriptValue::Float(v);
		}
		int v = 0;
		std::from_chars(s.data(), s.data() + s.size(), v);
		return ScriptValue::Int(v);
	}

	/// GeneratorVariable を読む（無ければ false）
	bool ReadGenVar(GeneratorVariable* vars, const std::string& fullName, ScriptValue& out) {
		if (!vars) return false;
		std::string name, member;
		SplitMember(fullName, name, member);
		Variable* v = vars->FindVariable(name);
		if (!v) return false;

		switch (v->type) {
		case VarType::Int:    out = ScriptValue::Int(GetValue<int>(*v)); return true;
		case VarType::Float:  out = ScriptValue::Float(GetValue<float>(*v)); return true;
		case VarType::Double: out = ScriptValue::Float(static_cast<float>(GetValue<double>(*v))); return true;
		case VarType::Bool:   out = ScriptValue::Int(GetValue<bool>(*v) ? 1 : 0); return true;
		case VarType::Vector2: { Vector2 a = GetValue<Vector2>(*v); float* p = PickMember(member, &a.x, &a.y);             if (!p) return false; out = ScriptValue::Float(*p); return true; }
		case VarType::Vector3: { Vector3 a = GetValue<Vector3>(*v); float* p = PickMember(member, &a.x, &a.y, &a.z);       if (!p) return false; out = ScriptValue::Float(*p); return true; }
		case VarType::Vector4: { Vector4 a = GetValue<Vector4>(*v); float* p = PickMember(member, &a.x, &a.y, &a.z, &a.w); if (!p) return false; out = ScriptValue::Float(*p); return true; }
		default: return false;   // String は数値にできない
		}
	}

	/// GeneratorVariable に書く（コピーに書くので元は壊れない）。型はその変数のまま
	bool WriteGenVar(GeneratorVariable* vars, const std::string& fullName, const ScriptValue& value) {
		if (!vars) return false;
		std::string name, member;
		SplitMember(fullName, name, member);
		Variable* v = vars->FindVariable(name);
		if (!v) return false;

		float* p = nullptr;
		switch (v->type) {
		case VarType::Int:    v->value = value.AsInt(); return true;
		case VarType::Float:  v->value = value.AsFloat(); return true;
		case VarType::Double: v->value = static_cast<double>(value.AsFloat()); return true;
		case VarType::Bool:   v->value = value.AsBool(); return true;
		case VarType::Vector2: { auto& a = GetValueRef<Vector2>(*v); p = PickMember(member, &a.x, &a.y); break; }
		case VarType::Vector3: { auto& a = GetValueRef<Vector3>(*v); p = PickMember(member, &a.x, &a.y, &a.z); break; }
		case VarType::Vector4: { auto& a = GetValueRef<Vector4>(*v); p = PickMember(member, &a.x, &a.y, &a.z, &a.w); break; }
		default: return false;
		}
		if (!p) return false;
		*p = value.AsFloat();
		return true;
	}
}

std::string MapProgramAdapter::LoadCompile(const std::string& filePath) {
	// 讀取 JSON 檔案
	std::ifstream in(filePath);
	if (!in.is_open()) {
		return "{}"; // 檔案不存在
	}

	nlohmann::json file;
	in >> file;

	// 取出 nodes
	if (!file.contains("nodes")) {
		return "{}"; // 沒有 nodes
	}

	nlohmann::json nodesJson = file["nodes"];

	// 這裡你可以選擇：
	// 1. 直接回傳 JSON 字串（最簡單）
	// 2. 重建 Node 結構（之後 Blender 也會用）

	// 目前先回傳 JSON 字串（你最需要的）
	return nodesJson.dump(4);
}


void MapProgramAdapter::SaveCompile(const std::string& code,
	const std::string& filePath,
	const std::string& name) {

	std::string path = filePath + name + ".json";

	if (path.ends_with(".json.json")) {
		path = path.substr(0, path.length() - 5);
	} else if (!path.ends_with(".json")) {
		path += ".json";
	}

	nlohmann::json file;

	file["name"] = name;
	file["code"] = code;

	// 1. Tokenize
	auto tokens = Tokenize(code);
	file["tokens"] = tokens;

	// 2. SentenceBuilder
	auto sentences = BuildSentences(tokens);
	file["sentences"] = sentences;

	// 3. Parser → AST
	auto nodes = ParseProgram(sentences, nullptr);

	// 4. AST → JSON
	file["nodes"] = BodyToJson(nodes);

	file["debug_tokens"] = tokens;
	file["debug_sentences"] = sentences;


	// 寫入檔案
	std::ofstream out(path);
	out << file.dump(4);
}

TileMapData MapProgramAdapter::LoadMapBulider(const std::string& filePath) {
	return TileMapData();
}

TileMapData MapProgramAdapter::RunMapBulider(const std::string& filePath, GeneratorVariable* generatorVariables_,
	int blankId, int seed) {
	std::ifstream in(filePath);
	if (!in.is_open()) {
		std::cout << "MapBuilder file not found: " << filePath << std::endl;
		return TileMapData();
	}
	nlohmann::json file = nlohmann::json::parse(in, nullptr, false);
	if (file.is_discarded()) {
		std::cout << "MapBuilder file broken: " << filePath << std::endl;
		return TileMapData();
	}
	return GenerateFromJson(file, blankId, seed, generatorVariables_);
}

void MapProgramAdapter::ExecuteNode(Node* node, RuntimeContext& ctx) {
	if (node->type == "call") {
		auto* cn = static_cast<CallNode*>(node);
		CallBuiltin(cn->func, cn->args, ctx);          // 戻り値は使わない

	} else if (node->type == "assign") {
		auto* an = static_cast<AssignNode*>(node);
		const ScriptValue value = EvaluateExpression(an->expr, ctx);

		/// 1. 宣言（int x = ... / float f = ...）→ ローカルを作る（同名の GeneratorVariable より優先）
		if (!an->declType.empty()) {
			ctx.locals[an->varName] = value.As(an->declType == "float");
		}
		/// 2. 宣言済みのローカル → その型のまま
		else if (auto it = ctx.locals.find(an->varName); it != ctx.locals.end()) {
			it->second = value.As(it->second.isFloat);
		}
		/// 3. GeneratorVariable（"pos.x" も）→ その型のまま
		else if (WriteGenVar(ctx.genVars, an->varName, value)) {
		}
		/// 4. どこにも無い
		else {
			AddError(ctx, "undeclared variable: " + an->varName + "  (declare with int / float)");
		}

	} else if (node->type == "for") {
		auto* fn = static_cast<ForNode*>(node);
		const std::string var = fn->loopVar.empty() ? std::string("i") : fn->loopVar;   // 古い AST は i
		const bool isFloat = (fn->loopType == "float");

		/// for のカウンタはここで宣言扱い（int を書かなくても OK）
		ctx.locals[var] = EvaluateExpression(fn->initExpr, ctx).As(isFloat);

		/// 無限ループ防止
		constexpr int kMaxLoop = 1000000;
		for (int count = 0; EvaluateExpression(fn->condExpr, ctx).AsBool(); ++count) {
			if (count >= kMaxLoop) {
				AddError(ctx, "loop limit reached (" + std::to_string(kMaxLoop) + "): for " + var);
				break;
			}
			for (auto* child : fn->body) ExecuteNode(child, ctx);
			ctx.locals[var] = EvaluateExpression(fn->incExpr, ctx).As(isFloat);
		}

	} else if (node->type == "if") {
		auto* inNode = static_cast<IfNode*>(node);
		const auto& body = EvaluateExpression(inNode->condExpr, ctx).AsBool() ? inNode->thenBody : inNode->elseBody;
		for (auto* child : body) ExecuteNode(child, ctx);
	}
}

TileMapData MapProgramAdapter::Run(const std::vector<Node*>& nodes, int width, int height, int seed,
	int blankId, GeneratorVariable* vars, std::vector<std::string>* outErrors) {

	TileMapData map;
	map.Resize(width, height, blankId);

	RuntimeContext ctx;
	ctx.width = width;
	ctx.height = height;
	ctx.blankId = blankId;
	ctx.map = &map;
	ctx.genVars = vars;
	ctx.SetSeed(seed);

	for (auto* n : nodes) ExecuteNode(n, ctx);
	if (outErrors) *outErrors = ctx.errors;   // エラーを返す
	return map;
}

GeneratorVariable MapProgramAdapter::JsonToVariables(const nlohmann::json& file) {
	GeneratorVariable out;
	for (const auto& group : file.value("variables", nlohmann::json::array())) {
		for (const auto& jv : group.value("units", nlohmann::json::array())) {
			const std::string name = jv.value("name", std::string());
			const nlohmann::json& v = jv.contains("value") ? jv["value"] : nlohmann::json();
			VariantValue value;
			switch (StringToVarType(jv.value("type", std::string("Int")))) {
			case VarType::Bool:    value = v.is_boolean() ? v.get<bool>() : false; break;
			case VarType::Int:     value = v.is_number() ? v.get<int>() : 0; break;
			case VarType::Float:   value = v.is_number() ? v.get<float>() : 0.0f; break;
			case VarType::Double:  value = v.is_number() ? v.get<double>() : 0.0; break;
			case VarType::String:  value = v.is_string() ? v.get<std::string>() : std::string(); break;
			case VarType::Vector2: value = v.size() >= 2 ? Vector2{ v[0].get<float>(), v[1].get<float>() } : Vector2{}; break;
			case VarType::Vector3: value = v.size() >= 3 ? Vector3{ v[0].get<float>(), v[1].get<float>(), v[2].get<float>() } : Vector3{}; break;
			case VarType::Vector4: value = v.size() >= 4 ? Vector4{ v[0].get<float>(), v[1].get<float>(), v[2].get<float>(), v[3].get<float>() } : Vector4{}; break;
			}
			Variable var = CreateVariable(name, value);
			out.variableList.push_back(var);
		}
	}
	return out;
}

nlohmann::json MapProgramAdapter::VariablesToJson(const GeneratorVariable& vars) {
	nlohmann::json unit;
	unit["unitName"] = "Variables";
	unit["units"] = nlohmann::json::array();
	for (const auto& v : vars.variableList) {
		nlohmann::json jv;
		jv["name"] = v.name;
		jv["type"] = VarTypeToString(v.type);
		switch (v.type) {
		case VarType::Bool:    jv["value"] = std::get<bool>(v.value); break;
		case VarType::Int:     jv["value"] = std::get<int>(v.value); break;
		case VarType::Float:   jv["value"] = std::get<float>(v.value); break;
		case VarType::Double:  jv["value"] = std::get<double>(v.value); break;
		case VarType::String:  jv["value"] = std::get<std::string>(v.value); break;
		case VarType::Vector2: { const auto& a = std::get<Vector2>(v.value); jv["value"] = { a.x, a.y }; break; }
		case VarType::Vector3: { const auto& a = std::get<Vector3>(v.value); jv["value"] = { a.x, a.y, a.z }; break; }
		case VarType::Vector4: { const auto& a = std::get<Vector4>(v.value); jv["value"] = { a.x, a.y, a.z, a.w }; break; }
		}
		unit["units"].push_back(jv);
	}
	/// JsonToVariables と同じ形：[ { unitName, units:[...] } ]
	return nlohmann::json::array({ unit });
}

bool MapProgramAdapter::ReadVariable(RuntimeContext& ctx, const std::string& name, ScriptValue& out) {
	if (auto it = ctx.locals.find(name); it != ctx.locals.end()) { out = it->second; return true; }
	return ReadGenVar(ctx.genVars, name, out);
}

ScriptValue MapProgramAdapter::CallBuiltin(const std::string& name, const std::vector<ExpressionNode*>& args, RuntimeContext& ctx) {
	const BuiltinFunc* f = ScriptBuiltins::Find(name);
	/// rand() / burrow() のような空の () は「引数 0 個」（parser は空の引数を 1 つ作るので外す）
	std::vector<ExpressionNode*> used;
	for (auto* a : args) {
		const bool isEmpty = a && a->isVariable && a->varName.empty();
		if (a && !isEmpty) used.push_back(a);
	}
	const int count = static_cast<int>(used.size());

	/// 見つからない / 引数の数が違う → エラーを 1 回だけ記録して 0
	if (!f) { AddError(ctx, "unknown function: " + name); return ScriptValue::Int(0); }
	if (count < f->minArgs || count > f->maxArgs) { AddError(ctx, "wrong argument count: " + f->usage); return ScriptValue::Int(0); }

	/// 引数を先に全部評価してから呼ぶ（int / float のまま渡す）
	std::vector<ScriptValue> values;
	values.reserve(used.size());
	for (auto* a : used) values.push_back(EvaluateExpression(a, ctx));
	return f->func(ctx, values);
}

TileMapData MapProgramAdapter::GenerateFromJson(const nlohmann::json& file, int blankId, int seed, const GeneratorVariable* vars, std::vector<std::string>* outErrors) {
	/// 生成設定（MapMaker::BuildJson と同じキー、古いファイルは maxCol / maxRow）
	const nlohmann::json ctx = file.value("context", nlohmann::json::object());
	const int width = ctx.value("width", ctx.value("maxCol", 32));
	const int height = ctx.value("height", ctx.value("maxRow", 32));
	const int useSeed = (seed >= 0) ? seed : ctx.value("seed", 0);

	/// 変数：指定があればそれ、無ければファイルの値（どちらもコピーで実行）
	GeneratorVariable runVars = vars ? *vars : JsonToVariables(file);

	/// JSON → AST（函數ごと。OFF の函數は飛ばす。古い形式は "script" 1 つ）
	std::vector<std::vector<Node*>> layers;
	std::vector<std::string> layerNames;
	if (file.contains("functions")) {
		for (const auto& f : file["functions"]) {
			if (!f.value("enabled", true)) continue;
			layers.push_back(LoadAstFromJson(f.value("script", nlohmann::json::array())));
			layerNames.push_back(f.value("name", std::string("Function")));
		}
	} else {
		layers.push_back(LoadAstFromJson(file.value("script", nlohmann::json::array())));
		layerNames.push_back("Main");
	}

	TileMapData map = GenerateUnit(layers, layerNames, width, height, useSeed, blankId, &runVars, outErrors);
	map.name = file.value("name", std::string("UnnamedMap"));

	for (auto& layer : layers) for (auto* n : layer) delete n;
	return map;
}

TileMapData MapProgramAdapter::GenerateUnit(
	const std::vector<std::vector<Node*>>& layers,
	const std::vector<std::string>& layerNames,
	int width,
	int height,
	int seed,
	int blankId,
	GeneratorVariable* vars,
	std::vector<std::string>* outErrors,
	const std::vector<GeneratorVariable*>* layerVars) {

	TileMapData map;
	map.Resize(width, height, blankId);

	RuntimeContext ctx;
	ctx.width = width;
	ctx.height = height;
	ctx.blankId = blankId;
	ctx.map = &map;
	ctx.genVars = vars;
	ctx.SetSeed(seed);

	/// 函數（層）を上から順に、同じマップへ重ねる
	for (size_t i = 0; i < layers.size(); ++i) {
		ctx.locals.clear();                         // int x などは函數ごと（マップ・変数・乱数は引き継ぐ）
		/// その層の変数（函數ごとに違う Variables を使う時）
		ctx.genVars = (layerVars && i < layerVars->size() && (*layerVars)[i]) ? (*layerVars)[i] : vars;
		const size_t before = ctx.errors.size();
		for (auto* n : layers[i]) ExecuteNode(n, ctx);

		/// どの函數のエラーか分かるように名前を付ける
		const std::string tag = "[" + (i < layerNames.size() ? layerNames[i] : std::to_string(i + 1)) + "] ";
		for (size_t e = before; e < ctx.errors.size(); ++e) ctx.errors[e] = tag + ctx.errors[e];
	}

	if (outErrors) *outErrors = ctx.errors;
	return map;
}

nlohmann::json MapProgramAdapter::ExpressionToJson(ExpressionNode* expr) {
	nlohmann::json j; 
	if (!expr) return j;

	if (expr->isLiteral) {
		j["type"] = "literal";
		j["value"] = expr->literal;
	} else if (expr->isVariable) {
		j["type"] = "variable";
		j["name"] = expr->varName;
	} else if (expr->isBinary) {
		j["type"] = "binary";
		j["op"] = expr->op;
		j["left"] = ExpressionToJson(expr->left);
		j["right"] = ExpressionToJson(expr->right);
	} else if (expr->isFuncCall) {
		j["type"] = "funcCall";
		j["func"] = expr->funcName;

		j["args"] = nlohmann::json::array();
		for (auto* a : expr->funcArgs)
			j["args"].push_back(ExpressionToJson(a));
	}

	return j;
}

nlohmann::json MapProgramAdapter::BodyToJson(const std::vector<Node*>& body) {
	nlohmann::json arr = nlohmann::json::array();

	for (auto* n : body) {
		nlohmann::json item;
		item["type"] = n->type;

		if (n->type == "call") {
			auto* cn = static_cast<CallNode*>(n);
			item["func"] = cn->func;

			item["args"] = nlohmann::json::array();
			for (auto& a : cn->args) {
				item["args"].push_back(ExpressionToJson(a));
			}
		} else if (n->type == "assign") {
			auto* an = static_cast<AssignNode*>(n);
			item["var"] = an->varName;
			item["declType"] = an->declType;
			item["expr"] = ExpressionToJson(an->expr);

		} else if (n->type == "for") {
			auto* fn = static_cast<ForNode*>(n);

			item["init"] = ExpressionToJson(fn->initExpr);
			item["cond"] = ExpressionToJson(fn->condExpr);
			item["inc"] = ExpressionToJson(fn->incExpr);
			item["varType"] = fn->loopType;
			item["var"] = fn->loopVar;

			item["body"] = BodyToJson(fn->body);
		} else if (n->type == "if") {
			auto* inNode = static_cast<IfNode*>(n);

			item["cond"] = ExpressionToJson(inNode->condExpr);
			item["then"] = BodyToJson(inNode->thenBody);
			item["else"] = BodyToJson(inNode->elseBody);
		}

		arr.push_back(item);
	}

	return arr;
}

ScriptValue MapProgramAdapter::EvaluateExpression(ExpressionNode* expr, RuntimeContext& ctx) {
	if (!expr) return ScriptValue::Int(0);

	if (expr->isLiteral) return ParseLiteral(expr->literal);

	if (expr->isVariable) {
		/// 1. スクリプト内の変数（int x; / for の i など）
		if (auto it = ctx.locals.find(expr->varName); it != ctx.locals.end()) return it->second;
		/// 2. 組み込み
		if (expr->varName == "width")  return ScriptValue::Int(ctx.width);
		if (expr->varName == "height") return ScriptValue::Int(ctx.height);
		if (expr->varName == "blank")  return ScriptValue::Int(ctx.blankId);
		/// 3. GeneratorVariable（"pos.x" などの成分も）
		ScriptValue v;
		if (ReadGenVar(ctx.genVars, expr->varName, v)) return v;
		/// 4. どこにも無い → 0（打ち間違いに気付けるようにエラーも残す）
		AddError(ctx, "unknown variable: " + expr->varName);
		return ScriptValue::Int(0);
	}

	if (expr->isBinary) {
		const ScriptValue L = EvaluateExpression(expr->left, ctx);
		const ScriptValue R = EvaluateExpression(expr->right, ctx);
		const std::string& op = expr->op;

		/// どちらかが float なら float で計算
		if (L.isFloat || R.isFloat) {
			const float a = L.AsFloat(), b = R.AsFloat();
			if (op == "+") return ScriptValue::Float(a + b);
			if (op == "-") return ScriptValue::Float(a - b);
			if (op == "*") return ScriptValue::Float(a * b);
			if (op == "/") return ScriptValue::Float(b != 0.0f ? a / b : 0.0f);
			if (op == "%") return ScriptValue::Float(b != 0.0f ? std::fmod(a, b) : 0.0f);
			if (op == "<")  return ScriptValue::Int(a < b);
			if (op == ">")  return ScriptValue::Int(a > b);
			if (op == "<=") return ScriptValue::Int(a <= b);
			if (op == ">=") return ScriptValue::Int(a >= b);
			if (op == "==") return ScriptValue::Int(a == b);
			if (op == "!=") return ScriptValue::Int(a != b);
		} else {
			/// 両方 int → 今まで通り（7 / 2 = 3）
			const int a = L.i, b = R.i;
			if (op == "+") return ScriptValue::Int(a + b);
			if (op == "-") return ScriptValue::Int(a - b);
			if (op == "*") return ScriptValue::Int(a * b);
			if (op == "/") return ScriptValue::Int(b != 0 ? a / b : 0);
			if (op == "%") return ScriptValue::Int(b != 0 ? a % b : 0);
			if (op == "<")  return ScriptValue::Int(a < b);
			if (op == ">")  return ScriptValue::Int(a > b);
			if (op == "<=") return ScriptValue::Int(a <= b);
			if (op == ">=") return ScriptValue::Int(a >= b);
			if (op == "==") return ScriptValue::Int(a == b);
			if (op == "!=") return ScriptValue::Int(a != b);
		}
	}

	/// 関数は全部 ScriptBuiltins から（rand / getTile もここ）
	if (expr->isFuncCall) {
		return CallBuiltin(expr->funcName, expr->funcArgs, ctx);   // float を返す関数もある（noise）
	}

	return ScriptValue::Int(0);
}


std::vector<Node*> MapProgramAdapter::LoadAstFromJson(const nlohmann::json& arr) {
	std::vector<Node*> out;

	for (auto& item : arr) {
		std::string type = item["type"];

		if (type == "call") {
			auto* cn = new CallNode();
			cn->type = "call";
			cn->func = item["func"];

			for (auto& a : item["args"]) {
				cn->args.push_back(LoadExpressionFromJson(a));
			}

			out.push_back(cn);
		} else if (type == "assign") {
			auto* an = new AssignNode();
			an->type = "assign";
			an->varName = item["var"];
			an->expr = LoadExpressionFromJson(item["expr"]);
			an->declType = item.value("declType", std::string());
			out.push_back(an);
		} else if (type == "for") {
			auto* fn = new ForNode();
			fn->type = "for";
			fn->loopVar = item.value("var", std::string("i"));
			fn->loopType = item.value("varType", std::string("int"));

			fn->initExpr = LoadExpressionFromJson(item["init"]);
			fn->condExpr = LoadExpressionFromJson(item["cond"]);
			fn->incExpr = LoadExpressionFromJson(item["inc"]);

			fn->body = LoadAstFromJson(item["body"]);

			out.push_back(fn);
		} else if (type == "if") {
			auto* inNode = new IfNode();
			inNode->type = "if";

			inNode->condExpr = LoadExpressionFromJson(item["cond"]);
			inNode->thenBody = LoadAstFromJson(item["then"]);
			inNode->elseBody = LoadAstFromJson(item["else"]);

			out.push_back(inNode);
		}
	}

	return out;
}

ExpressionNode* MapProgramAdapter::LoadExpressionFromJson(const nlohmann::json& j) {
	if (!j.is_object()) return nullptr;
	auto* node = new ExpressionNode();

	std::string type = j["type"];

	if (type == "literal") {
		node->isLiteral = true;
		node->literal = j["value"];
	} else if (type == "variable") {
		node->isVariable = true;
		node->varName = j["name"];
	} else if (type == "binary") {
		node->isBinary = true;
		node->op = j["op"];
		node->left = LoadExpressionFromJson(j["left"]);
		node->right = LoadExpressionFromJson(j["right"]);
	} else if (type == "funcCall") {
		node->isFuncCall = true;
		node->funcName = j["func"];

		for (auto& a : j["args"]) {
			node->funcArgs.push_back(LoadExpressionFromJson(a));
		}
	}

	return node;
}
