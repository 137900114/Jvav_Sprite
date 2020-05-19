#pragma once
#include "Token.h"



struct ASTNode {
	ASTNode(Token t) :token(t) {}
	Token token;
protected:
	//this dummy is useless except with this I can do dynamic cast on ast nodes.
	virtual void dynamic_dummy_func() {}
};

struct BinOp : public ASTNode {
	BinOp(Token t) :ASTNode(t),
		right(nullptr), left(nullptr) {}
	ASTNode* right;
	ASTNode* left;

	TokenType type() { return token.type(); }
	op opera();
};

struct Num : public ASTNode {
	Num(Token t) :ASTNode(t) {}
	uint num() { return token.value(); }
};

struct StringNode : public ASTNode {
	StringNode(Token t) :ASTNode(t) {}
};

struct Empty : public ASTNode {
	Empty(Token t) :ASTNode(t) {}
};

struct ObjectNode : public ASTNode {
	ObjectNode(Token t) :ASTNode(t) {}
};

struct Assign : public ASTNode {
	Assign(Token t) :ASTNode(t) {}
	ObjectNode* lnode;//assign expression's lhs must be a object
	ASTNode* rnode;//assign expresion's rhs must be a expression
};

struct StatementList : public ASTNode {
	StatementList(Token t) :ASTNode(t) {}
	vector<ASTNode*> statemens;
};

struct TypeConvert : public ASTNode {
	TypeConvert(Token t) :ASTNode(t) {}
	ASTNode* target;
};

struct Branch : public ASTNode {
	Branch(Token t) : ASTNode(t) {}
	ASTNode* expr;
	StatementList* if_node;
	StatementList* else_node;
};

struct Loop : public ASTNode {
	Loop(Token t) :ASTNode(t) {}
	ASTNode* expr;
	StatementList* list;
};

struct Invoke : public ASTNode {
	Invoke(Token t) :ASTNode(t) {}
	vector<ASTNode*> params;
};

class Parser : public TokenSpliter {
public:
	ASTNode* get_AST_root() { return root; }

	void parse_line(string str);

	void parse_program(vector<string>& lines);

	void reset();

	Parser() :root(nullptr) {}

private:
	template<typename T>
	T* create_node(Token t);

	ASTNode* express3();
	ASTNode* express2();
	ASTNode* express1();
	ASTNode* factor();

	inline ASTNode* expression() { return express3(); }

	ASTNode* empty();
	ASTNode* single_statement();
	ASTNode* assign_statement();
	ASTNode* declare_statement();

	ASTNode* function_declare_statement(FuncSignature& sig);
	ASTNode* invoke();
	ASTNode* function_statement(FuncSignature& sig);
	ASTNode* declare_in_func(FuncSignature& sig);

	ASTNode* statement();
	ASTNode* ndcl_statement();

	ASTNode* loop();
	ASTNode* branch();

	ASTNode* root;

	bool is_parsing_func = false;
};

template<typename T>
T* Parser::create_node(Token t) {
	uint node_size = sizeof(T);
	Meta mem = allocate(node_size);

	return new (mem.data) T(t);
}

class Interpreter : public MetaPool {
public:
	void program(ASTNode* node);

	void statement_list(StatementList* node);

	void statement(ASTNode* node);

	//expr will return a token in the future
	Token expr(ASTNode* node);
private:
	Token calcu(ASTNode* node);

	void assign(Assign* ass);

	void call(Invoke* invoke);

	static int cmd_counter;
	static const int release_cycle = 100;
};

//JVM(JVav's Mother)
class JVM {
public:
	void command_line(string cmd);
	void excute(vector<string>& cmds);
private:
	Interpreter inter;
	Parser par;

};