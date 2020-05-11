#pragma once
#include "Meta.h"
#include "common.h"
#include "Object.h"

enum  TokenType {
	INVAILD = 0,//this type is invaild.
	INTEGER,//a constant integer number
	FLOAT,//a constant float number
	ADD_SUB,//add and sub operator
	MUL_DIV,//multiple and divide operator
	END,//end of a expression
	LPAREN,//(
	RPAREN,//)
	FIELD_START,//{ the start of a field
	FIELD_END,//} the end of a field
	ASSIGN,//= assign a value to a variable
	DCL_INT, //notations of spcifing a variable 
	DCL_FLOAT,//a variable float
	ENTRY, //the entry of the jvav
	OBJECT, //stands for a variable
	EMPTY //stands for no command in a line
};

string translate_reserved_word(TokenType Token);
TokenType translate_reserved_word(string word);

//jvav has two modes : command line mode and excute mode
/*

[ Grammar for jvav excute mode ]
> program : ENTRY FIELD_START END statement_list END FIELD_END
> statement_list : statement END statement_list
> END : \n
> statement : declare_statement | assign_statement | EMPTY
> EMPTY :
> assign_statement : OBJECT ASSIGN expression
> declare_statement : DCL_FLOAT assign_statement | DCL_INT assign_statement
> expression : exp1 (MUL_DIV exp1)*
> exp1 : factor (ADD_SUB factor)*
> factor : OBJECT | (LPAREN expression RPAREN) | INTEGER

[ Grammar for jvav command line mode ]
> statement : declare_statement |assign_statement | single_statement | EMPTY | expression
> single_statement : OBJECT | INTEGER | expression (single_statement will print the token of the object | integer)
*/

class Token {
public:
	TokenType type() { return mtype; }
	uint value() { return mval; }
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


class TokenSpliter {
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

	Meta allocate_mem(uint size);
	void release_mem();

	vector<Meta> mems;
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
	int num() { return token.value(); }
};

struct Empty : public ASTNode {
	Empty(Token t):ASTNode(t) {}
};

struct ObjectNode : public ASTNode {
	ObjectNode(Token t):ASTNode(t){}
	OBJECT_TYPE obj_type;
};

struct Assign : public ASTNode {
	Assign(Token t):ASTNode(t){}
	ObjectNode* lnode;//assign expression's lhs must be a object
	ASTNode* rnode;//assign expresion's rhs must be a expression
};

class Parser : public TokenSpliter{
public:
	ASTNode* get_AST_root() { return root; }

	void parse_line(string str);

	Parser() :root(nullptr) { mems.reserve(8); }

private:
	template<typename T>
	T* create_node(Token t);

	ASTNode* express2();
	ASTNode* express1();
	ASTNode* factor();

	ASTNode* empty();
	ASTNode* single_statement();
	ASTNode* assign_statement();
	ASTNode* declare_statement();

	ASTNode* statement();

	ASTNode* root;
};

template<typename T>
T* Parser::create_node(Token t) {
	uint node_size = sizeof(T);
	Meta mem = allocate_mem(node_size);

	return new (mem.data) T(t);
}

class Interpreter {
public:
	void statement(ASTNode* node);

	//expr will return a token in the future
	int expr(ASTNode* node);
private:
	int calcu(ASTNode* node);

	void assign(Assign* ass);
};

//JVM(JVav's Mother)
class JVM {
public:
	void command_line(string name);
	void excute();
private:
	Interpreter inter;
	Parser par;

};