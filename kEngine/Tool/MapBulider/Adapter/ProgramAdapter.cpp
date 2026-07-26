#include "ProgramAdapter.h"

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
	nlohmann::json nodeJson = nlohmann::json::array();

	for (auto* n : nodes) {
		nlohmann::json item;
		item["type"] = n->type;

		if (n->type == "call") {
			auto* cn = static_cast<CallNode*>(n);
			item["func"] = cn->func;

			nlohmann::json args = nlohmann::json::array();
			for (auto& a : cn->args) {
				args.push_back(ExpressionToJson(a)); // 之後改成 ExpressionNode
			}
			item["args"] = args;
		} else if (n->type == "assign") {
			auto* an = static_cast<AssignNode*>(n);
			item["var"] = an->varName;
			item["expr"] = ExpressionToJson(an->expr);
		} else if (n->type == "for") {
			auto* fn = static_cast<ForNode*>(n);
			item["init"] = ExpressionToJson(fn->initExpr);
			item["cond"] = ExpressionToJson(fn->condExpr);
			item["inc"] = ExpressionToJson(fn->incExpr);

			item["body"] = BodyToJson(fn->body);
			// 之後補 body AST
		} else if (n->type == "if") {
			auto* in = static_cast<IfNode*>(n);

			item["cond"] = ExpressionToJson(in->condExpr);
			item["then"] = BodyToJson(in->thenBody);
			item["else"] = BodyToJson(in->elseBody);
		}

		nodeJson.push_back(item);
	}
	file["nodes"] = nodeJson;

	file["debug_tokens"] = tokens;
	file["debug_sentences"] = sentences;


	// 寫入檔案
	std::ofstream out(path);
	out << file.dump(4);
}

TileMapData MapProgramAdapter::LoadTileMap(const std::string& filePath) {
	return TileMapData();
}

TileMapData MapProgramAdapter::RunMapBulider(const std::string& filePath, GeneratorVariable* generatorVariables_) {
	// 讀取 JSON
	std::ifstream in(filePath);
	if (!in.is_open()) {
		std::cout << "MapBuilder file not found: " << filePath << std::endl;
		return TileMapData();
	}

	nlohmann::json file;
	in >> file;

	// 建立 TileMapData
	TileMapData map;
	map.name = file.value("name", "UnnamedMap");

	// 讀取 context
	int maxRow = file["context"].value("maxRow", 0);
	int maxCol = file["context"].value("maxCol", 0);

	// 初始化 Row
	map.Row.resize(maxRow);
	for (int y = 0; y < maxRow; y++) {
		map.Row[y].resize(maxCol);
		for (int x = 0; x < maxCol; x++) {
			map.Row[y][x] = 0; // 預設 AIR
		}
	}

	// 建立 RuntimeContext
	RuntimeContext ctx;
	ctx.maxRow = maxRow;
	ctx.maxCol = maxCol;
	ctx.map = &map;
	ctx.genVars = generatorVariables_; // ★ MapMaker 的變數表

	// 讀取 script
	auto script = file["script"];

	// JSON → AST
	std::vector<Node*> nodes = LoadAstFromJson(script);

	// 執行 AST
	for (auto* n : nodes) {
		ExecuteNode(n, ctx);
	}

	return map;
}

void MapProgramAdapter::ExecuteNode(Node* node, RuntimeContext& ctx) {
	if (node->type == "call") {
		auto* cn = static_cast<CallNode*>(node);

		if (cn->func == "setTile") {
			if (cn->func == "setTile") {

				int x = EvaluateExpression(cn->args[0], ctx);
				int y = EvaluateExpression(cn->args[1], ctx);
				int type = EvaluateExpression(cn->args[2], ctx);

				ctx.map->Row[y][x] = type;
			}
		}
	} else if (node->type == "assign") {
		auto* an = static_cast<AssignNode*>(node);
		int value = EvaluateExpression(an->expr, ctx);

		Variable* v = ctx.genVars->FindVariable(an->varName);
		if (v) {
			v->type = VarType::Int;
			v->value = value;
		}
	} else if (node->type == "for") {
		auto* fn = static_cast<ForNode*>(node);

		for (int i = EvaluateExpression(fn->initExpr, ctx);
			EvaluateExpression(fn->condExpr, ctx);
			i = EvaluateExpression(fn->incExpr, ctx)) {
			ctx.loopVar = i;

			for (auto* child : fn->body)
				ExecuteNode(child, ctx);
		}
	} else if (node->type == "if") {
		auto* inNode = static_cast<IfNode*>(node);

		int cond = EvaluateExpression(inNode->condExpr, ctx);

		if (cond) {
			for (auto* child : inNode->thenBody)
				ExecuteNode(child, ctx);
		} else {
			for (auto* child : inNode->elseBody)
				ExecuteNode(child, ctx);
		}
	}
}

nlohmann::json MapProgramAdapter::ExpressionToJson(ExpressionNode* expr) {
	nlohmann::json j;

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
			item["expr"] = ExpressionToJson(an->expr);
		} else if (n->type == "for") {
			auto* fn = static_cast<ForNode*>(n);

			item["init"] = ExpressionToJson(fn->initExpr);
			item["cond"] = ExpressionToJson(fn->condExpr);
			item["inc"] = ExpressionToJson(fn->incExpr);

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

int MapProgramAdapter::EvaluateExpression(ExpressionNode* expr, RuntimeContext& ctx) {
	if (expr->isLiteral) {
		return std::stoi(expr->literal);
	}
	if (expr->isVariable) {
		// 1. for-loop 變數
		if (expr->varName == "i")
			return ctx.loopVar;
		// 2. GeneratorVariable
		Variable* v = ctx.genVars->FindVariable(expr->varName);
		if (v) {
			switch (v->type) {
			case VarType::Int: return GetValue<int>(*v);
			case VarType::Float: return (int)GetValue<float>(*v);
			case VarType::Double: return (int)GetValue<double>(*v);
			case VarType::Bool: return GetValue<bool>(*v) ? 1 : 0;
			case VarType::String: return std::stoi(GetValue<std::string>(*v));
			default: break;
			}
		}
	}
	if (expr->isBinary) {
		int L = EvaluateExpression(expr->left, ctx);
		int R = EvaluateExpression(expr->right, ctx);

		if (expr->op == "+") return L + R;
		if (expr->op == "-") return L - R;
		if (expr->op == "*") return L * R;
		if (expr->op == "/") return L / R;
		if (expr->op == "%") return L % R;

		if (expr->op == "<") return L < R;
		if (expr->op == ">") return L > R;
		if (expr->op == "<=") return L <= R;
		if (expr->op == ">=") return L >= R;
		if (expr->op == "==") return L == R;
		if (expr->op == "!=") return L != R;
	}
	if (expr->isFuncCall) {
		if (expr->funcName == "rand") {
			return rand() % 100; // 你之後可以改成 RNG
		}
	}

	return 0;
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
			out.push_back(an);
		} else if (type == "for") {
			auto* fn = new ForNode();
			fn->type = "for";

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
