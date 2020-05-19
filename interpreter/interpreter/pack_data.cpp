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
	case '<':
		type = CMP_OP;
		return cmp_less;
	case '>':
		type = CMP_OP;
		return cmp_larger;
	case '%':
		type = MUL_DIV;
		return operat_perc;
	default:
		return nullptr;
	}
}

Meta pack_string(string& str,MetaPool* pool) {
	Meta data = pool->allocate(str.size() + 1);

	memcpy(data.data, str.c_str(), str.size());
	*((char*)data.data + str.size()) = '\0';
	return data;
}

string unpack_string(Meta& data) {
	string res = (char*)data.data;
	return res;
}

Token operat_plus(Token num1, Token num2, MetaPool* pool) {
	if (num1.type() != num2.type())
		throw string("operator + : fail to convert lhs type to rhs type");
	string lhs, rhs;
	Token t; Meta m;
	switch (num1.type())
	{
	case INTEGER:
		return Token(INTEGER, num1.value() + num2.value());
	case FLOAT:
		return Token(FLOAT,
			pack_float(
				unpack_float(num1.value()) +
				unpack_float(num2.value())
			)
		);
	case STRING:
		//to many waste!!
		lhs = unpack_string(num1.data());
		rhs = unpack_string(num2.data());
		lhs = lhs + rhs;
		m = pack_string(lhs,pool);
		t.data() = m;
		t.type() = STRING;
		break;
	default:
		throw string("operator + : don't support this type of operation");
	}
	return t;
}
Token operat_sub(Token num1, Token num2, MetaPool* pool) {
	if (num1.type() != num2.type())
		throw string("operator - : fail to convert lhs to rhs type ");
	switch (num2.type())
	{
	case INTEGER:
		return Token(INTEGER, num1.value() - num2.value());
	case FLOAT:
		return Token(FLOAT,
			pack_float(
				unpack_float(num1.value()) -
				unpack_float(num2.value())
			)
		);
	default:
		throw string("operator - : don't support this type of operation");
	}
}

Token operat_div(Token num1, Token num2, MetaPool* pool) {
	if (num1.type() != num2.type())
		throw string("operator / : fail to convert lhs to rhs type ");
	switch (num2.type())
	{
	case INTEGER:
		return Token(INTEGER, num1.value() / num2.value());
	case FLOAT:
		return Token(FLOAT,
			pack_float(
				unpack_float(num1.value()) /
				unpack_float(num2.value())
			)
	);
	default:
		throw string("operator / : don't support this type of operation");
	}
}

Token operat_mul(Token num1, Token num2, MetaPool* pool) {
	if (num1.type() != num2.type())
		throw string("operator * : fail to convert lhs to rhs type ");
	switch (num2.type())
	{
	case INTEGER:
		return Token(INTEGER, num1.value() * num2.value());
	case FLOAT:
		return Token(FLOAT, 
			pack_float(
				unpack_float(num1.value()) * 
				unpack_float(num2.value())
			)
		);
	default:
		throw string("operator * : don't support this type of operation");
	}
}

Token operat_perc(Token lhs,Token rhs,MetaPool* pool) {
	if (lhs.type() != rhs.type())
		throw string("operator % : fail to lhs to rhs ");
	else if (lhs.type() != INTEGER)
		throw string("operator % : operator % only support data type INTEGER");

	return Token(INTEGER,(int)lhs.value() % (int)rhs.value());
}

#define CMP_HELPER(lhs,rhs,opera)\
	if(lhs.type() != rhs.type()){\
		throw string("invaild less operator lhs's type must be the same as rhs's type");\
	}\
	switch(lhs.type()){\
	case INTEGER:\
		return Token(INTEGER,(int)lhs.value() opera (int)rhs.value());\
	case FLOAT:\
		return Token(INTEGER,unpack_float(lhs.value()) opera unpack_float(rhs.value()));\
	case STRING:\
		return Token(INTEGER,unpack_string(lhs.data()) opera unpack_string(rhs.data()));\
	}\
	throw string("operator : invaild type ,operator don't support");


// lhs < rhs ?
Token cmp_less(Token lhs, Token rhs, MetaPool* pool) {
	CMP_HELPER(lhs, rhs, < );
}

Token cmp_larger(Token lhs, Token rhs, MetaPool* pool) {
	CMP_HELPER(lhs,rhs,>);
}

Token cmp_equal(Token lhs,Token rhs,MetaPool* pool) {
	CMP_HELPER(lhs, rhs, == );
}

Token cmp_inequal(Token lhs,Token rhs,MetaPool* pool) {
	CMP_HELPER(lhs,rhs, != );
}