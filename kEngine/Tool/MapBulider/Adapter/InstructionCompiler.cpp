#include <iostream>
#include "InstructionCompiler.h"

std::vector<std::string> Tokenize(const std::string& program) {
	std::vector<std::string> tokens;
	std::string cur;

	auto flush = [&]() {
		if (!cur.empty()) {
			tokens.push_back(cur);
			cur.clear();
		}
		};

	auto isSymbol = [&](char c) {
		return c == '(' || c == ')' ||
			c == '{' || c == '}' ||
			c == ';' || c == ':' ||
			c == '=' || c == '+' ||
			c == '-' || c == '*' ||
			c == '/' || c == '<' ||
			c == '>' || c == '!';
		};

	for (size_t i = 0; i < program.size(); i++) {
		char c = program[i];

		// 空白：結束 token
		if (c == ' ' || c == '\t' || c == '\n') {
			flush();
			continue;
		}

		// ★ 先檢查兩字元運算子 ★
		if (i + 1 < program.size()) {
			std::string two = program.substr(i, 2);

			if (two == "+=" || two == "-=" ||
				two == "==" || two == "!=" ||
				two == "<=" || two == ">=" ||
				two == "++" || two == "--" ||
				two == "&&" || two == "||") {

				flush();
				tokens.push_back(two);
				i++; // 跳過第二個字元
				continue;
			}
		}

		// 單字元符號
		if (isSymbol(c)) {
			flush();
			tokens.push_back(std::string(1, c));
			continue;
		}

		// 一般字元
		cur += c;
	}
	flush();
	return tokens;
}

std::vector<std::string> BuildSentences(const std::vector<std::string>& tokens) {
	std::vector<std::string> sentences;
	std::string cur;

	int parenDepth = 0; // 用來判斷是否在 for(...) 裡面

	for (const std::string& t : tokens) {
		// 加入 token
		cur += t;

		// 括號深度
		if (t == "(") parenDepth++;
		if (t == ")") parenDepth--;

		// block 開始
		if (t == "{") {
			sentences.push_back(cur);
			cur.clear();
			continue;
		}

		// block 結束
		if (t == "}") {
			sentences.push_back(cur);
			cur.clear();
			continue;
		}

		// 一般語句結束（但不能在括號內）
		if (t == ";" && parenDepth == 0) {
			sentences.push_back(cur);
			cur.clear();
			continue;
		}

		// token 之間加空白讓語句好讀
		cur += " ";
	}

	if (!cur.empty())
		sentences.push_back(cur);

	return sentences;
}

/// "result[Y][X]" を Y, X の式に分ける（形が違えば false）
static bool SplitResultIndex(const std::string& src, std::string& outY, std::string& outX) {
	const std::string t = Trim(src);
	if (t.rfind("result", 0) != 0) return false;
	size_t p = 6;   // "result" の後ろ
	auto readBracket = [&](std::string& out) {
		while (p < t.size() && t[p] == ' ') ++p;
		if (p >= t.size() || t[p] != '[') return false;
		const size_t start = p + 1;
		int depth = 0;
		for (; p < t.size(); ++p) {
			if (t[p] == '[') depth++;
			else if (t[p] == ']' && --depth == 0) { out = t.substr(start, p - start); ++p; return true; }
		}
		return false;
		};
	return readBracket(outY) && readBracket(outX) && Trim(t.substr(p)).empty();
}

/// "xxx ( ... ) {" の最初の '(' に対応する ')' までの中身（rand(100) のような入れ子にも対応）
static std::string HeaderInside(const std::string& s) {
	const size_t l = s.find('(');
	if (l == std::string::npos) return {};
	int depth = 0;
	for (size_t i = l; i < s.size(); ++i) {
		if (s[i] == '(') depth++;
		if (s[i] == ')' && --depth == 0) return s.substr(l + 1, i - l - 1);
	}
	return s.substr(l + 1);
}

