#include "pack_data.h"

op get_operator(char c, TokenType& type) {
	switch (c) {
	case '+':
		type = ADD_SUB;
		return operat_plus;
	case '-':
		type = ADD_SUB;
		return operat_sub;
	case '/':
		type = MUL_DIV;
		return operat_div;
	case '*':
		type = MUL_DIV;
		return operat_mul;
	default:
		return nullptr;
	}
}

void pack_string(string str,Meta& data) {
	memcpy(data.data, str.c_str(), str.size());
	*((char*)data.data + str.size()) = '\0';
}

string unpack_string(Meta& data) {
	string res = (char*)data.data;
	return res;
}

int operat_plus(int num1, int num2) {
	return num1 + num2;
}
int operat_sub(int num1, int num2) {
	return num1 - num2;
}

int operat_div(int num1, int num2) {
	return num1 / num2;
}

int operat_mul(int num1, int num2) {
	return num1 * num2;
}

uint pack_float(float f) {
	uint v = *(uint*)&f;
	return v;
}
float unpack_float(uint v) {
	float f = *(float*)&v;
	return f;
}
