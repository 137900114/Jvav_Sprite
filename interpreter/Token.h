#pragma once
#include "Meta.h"
#include "common.h"
#include "Object.h"

enum  TokenType {
	INVAILD = 0,//this type is invaild.

	INTEGER,//a constant integer number
	FLOAT,//a constant float number
	STRING,//a constant string

	ADD_SUB,//add and sub operator
	MUL_DIV,//multiple and divide operator
	CMP_OP,//operator compare two tokens like < , >
	END,//end of a expression

	LPAREN,//(
	RPAREN,//)
	FIELD_START,//{ the start of a field
	FIELD_END,//} the end of a field
	IF,//represent a if reserved word
	ELSE,//represent a else reserved word
	LOOP,//represent a loop reserved word

	ASSIGN,//= assign a value to a variable

	DCL_INT, //notations of spcifing a variable 
	DCL_FLOAT,//a variable float
	DCL_STRING,//a variable saves string(size will dynamicly change)

	ENTRY, //the entry of the jvav
	OBJECT, //stands for a variable
	EMPTY,//stands for no command in a line
	MAIN_FIELD //stands for the main field of the system
};

string translate_reserved_word(TokenType Token);
TokenType translate_reserved_word(string word);

//jvav has two modes : command line mode and excute mode
/*

[ Grammar for jvav excute mode ]
> program : ENTRY FIELD_START END statement_list FIELD_END
> statement_list : ndcl_statement_list | declare_statment END statement_list
> branch : IF LPAREN expression RPAREN FIELD_START END ndcl_statement_list FIELD_END (ELSE FIELD START ndcl_statement_list FIELD_END) END
> ndcl_statement_list :  loop | branch | assign_statement | EMPTY END ndcl_statement_list
> loop : LOOP LPAREN expression RPAREN FIELD_START ndcl_statement_list FIELD_END END
> END : \n
> statement : declare_statement | assign_statement | EMPTY
> EMPTY :
> assign_statement : OBJECT ASSIGN expression
> declare_statement : DCL_FLOAT assign_statement | DCL_INT assign_statement
> expression : exp2 (CMP_OP exp2)
> exp2 : exp1 (MUL_DIV exp1)*
> exp1 : factor (ADD_SUB factor)*
> factor : OBJECT | LPAREN expression RPAREN | INTEGER | DCL_TYPE£¨expression£© | STRING | FLOAT

[ Grammar for jvav command line mode ]
> statement : declare_statement |assign_statement | single_statement | EMPTY | expression
> single_statement : OBJECT | INTEGER | expression (single_statement will print the token of the object | integer)
*/

class Token {
public:
	TokenType& type() { return mtype; }
	uint& value() { return mval; }
	Meta& data() { return mdata; }

	Token(TokenType type,uint val = 0):
	mtype(type),mval(val){}

	Token():mtype(INVAILD),mval(0){}
private:
	TokenType mtype;
	union {
		uint mval;
		Meta mdata;
	};
};

using op = Token(*)(Token, Token,MetaPool*);

class TokenSpliter  : public MetaPool {
public:
	void split_token(string str);
protected:
	Token get_next_token();

	bool peek_token(TokenType type, uint offset = 0) {
		if (token_counter + offset >= tokens.size())
			return false;
		return tokens[token_counter + offset].type() == type;
	}

	TokenType peek_token(uint offset = 0) { 
		if(token_counter + offset < tokens.size())
			return tokens[token_counter + offset].type();
		return INVAILD;
	}

	inline void eat(TokenType type,string error) {
		if (get_next_token().type() != type)
			throw string(error);
	}

	//Meta allocate_mem(uint size);
	//void release_mem();

	//vector<Meta> mems;
	vector<Token> tokens;
	int token_counter;
};



struct ASTNode {
	ASTNode(Token t) :token(t) {}
	Token token;
protected:
	//this dummy is useless except with this I can do dynamic cast on ast nodes.
	virtual void dynamic_dummy_func() {}
};

struct BinOp : public ASTNode {
	BinOp(Token t) :ASTNode(t),
		right(nullptr),left(nullptr) {}
	ASTNode* right;
	ASTNode* left;

	TokenType type() { return token.type(); }
	op opera();
};

struct Num : public ASTNode {
	Num(Token t) :ASTNode(t){}
	uint num() { return token.value(); }
};

struct StringNode : public ASTNode {
	StringNode(Token t) :ASTNode(t) {}
};

struct Empty : public ASTNode {
	Empty(Token t):ASTNode(t) {}
};

struct ObjectNode : public ASTNode {
	ObjectNode(Token t):ASTNode(t){}
};

struct Assign : public ASTNode {
	Assign(Token t):ASTNode(t){}
	ObjectNode* lnode;//assign expression's lhs must be a object
	ASTNode* rnode;//assign expresion's rhs must be a expression
};

struct StatementList : public ASTNode {
	StatementList(Token t) :ASTNode(t) {}
	vector<ASTNode*> statemens;
};

struct TypeConvert : public ASTNode {
	TypeConvert(Token t) :ASTNode(t){}
	ASTNode* target;
};

struct Branch : public ASTNode {
	Branch(Token t): ASTNode(t) {}
	ASTNode* expr;
	StatementList* if_node;
	StatementList* else_node;
};

struct Loop : public ASTNode {
	Loop(Token t):ASTNode(t) {}
	ASTNode* expr;
	StatementList* list;
};

class Parser : public TokenSpliter{
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

	ASTNode* statement();
	ASTNode* ndcl_statement();

	ASTNode* loop();
	ASTNode* branch();

	ASTNode* root;
};

template<typename T>
T* Parser::create_node(Token t) {
	uint node_size = sizeof(T);
	Meta mem = allocate(node_size);

	return new (mem.data) T(t);
}

class Interpreter : public MetaPool{
public:
	void program(ASTNode* node);

	void statement_list(StatementList* node);

	void statement(ASTNode* node);

	//expr will return a token in the future
	Token expr(ASTNode* node);
private:
	Token calcu(ASTNode* node);

	void assign(Assign* ass);

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