#pragma once
#include "Meta.h"
#include "Token.h"
#include "common.h"

Token cmp_less(Token lhs,Token rhs,MetaPool* pool);
Token cmp_larger(Token lhs, Token rhs, MetaPool* pool);
Token cmp_equal(Token lhs,Token rhs,MetaPool* pool);
Token cmp_inequal(Token lhs,Token rhs,MetaPool* pool);

Token operat_plus(Token num1, Token num2,MetaPool* pool);
Token operat_sub(Token num1, Token num2,MetaPool* pool);
Token operat_div(Token num1, Token num2,MetaPool* pool);
Token operat_mul(Token num1, Token num2,MetaPool* pool);

Token operat_perc(Token lhs,Token rhs,MetaPool* pool);

op get_operator(char c, TokenType& type);

inline op unpack_operator(uint val) {
	return *reinterpret_cast<op*>(&val);
}

inline uint pack_string_size(string str) { return str.size() + 1; }

//data's size must be str.size() + 1
Meta pack_string(string& str,MetaPool* pool);

string unpack_string(Meta& data);

inline uint pack_float(float f) {
	uint v = *(uint*)&f;
	return v;
}

inline float unpack_float(uint v) {
	float f = *(float*)&v;
	return f;
}
