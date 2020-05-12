#pragma once
#include "Meta.h"
#include "Token.h"
#include "common.h"


Token operat_plus(Token num1, Token num2,MetaPool* pool);
Token operat_sub(Token num1, Token num2,MetaPool* pool);
Token operat_div(Token num1, Token num2,MetaPool* pool);
Token operat_mul(Token num1, Token num2,MetaPool* pool);

op get_operator(char c, TokenType& type);

inline op unpack_operator(uint val) {
	return *reinterpret_cast<op*>(&val);
}

inline uint pack_string_size(string str) { return str.size() + 1; }

//data's size must be str.size() + 1
Meta pack_string(string& str,MetaPool* pool);

string unpack_string(Meta& data);

uint pack_float(float f);
float unpack_float(uint v);