/// 1 文を解析して out に追加し、index を次の文へ進める（for / if-else はブロックごと）
static void ParseStatement(const std::vector<std::string>& sentences, int& index, GeneratorVariable* genVars, std::vector<Node*>& out) {
	const std::string s = Trim(sentences[index]);

	/// for(init;cond;inc){ ... }
	if (s.rfind("for", 0) == 0 && s.find('{') != std::string::npos) {
		auto header = ParseSentence(s, genVars);
		if (!header.empty()) {
			if (auto* forNode = dynamic_cast<ForNode*>(header[0])) {
				forNode->body = ParseBlock(sentences, index, genVars);   // index はヘッダー行 → ブロックの次へ
				out.push_back(forNode);
				return;
			}
		}
	}

	/// if(cond){ ... } else { ... }
	if (s.rfind("if", 0) == 0 && s.find('{') != std::string::npos) {
		auto* ifNode = new IfNode();
		ifNode->type = "if";
		ifNode->condExpr = ParseExpression(HeaderInside(s), genVars);
		ifNode->thenBody = ParseBlock(sentences, index, genVars);

		/// else：ParseBlock が「else {」の行を飛ばしてくれるので、そのまま渡す
		if (index < static_cast<int>(sentences.size()) && Trim(sentences[index]).rfind("else", 0) == 0) {
			ifNode->elseBody = ParseBlock(sentences, index, genVars);
		}
		out.push_back(ifNode);
		return;
	}

	/// 余った '}' は飛ばす
	if (s == "}") { index++; return; }

	/// その他：assign / call など 1 行
	auto nodes = ParseSentence(s, genVars);
	out.insert(out.end(), nodes.begin(), nodes.end());
	index++;
}

std::vector<Node*> ParseBlock(const std::vector<std::string>& sentences, int& index, GeneratorVariable* genVars) {
	std::vector<Node*> body;
	// 跳過 '{'
	index++;
	while (index < static_cast<int>(sentences.size())) {
		if (Trim(sentences[index]) == "}") {
			index++; // 跳過 '}'
			break;
		}
		ParseStatement(sentences, index, genVars, body);   // 入れ子の for / if もここで
	}

	return body;
}

