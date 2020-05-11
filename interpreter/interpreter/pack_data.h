#pragma once
#include "Meta.h"
#include "Token.h"
#include "common.h"

int operat_plus(int num1, int num2);
int operat_sub(int num1, int num2);
int operat_div(int num1, int num2);
int operat_mul(int num1, int num2);

op get_operator(char c, TokenType& type);

inline op unpack_operator(uint val) {
	return *reinterpret_cast<op*>(&val);
}

inline uint pack_string_size(string str) { return str.size() + 1; }

//data's size must be str.size() + 1
void pack_string(string str,Meta& data);

string unpack_string(Meta& data);

uint pack_float(float f);
float unpack_float(uint v);
