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
	INVOKE,//A function object that can be called

	DCL_INT, //notations of spcifing a variable 
	DCL_FLOAT,//a variable float
	DCL_STRING,//a variable saves string(size will dynamicly change)
	DCL_FUNC,//delare a function object that can be excuted by jvav sprite
	REF,//indicate that a parameter of the function is a reference type
	COMMA,//commas between parameters that divides the parameters

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
> program : (function_declare_statement)* ENTRY FIELD_START END statement_list FIELD_END (function_declare_statement)*
> statement_list : ndcl_statement_list | declare_statment END statement_list
> branch : IF LPAREN expression RPAREN FIELD_START END ndcl_statement_list FIELD_END (ELSE FIELD START ndcl_statement_list FIELD_END) END
> ndcl_statement_list :  loop | branch | assign_statement | EMPTY END ndcl_statement_list
> loop : LOOP LPAREN expression RPAREN FIELD_START ndcl_statement_list FIELD_END END
> END : \n
> statement : declare_statement | assign_statement | EMPTY | invoke
> function_declare_statement : DCL_FUNC func_name LPAREN ((REF) DCL_TYPE object_name COMMA)* RPAREN field 
> invoke : object LPAAREN ( expression COMMA )* RPAREN

> EMPTY :
> assign_statement : OBJECT ASSIGN expression
> declare_statement : DCL_FLOAT | DCL_TYPE assign_statement
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

	vector<Token> tokens;
	int token_counter;
};