std::vector<Node*> ParseSentence(const std::string& sentence, GeneratorVariable* genVars) {
	std::vector<Node*> out;
	std::string s = Trim(sentence);
	if (!s.empty() && s.back() == ';') s = Trim(s.substr(0, s.size() - 1));

	// if(cond){
	if (s.rfind("if", 0) == 0) {
		auto* node = new IfNode();
		node->type = "if";

		std::string cond = HeaderInside(s);
		node->condExpr = ParseExpression(cond, genVars); // 你也可以改成 ParseExpression(cond)

		out.push_back(node);
		return out;
	}

	// for-loop
	if (s.rfind("for", 0) == 0) {
		auto* node = new ForNode();
		node->type = "for";

		std::string inside = HeaderInside(s);
		std::cout << "inside = [" << inside << "]" << std::endl;

		std::vector<std::string> parts;
		{
			std::string cur;
			for (char c : inside) {
				if (c == ';') {
					parts.push_back(Trim(cur));
					cur.clear();
				} else {
					cur += c;
				}
			}
			if (!cur.empty()) parts.push_back(Trim(cur));
		}

		if (parts.size() == 3) {
			// init：用 ParseSentence 再跑一次，吃你已經寫好的 += / i++ / assignment 規則
			{
				auto initNodes = ParseSentence(parts[0], genVars);
				if (!initNodes.empty()) {
					if (auto* an = dynamic_cast<AssignNode*>(initNodes[0])) {
						node->loopVar = an->varName;
						node->loopType = an->declType.empty() ? std::string("int") : an->declType;   // for (float t = 0; ...)
						node->initExpr = an->expr;
					} else if (auto* exprNode = dynamic_cast<ExpressionNode*>(initNodes[0])) {
						node->initExpr = exprNode;
					}
				}
			}

			// cond：仍然是純 expression
			node->condExpr = ParseExpression(parts[1], genVars);

			// inc：同樣用 ParseSentence，再吃你已經寫好的 += / i++ 規則
			{
				auto incNodes = ParseSentence(parts[2], genVars);
				if (!incNodes.empty()) {
					if (auto* an = dynamic_cast<AssignNode*>(incNodes[0])) {
						node->incExpr = an->expr;
					} else if (auto* exprNode = dynamic_cast<ExpressionNode*>(incNodes[0])) {
						node->incExpr = exprNode;
					}
				} else {
					// ⭐ 純 expression：例如 i+1
					node->incExpr = ParseExpression(parts[2], genVars);
				}
			}
		}

		out.push_back(node);
		return out;
	}

	/// 宣言：int x; / int x = 5; / float f = 0.5;（for (int i = 0; ...) の init もここを通る）
	for (const std::string kw : { "int", "float" }) {
		if (s.rfind(kw + " ", 0) != 0) continue;

		auto* node = new AssignNode();
		node->type = "assign";
		node->declType = kw;

		std::string rest = Trim(s.substr(kw.size()));
		if (!rest.empty() && rest.back() == ';') rest = Trim(rest.substr(0, rest.size() - 1));   // "int x ;" の ;
		const size_t eq = rest.find('=');
		node->varName = Trim(rest.substr(0, eq));                                   // "x = 5" → "x" / "x" → "x"
		node->expr = ParseExpression(eq == std::string::npos ? std::string("0") : rest.substr(eq + 1), genVars);   // 初期値なし → 0

		out.push_back(node);
		return out;
	}

	// +=, -=
	if (s.find("+=") != std::string::npos || s.find("-=") != std::string::npos) {
		auto* node = new AssignNode();
		node->type = "assign";

		bool isAdd = s.find("+=") != std::string::npos;
		size_t pos = isAdd ? s.find("+=") : s.find("-=");

		std::string var = Trim(s.substr(0, pos));
		std::string rhs = Trim(s.substr(pos + 2));



		// c.z += i  →  c.z = c.z + i
		ExpressionNode* left = ParseExpression(var, genVars);
		ExpressionNode* right = ParseExpression(rhs, genVars);

		ExpressionNode* bin = new ExpressionNode();
		bin->isBinary = true;
		bin->op = isAdd ? "+" : "-";
		bin->left = left;
		bin->right = right;

		node->varName = var;
		node->expr = bin;

		out.push_back(node);
		return out;
	}

	// i++ / i--
	if (s.size() >= 2 && (s.substr(s.size() - 2) == "++" || s.substr(s.size() - 2) == "--")) {
		auto* node = new AssignNode();
		node->type = "assign";

		bool isAdd = s.substr(s.size() - 2) == "++";
		std::string var = Trim(s.substr(0, s.size() - 2));

		ExpressionNode* left = ParseExpression(var, genVars);

		ExpressionNode* one = new ExpressionNode();
		one->isLiteral = true;
		one->literal = "1";

		ExpressionNode* bin = new ExpressionNode();
		bin->isBinary = true;
		bin->op = isAdd ? "+" : "-";
		bin->left = left;
		bin->right = one;

		node->varName = var;
		node->expr = bin;

		out.push_back(node);
		return out;
	}

	// assignment
	if (s.find("=") != std::string::npos &&
		s.find("==") == std::string::npos &&
		s.find(">=") == std::string::npos &&
		s.find("<=") == std::string::npos &&
		s.find("+=") == std::string::npos &&
		s.find("-=") == std::string::npos &&
		s.find("++") == std::string::npos &&
		s.find("--") == std::string::npos &&
		s.rfind("if", 0) != 0) {
		auto* node = new AssignNode();
		node->type = "assign";

		size_t eq = s.find("=");

		node->varName = Trim(s.substr(0, eq));
		std::string rhs = Trim(s.substr(eq + 1));

		/// result[y][x] = v → setTile(x, y, v)（マップへの出力）
		std::string ys, xs;
		if (SplitResultIndex(node->varName, ys, xs)) {
			delete node;
			auto* call = new CallNode();
			call->type = "call";
			call->func = "setTile";
			call->args = { ParseExpression(xs, genVars), ParseExpression(ys, genVars), ParseExpression(rhs, genVars) };
			out.push_back(call);
			return out;
		}

		node->expr = ParseExpression(rhs, genVars);

		out.push_back(node);
		return out;
	}

	// function call: func(...)
	size_t posParen = s.find("(");
	if (posParen != std::string::npos) {
		auto* node = new CallNode();
		node->type = "call";

		node->func = Trim(s.substr(0, posParen));

		std::string inside = HeaderInside(s);

		std::vector<std::string> args;
		{
			std::string cur;
			int depth = 0;

			for (char c : inside) {
				if (c == '(') depth++;
				if (c == ')') depth--;

				if (c == ',' && depth == 0) {
					args.push_back(Trim(cur));
					cur.clear();
				} else {
					cur += c;
				}
			}
			if (!cur.empty()) args.push_back(Trim(cur));
		}

		for (auto& a : args) {
			ExpressionNode* exprArg = ParseExpression(a, genVars);
			node->args.push_back(exprArg);
		}

		out.push_back(node);
		return out;
	}

	return out;
}

