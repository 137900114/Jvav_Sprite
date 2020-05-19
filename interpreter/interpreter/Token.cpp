#include "Token.h"
#include "pack_data.h"
#include "system_invoke.h"

map<string, TokenType> reserved_words = {
	{"mian",ENTRY} , {"int",DCL_INT} , {"float",DCL_FLOAT},
	{"true",INTEGER},{"false",INTEGER},{"str",DCL_STRING},
	{"if",IF},{"else",ELSE},{"while",LOOP},{"func",DCL_FUNC},
	{"ref",REF}
};

TokenType translate_reserved_word(string name) {
	return reserved_words[name];
}

#pragma region TokenSpliter

void TokenSpliter::split_token(string str) {

	int current_i = 0;
	op p = nullptr;
	TokenType type;
	uint before = tokens.size();
	for (; current_i < str.size(); current_i++) {
		if (isdigit(str[current_i])) {
			int start = current_i;
			bool f_flag = false;
			while (current_i != str.size() && isdigit(str[current_i])) {
				current_i++;
				if (str[current_i] == '.' && !f_flag) {
					f_flag = true;
					current_i++;
				}
			}
			string num = str.substr(start,current_i - start);
			if (!f_flag)
				tokens.push_back(Token(INTEGER, stoi(num)));
			else
				tokens.push_back(Token(FLOAT, pack_float(stof(num))));
			current_i--;
		}
		else if (p = get_operator(str[current_i], type)) {
			tokens.push_back(Token(type, *(uint*)&p));
		}
		else if (isalpha(str[current_i])) {
			uint start = current_i;
			while (current_i != str.size() && 
				(isalpha(str[++current_i]) || str[current_i] == '_'));

			string word = str.substr(start,current_i - start);
			if (TokenType t = translate_reserved_word(word)) {
				if (word == "true") {
					tokens.push_back(Token(t, 1));
				}
				else if (word == "false") {
					tokens.push_back(Token(t, 0));
				}
				else {
					tokens.push_back(Token(t));
				}
			}
			else {
				//maybe we need to create a object pool later
				Meta mem = pack_string(word, this);
				Token obj(OBJECT);
				obj.data() = mem;
				tokens.push_back(obj);
			}
			current_i--;
		}
		else {
			Token t; Meta m;
			string str_buf;
			int start, end;
			switch (str[current_i]) {
			case '\t':
			case ' ':
				break;
			case '(':
				tokens.push_back(Token(LPAREN)); break;
			case ')':
				tokens.push_back(Token(RPAREN)); break;
			case '=':
				if (current_i + 1 != str.size() && str[current_i + 1] == '=') {
					t.type() = CMP_OP;
					p = cmp_equal;
					t.value() = *(uint*)&p;
					tokens.push_back(t);
					current_i++;
				}else 
					tokens.push_back(Token(ASSIGN));
				break;
			case '{':
				tokens.push_back(Token(FIELD_START)); break;
			case '}':
				tokens.push_back(Token(FIELD_END)); break;
			case '"':
			    end = 1,current_i++;
				while (true) {
					if (current_i  + end == str.size())
						throw string("invaild expression : a string must end with a quote ");
					if (str[current_i + end] == '"') break;
					end++;
				}
				str_buf = str.substr(current_i,end);
				m = pack_string(str_buf,this);
				t.type() = STRING;
				t.data() = m;
				tokens.push_back(t);
				current_i += end;
				break;
			case '!':
				if (current_i + 1 != str.size() && str[current_i + 1] == '=') {
					t.type() = CMP_OP;
					p = cmp_inequal;
					t.value() = *(uint*)&p;
					tokens.push_back(t);
					current_i++;
					break;
				}
			case '#':
				current_i = str.size();
				break;

			case ',':
				tokens.push_back(Token(COMMA));
				break;
			default:
				string error = "invaild charactor :";
				error += "in command" + str.substr(0, current_i) + "[" + str[current_i] + "]"
					+ str.substr(current_i + 1, str.size());
				throw error;
			}
		}
	}

	if (before == tokens.size())
		tokens.push_back(Token(EMPTY));

	tokens.push_back(Token(TokenType::END, 0));
}

Token TokenSpliter::get_next_token() {
	return tokens[token_counter++];
}
#pragma endregion

