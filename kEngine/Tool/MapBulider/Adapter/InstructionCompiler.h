#pragma once
#include <vector>
#include <string>
#include "Tool/MapBulider/Data/GeneratorVariable.h"

struct ExpressionNode {
    bool isLiteral = false;
    bool isVariable = false;
    bool isBinary = false;
    bool isFuncCall = false;

    std::string literal;   // "10"
    std::string varName;   // "i"
    std::string op;
    std::string funcName;


    ExpressionNode* left = nullptr;
    ExpressionNode* right = nullptr;
    std::vector<ExpressionNode*> funcArgs;
};

struct Node {
    std::string type; // "for", "if", "call"
    std::vector<Node*> body; // for-loop 或 if 的 then/else
    virtual ~Node() {};
};

struct ArgNode {
    bool isLiteral;
    bool isVariable;

    std::string literal;
    std::string varName;
};

struct ForNode : Node {
    ExpressionNode* initExpr;
    ExpressionNode* condExpr;
    ExpressionNode* incExpr;
};

struct IfNode : Node {
    ExpressionNode* condExpr = nullptr;
    std::vector<Node*> thenBody;
    std::vector<Node*> elseBody;
};


struct CallNode : Node {
    std::string func;
    std::vector<ExpressionNode*> args;
};

struct AssignNode : Node {
    std::string varName;
    ExpressionNode* expr;
};


std::vector<std::string> Tokenize(const std::string& program);

std::vector<std::string> BuildSentences(const std::vector<std::string>& tokens);

std::vector<Node*> ParseBlock(const std::vector<std::string>& sentences, int& index, GeneratorVariable* genVars);
std::vector<Node*> ParseSentence(const std::string& sentence, GeneratorVariable* genVars = nullptr);
std::vector<Node*> ParseProgram(const std::vector<std::string>& sentences, GeneratorVariable* genVars);

std::string Trim(const std::string& s);

ExpressionNode* ParseExpression(const std::string& token, GeneratorVariable* genVars);