std::vector<Node*> ParseProgram(const std::vector<std::string>& sentences, GeneratorVariable* genVars) {
	std::vector<Node*> out;
	for (int i = 0; i < static_cast<int>(sentences.size()); ) {
		ParseStatement(sentences, i, genVars, out);
	}
	return out;
}

std::string Trim(const std::string& s) {
	size_t start = s.find_first_not_of(" \t\n\r");
	if (start == std::string::npos) return "";

	size_t end = s.find_last_not_of(" \t\n\r");
	return s.substr(start, end - start + 1);
}

/// 数字か？（12 / -3 / 1.5 / .5 / 2.0f）。"pos.x" は数字ではない
static bool IsNumberLiteral(const std::string& s) {
	size_t i = (s.size() > 1 && s[0] == '-') ? 1 : 0;
	size_t end = s.size();
	if (end > i + 1 && (s[end - 1] == 'f' || s[end - 1] == 'F') && s.find('.') != std::string::npos) end--;   // 2.0f
	bool digit = false, dot = false;
	for (; i < end; ++i) {
		if (std::isdigit(static_cast<unsigned char>(s[i]))) digit = true;
		else if (s[i] == '.' && !dot) dot = true;
		else return false;
	}
	return digit;
}

/// s[i] の +/- が「符号」か？（左が空 / 左が演算子 → -1 や a * -1 の '-'）
static bool IsUnarySign(const std::string& s, int i) {
	int k = i - 1;
	while (k >= 0 && s[k] == ' ') --k;
	if (k < 0) return true;
	return std::string("+-*/%<>=!(,").find(s[k]) != std::string::npos;
}

