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

std::vector<Node*> ParseBlock(const std::vector<std::string>& sentences, int& index, GeneratorVariable* genVars) {
	std::vector<Node*> body;

	// 跳過 '{'
	index++;

	while (index < sentences.size()) {
		std::string s = Trim(sentences[index]);

		if (s == "}") {
			index++; // 跳過 '}'
			break;
		}

		auto nodes = ParseSentence(s, genVars);
		for (auto* n : nodes)
			body.push_back(n);

		index++;
	}

	return body;
}

std::vector<Node*> ParseSentence(const std::string& sentence, GeneratorVariable* genVars) {
	std::vector<Node*> out;
	std::string s = Trim(sentence);

	// if(cond){
	if (s.rfind("if", 0) == 0) {
		auto* node = new IfNode();
		node->type = "if";

		size_t l = s.find("(");
		size_t r = s.find(")");

		std::string cond = s.substr(l + 1, r - l - 1);
		node->condExpr = ParseExpression(cond, genVars); // 你也可以改成 ParseExpression(cond)

		out.push_back(node);
		return out;
	}

	// for-loop
	if (s.rfind("for", 0) == 0) {
		auto* node = new ForNode();
		node->type = "for";

		size_t l = s.find("(");
		size_t r = s.find(")");
		std::string inside = s.substr(l + 1, r - l - 1);
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

		size_t posR = s.find(")");
		std::string inside = s.substr(posParen + 1, posR - posParen - 1);

		std::vector<std::string> args;
		{
			std::string cur;
			int depth = 0;

			for (char c : inside) {
				if (c == '(') depth++;
				if (c == ')') depth--;

				if (c == ',') {
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
		std::string s = Trim(sentences[i]);

		// ============================
		// for(init;cond;inc){ ... }
		// ============================
		if (s.rfind("for", 0) == 0 && s.find("{") != std::string::npos) {

			// 先 parse for(...) header（單行）
			auto headerNodes = ParseSentence(s, genVars);
			if (!headerNodes.empty()) {
				auto* forNode = dynamic_cast<ForNode*>(headerNodes[0]);
				if (forNode) {

					// ⭐ 呼叫 ParseBlock（吃多行 + index）
					forNode->body = ParseBlock(sentences, i, genVars);

					out.push_back(forNode);
					continue;   // ⭐ 跳過 block 內的行
				}
			}
		}

		// ============================
		// if(cond){ ... } else { ... }
		// ============================
		if (s.rfind("if", 0) == 0 && s.find("{") != std::string::npos) {
			auto* ifNode = new IfNode();
			ifNode->type = "if";

			size_t l = s.find("(");
			size_t r = s.find(")");
			std::string cond = s.substr(l + 1, r - l - 1);
			ifNode->condExpr = ParseExpression(cond, genVars);

			ifNode->thenBody = ParseBlock(sentences, i, genVars);

			if (i < static_cast<int>(sentences.size())) {
				std::string next = Trim(sentences[i]);
				if (next.rfind("else", 0) == 0) {
					i++;
					ifNode->elseBody = ParseBlock(sentences, i, genVars);
				}
			}

			out.push_back(ifNode);
			continue;
		}

		// ============================
		// 其他語句：assign / call / etc
		// ============================
		auto nodes = ParseSentence(s, genVars);
		for (auto* n : nodes) out.push_back(n);
		i++;
	}

	return out;
}


std::string Trim(const std::string& s) {
	size_t start = s.find_first_not_of(" \t\n\r");
	if (start == std::string::npos) return "";

	size_t end = s.find_last_not_of(" \t\n\r");
	return s.substr(start, end - start + 1);
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

	// まず+-を探す（括弧の中は無視）
	int parenDepth = 0;
	for (int i = (int)(s.size() - 1); i >= 0; --i) {
		char c = s[i];

		if (c == ')') parenDepth++;
		if (c == '(') parenDepth--;

		// 括弧の深さが0のときに+-を見つけたら、二項演算子として分割する
		if (parenDepth == 0 &&
			(c == '+' || c == '-') &&
			!(i > 0 && s[i] == '+' && s[i - 1] == '+') &&
			!(i > 0 && s[i] == '-' && s[i - 1] == '-')) {
			ExpressionNode* node = new ExpressionNode();
			node->isBinary = true;
			node->op = std::string(1, c);

			node->left = ParseExpression(s.substr(0, i), genVars);
			node->right = ParseExpression(s.substr(i + 1), genVars);
			return node;
		}
	}

	// 次に比較演算子 <= >= == != を探す（括弧の中は無視）
	parenDepth = 0;
	for (int i = (int)(s.size() - 1); i >= 0; --i) {
		if (s[i] == ')') parenDepth++;
		if (s[i] == '(') parenDepth--;

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

	// 次に*/を探す（括弧の中は無視）
	parenDepth = 0;
	for (int i = (int)(s.size() - 1); i >= 0; --i) {
		char c = s[i];

		if (c == ')') parenDepth++;
		if (c == '(') parenDepth--;

		if (parenDepth == 0 && (c == '*' || c == '/' || c == '%')) {
			ExpressionNode* node = new ExpressionNode();
			node->isBinary = true;
			node->op = std::string(1, c);

			node->left = ParseExpression(s.substr(0, i), genVars);
			node->right = ParseExpression(s.substr(i + 1), genVars);
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

	if (s.find('.') != std::string::npos) {
		ExpressionNode* expr = new ExpressionNode();
		expr->isVariable = true;
		expr->varName = s; // 例如 "c.z"
		return expr;
	}

	// 最後に、変数かリテラルかを判定
	ExpressionNode* expr = new ExpressionNode();
	if (genVars && genVars->FindVariable(s)) {
		expr->isVariable = true;
		expr->varName = s;
	} else {
		expr->isLiteral = true;
		expr->literal = s;
	}

	return expr;
}