ExpressionNode* ParseExpression(const std::string& token, GeneratorVariable* genVars) {

	std::string s = Trim(token);

	// ;を削除（for-loop の最後の式などで残る場合がある）
	if (!s.empty() && s.back() == ';')
		s.pop_back();
	// 改変を加えた後に再度トリム
	s = Trim(s);

	// 前後の + - を削除（例: +i-1 のようなケースを考慮）
	//while (!s.empty() && (s.back() == '+' || s.back() == '-'))
	//	s.pop_back();
	// 改変を加えた後に再度トリム
	//s = Trim(s);


	// 括號で囲まれている場合は中身を再解析する
	if (s.size() >= 2 && s.front() == '(' && s.back() == ')') {
		// 括號が本当にペアか確認（例: (i+1)*2 のようなケースを除外）
		int depth = 0;
		bool isWrapped = true;

		for (int i = 0; i < s.size(); ++i) {
			if (s[i] == '(') depth++;
			if (s[i] == ')') depth--;

			// 最後の文字以外で depth が 0 になったら、外側括號ではない
			if (depth == 0 && i < s.size() - 1) {
				isWrapped = false;
				break;
			}
		}

		if (isWrapped) {
			return ParseExpression(s.substr(1, s.size() - 2), genVars);
		}
	}

	// 次に比較演算子 <= >= == != < > を探す（括弧の中は無視）
	int parenDepth = 0;
	for (int i = (int)(s.size() - 1); i >= 0; --i) {

		if (s[i] == ')' || s[i] == ']') parenDepth++;
		if (s[i] == '(' || s[i] == '[') parenDepth--;

		if (parenDepth != 0) continue;

		// 兩字元運算子
		if (i > 0) {
			std::string op2 = s.substr(i - 1, 2);
			if (op2 == "<=" || op2 == ">=" || op2 == "==" || op2 == "!=") {
				ExpressionNode* node = new ExpressionNode();
				node->isBinary = true;
				node->op = op2;

				node->left = ParseExpression(s.substr(0, i - 1), genVars);
				node->right = ParseExpression(s.substr(i + 1), genVars);
				return node;
			}
		}

		// 單字元運算子 < >
		if (s[i] == '<' || s[i] == '>') {
			ExpressionNode* node = new ExpressionNode();
			node->isBinary = true;
			node->op = std::string(1, s[i]);

			node->left = ParseExpression(s.substr(0, i), genVars);
			node->right = ParseExpression(s.substr(i + 1), genVars);
			return node;
		}
	}

	// まず+-を探す（括弧の中は無視）
	parenDepth = 0;
	for (int i = (int)(s.size() - 1); i >= 0; --i) {
		char c = s[i];

		if (c == ')' || c == ']') parenDepth++;
		if (c == '(' || c == '[') parenDepth--;

		// 括弧の深さが0のときに+-を見つけたら、二項演算子として分割する
		if (parenDepth == 0 &&
			(c == '+' || c == '-') &&
			!(i > 0 && s[i] == '+' && s[i - 1] == '+') &&
			!(i > 0 && s[i] == '-' && s[i - 1] == '-') &&
			!IsUnarySign(s, i)) {
			ExpressionNode* node = new ExpressionNode();
			node->isBinary = true;
			node->op = std::string(1, c);

			node->left = ParseExpression(s.substr(0, i), genVars);
			node->right = ParseExpression(s.substr(i + 1), genVars);
			return node;
		}
	}

	// 次に*/を探す（括弧の中は無視）
	parenDepth = 0;
	for (int i = (int)(s.size() - 1); i >= 0; --i) {
		char c = s[i];

		if (c == ')' || c == ']') parenDepth++;
		if (c == '(' || c == '[') parenDepth--;

		if (parenDepth == 0 && (c == '*' || c == '/' || c == '%')) {
			ExpressionNode* node = new ExpressionNode();
			node->isBinary = true;
			node->op = std::string(1, c);

			node->left = ParseExpression(s.substr(0, i), genVars);
			node->right = ParseExpression(s.substr(i + 1), genVars);
			return node;
		}
	}

	// 符号：- a / - 3 → 0 - a（"-3" のようにくっついた数字は後の literal で処理）
	if (!s.empty() && s[0] == '-' && !IsNumberLiteral(s)) {
		ExpressionNode* node = new ExpressionNode();
		node->isBinary = true;
		node->op = "-";
		node->left = new ExpressionNode();
		node->left->isLiteral = true;
		node->left->literal = "0";
		node->right = ParseExpression(s.substr(1), genVars);
		return node;
	}

	// result[y][x] を読む → getTile(x, y)
	{
		std::string ys, xs;
		if (SplitResultIndex(s, ys, xs)) {
			ExpressionNode* node = new ExpressionNode();
			node->isFuncCall = true;
			node->funcName = "getTile";
			node->funcArgs = { ParseExpression(xs, genVars), ParseExpression(ys, genVars) };
			return node;
		}
	}

	// 関数呼び出しの解析
	size_t posParen = s.find("(");
	if (posParen != std::string::npos && s.back() == ')') {

		ExpressionNode* node = new ExpressionNode();
		node->isFuncCall = true;

		node->funcName = Trim(s.substr(0, posParen));

		std::string inside = s.substr(posParen + 1, s.size() - posParen - 2);

		// 拆參數
		std::vector<std::string> args;
		{
			std::string cur;
			int depth = 0;

			for (char c : inside) {
				if (c == '(') depth++;
				if (c == ')') depth--;

				if (c == ',' && depth == 0) {
					args.push_back(Trim(cur));
					cur.clear();
				} else {
					cur += c;
				}
			}

			if (!cur.empty()) args.push_back(Trim(cur));
		}

		for (auto& a : args) {
			node->funcArgs.push_back(ParseExpression(a, genVars));
		}

		return node;
	}

	if (s == "true" || s == "false") {
		ExpressionNode* expr = new ExpressionNode();
		expr->isLiteral = true;
		expr->literal = s;
		return expr;
	}

	if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
		ExpressionNode* expr = new ExpressionNode();
		expr->isLiteral = true;
		expr->literal = s; // 保留引號
		return expr;
	}

	// 最後に、変数かリテラルかを判定（数字ならリテラル、それ以外は変数）
	ExpressionNode* expr = new ExpressionNode();
	const bool isNumber = IsNumberLiteral(s);
	if (isNumber) {
		expr->isLiteral = true;
		expr->literal = s;
	} else {
		expr->isVariable = true;
		expr->varName = s;
	}

	return expr;
}
