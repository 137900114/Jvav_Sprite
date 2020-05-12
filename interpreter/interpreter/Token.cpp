#include "Token.h"
#include "pack_data.h"
#include "system_invoke.h"


#pragma region Type Translation

//------------------------可能需要改-----------------------------//
map<string, TokenType> reserved_words = {
	{"mian",ENTRY} , {"int",DCL_INT} , {"float",DCL_FLOAT},
	{"true",INTEGER},{"false",INTEGER},{"str",DCL_STRING}
};

map<TokenType, OBJECT_TYPE> dcl_type_2_object_type = {
	{DCL_INT,OBJECT_TYPE::INTEGER},{DCL_FLOAT,OBJECT_TYPE::FLOAT},
	{DCL_STRING,OBJECT_TYPE::STRING},{INVAILD,OBJECT_TYPE::INVAILD}
};

map<TokenType, TokenType> dcl_type_2_type = {
	{DCL_INT,INTEGER},{DCL_FLOAT,FLOAT},{DCL_STRING,STRING}
};

map<OBJECT_TYPE, TokenType> obj_2_token_type = {
	{OBJECT_TYPE::FLOAT,FLOAT},{OBJECT_TYPE::INTEGER,INTEGER},
	{OBJECT_TYPE::STRING,STRING}
};
//--------------------------------------------------------------//

TokenType obj_2_token(OBJECT_TYPE type) {
	auto query = obj_2_token_type.find(type);
	if (query == obj_2_token_type.end())
		return OBJECT;
	return query->second;
}

bool is_dcl_type(TokenType type) {
	return dcl_type_2_object_type.find(type) != dcl_type_2_object_type.end();
}

bool is_single_statement(TokenType type) {
	
	switch (type) {
	case FLOAT:
	case INTEGER:
	case OBJECT:
	case ADD_SUB:
	case MUL_DIV:
	case STRING:
		return true;
	}
	return false;
}

template<typename T>
Token to_meta_string(T val,MetaPool* pool) {
	string str = to_string(val);
	Meta mem = pack_string(str,pool);
	Token t(STRING);
	t.data() = mem;
	return t;
}


Token type_convert(Token from,TokenType to,MetaPool* pool) {
	string str_buf;
	Token t;
	switch (from.type()) {
	case INTEGER:
		switch (to) {
		case INTEGER:
			return from;
		case FLOAT:
			from = Token(FLOAT,pack_float((float)(int)from.value()));
			return from;
		case STRING:
			return to_meta_string((int)from.value(),pool);
		default:
			throw string("invaild type convert ");
		}
	case FLOAT:
		switch (to) {
		case INTEGER:
			from = Token(INTEGER,(int)unpack_float(from.value()));
			return from;
		case FLOAT:
			return from;
		case STRING:
			return to_meta_string(unpack_float(from.value()),pool);
		default:
			throw string("invaild type convert ");
		}
	case STRING:
		str_buf = unpack_string(from.data());
		try {
			switch (to) {
			case INTEGER:
				t.value() = stoi(str_buf);
				break;
			case FLOAT:
				t.value() = pack_float(stof(str_buf));
				break;
			case STRING:
				return from;
			default:
				throw 1;
			}
		}
		catch (...) {
			throw string("invaild convertion from string : " + str_buf);
		}
		t.type() = to;
		return t;
	default:
		throw string("invaild type convert ");
	}
}

TokenType translate_reserved_word(string name) {
	return reserved_words[name];
}

void type_promotion(Token& a,Token& b,MetaPool* pool) {
	switch (a.type()) {
	case FLOAT:
		switch (b.type()) {
		case INTEGER:
			b = Token(FLOAT,pack_float((float)(int)b.value()));
			return;
		case STRING:
			a = type_convert(a, STRING, pool);
			return;
		default:
			return;
		}
	case INTEGER:
		switch (b.type()) {
		case FLOAT:
			a = Token(FLOAT, pack_float((float)(int)a.value()));
			return;
		case STRING:
			a = type_convert(a,STRING,pool);
			return;
		default:
			return;
		}

	case STRING:
		switch (b.type()) {
		case INTEGER:
			b = type_convert(b,STRING,pool);
			return;
		case FLOAT:
			b = type_convert(b,STRING,pool);
			return;
		default:
			return;
		}
	}
}

#pragma endregion

#pragma region TokenSpliter

void TokenSpliter::split_token(string str) {

	int current_i = 0;
	op p = nullptr;
	TokenType type;
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
				tokens.push_back(Token(ASSIGN)); break;
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
			default:
				string error = "invaild charactor :";
				error += "in command" + str.substr(0, current_i) + "[" + str[current_i] + "]"
					+ str.substr(current_i + 1, str.size());
				throw error;
			}
		}
	}

	if (str.empty())
		tokens.push_back(Token(EMPTY));

	tokens.push_back(Token(TokenType::END, 0));
}

Token TokenSpliter::get_next_token() {
	return tokens[token_counter++];
}
#pragma endregion

#pragma region Parser

void Parser::reset() {
	token_counter = 0;
	tokens.clear();

	root = nullptr;
	releaseAll();
}

void Parser::parse_line(string str) {

	split_token(str);
	root = statement();
}

ASTNode* Parser::express2() {
	ASTNode* node = express1();

	while (peek_token(ADD_SUB)) {
		Token t = get_next_token();
		BinOp* p = create_node<BinOp>(t);

		p->left = node;
		p->right = express1();
		node = p;
	}

	return node;
}

ASTNode* Parser::express1() {
	ASTNode* node = factor();
	while (peek_token(MUL_DIV)) {
		Token t = get_next_token();
		BinOp* p = create_node<BinOp>(t);

		p->left = node;
		p->right = factor();
		node = p;
	}
	return node;
}

//factor : OBJECT | (LPAREN expression RPAREN) | INTEGER | DCL_TYPE LPAREN expression RPAREN
ASTNode* Parser::factor() {

	if (is_dcl_type(peek_token())) {
		Token type = get_next_token();
		eat(LPAREN, "invaild expression : type convert should start with left paren ");
		ASTNode* exp = express2();
		eat(RPAREN, "invaild expression : type convert should end with right paren ");

		TypeConvert* conv = create_node<TypeConvert>(type);
		conv->target = exp;
		return conv;
	}

	ASTNode* node;

	switch (peek_token())
	{
	case STRING:
		node = create_node<StringNode>(get_next_token());
		return node;
	case FLOAT:
	case INTEGER:
		node = create_node<Num>(get_next_token());
		return node;
	case LPAREN:
		get_next_token();
		node = express2();
		if (get_next_token().type() == RPAREN)
			return node;
	case OBJECT:
		return create_node<ObjectNode>(get_next_token());

	default:
		throw string("invaild expression");
	}
}

//statement : declare_statement |assign_statement | single_statement | EMPTY 
ASTNode* Parser::statement() {
	ASTNode* result = nullptr;
	if (is_dcl_type(peek_token())) {
		result = declare_statement();
	}
	else {
		switch (peek_token()) {
		case EMPTY:
			result =  empty();
			get_next_token();
			break;
		case OBJECT:
			switch (peek_token(1)) {
			case ASSIGN:
				result = assign_statement();
				break;
			default:
				result = single_statement();
			}
			break;
		default:
			result = single_statement();
		}
	}
	eat(END,"invaild statement : statement don't end with empty");
	return result;
}


// declare_statement : DECLARE assign_statement
ASTNode* Parser::declare_statement() {
	TokenType type = get_next_token().type();
	string id = unpack_string(tokens[token_counter].data());
	ASTNode* node = assign_statement();
	
	ObjectPool::declare(id, dcl_type_2_object_type[type]);
	return node;
}

//assign_statement : OBJECT ASSIGN expression
ASTNode* Parser::assign_statement() {
	Token obj = get_next_token();
	if (obj.type() != OBJECT) {
		throw string("invaild assign expression : object must be the start ");
	}
	eat(ASSIGN, "invaild declare expression : object must be followed with a assign signal");
	ObjectNode* lhs = create_node<ObjectNode>(obj);
	ASTNode* rhs = express2();
	Assign* ass_node = create_node<Assign>(Token(ASSIGN));
	ass_node->rnode = rhs;
	ass_node->lnode = lhs;
	return ass_node;
}

//single_statement : OBJECT | INTEGER | expression 
ASTNode* Parser::single_statement() {
	return express2();
}

ASTNode* Parser::empty() {
	return create_node<Empty>(Token(EMPTY));
}

op BinOp::opera() {
	return unpack_operator(token.value());
}

void Parser::parse_program(vector<string>& prog) {

	for (auto line : prog) {
		split_token(line);
	}
	
	eat(ENTRY,"Expect Entry at the start of the program");
	eat(FIELD_START,"Expect field start after entry ");
	eat(END, "You should start new line after field_start");

	StatementList* slist = create_node<StatementList>(Token(MAIN_FIELD));
	while (!peek_token(FIELD_END)) {
		slist->statemens.push_back(statement());
	}

	root = slist;

}

#pragma endregion

#pragma region Interpreter

int Interpreter::cmd_counter;

Token Interpreter::expr(ASTNode* node) {

	return calcu(node);
}

Token unpack_Iobject_2_token(IndexedObject& obj) {
	Meta m = {obj.access_data(),obj.size()};
	Token t(obj_2_token(obj.type));
	t.data() = m;
	return t;
}

Token Interpreter::calcu(ASTNode* node) {
	BinOp* opera = nullptr;
	ObjectNode* obj = nullptr;
	Object obj_d;
	Token lhs, rhs;

	TokenType type;
	if (is_dcl_type(type = node->token.type())) {
		TypeConvert* conv = dynamic_cast<TypeConvert*>(node);
		TokenType target_type = dcl_type_2_type[conv->token.type()];
		return type_convert(calcu(conv->target),target_type,this);
	}

	switch (node->token.type())
	{
	case FLOAT:
	case INTEGER:
	case STRING:
		return node->token;
	case ADD_SUB:
	case MUL_DIV:
		opera = dynamic_cast<BinOp*>(node);
		lhs = calcu(opera->left);
		rhs = calcu(opera->right);
		type_promotion(lhs,rhs,this);
		return opera->opera()(lhs,rhs,this);
	case OBJECT:
		obj = dynamic_cast<ObjectNode*>(node);
		obj_d = ObjectPool::query_object(unpack_string(obj->token.data()));
		switch (obj_d.type) {
		case OBJECT_TYPE::INTEGER:
			return Token(INTEGER,obj_d.value);
		case OBJECT_TYPE::FLOAT:
			return Token(FLOAT,obj_d.value);
		case OBJECT_TYPE::STRING:
			return unpack_Iobject_2_token(
				IndexedObject::cast_2_indexed_object(obj_d)
			);
		default:
			return obj->token;
		}
	default:
		throw string("invaild token type in expression which is uncalcuable");
	}
}

void Interpreter::statement(ASTNode* root) {
	cmd_counter++;
	if (cmd_counter == release_cycle) {
		cmd_counter = 0;
		releaseAll();
	}

	if (is_single_statement(root->token.type())) {
		if (root->token.type() == OBJECT) {
			string id = unpack_string(root->token.data());
			cout<<  ObjectPool::query_object(id);
			return;
		}
		
		Token v = expr(root);
		switch (v.type()) {
		case FLOAT:
			cout << unpack_float(v.value());
			break;
		case INTEGER:
			cout << (int)v.value();
			break;
		case STRING:
			cout << unpack_string(v.data());
		}
		cout << endl;
		return;
	}

	Assign* assign_node;
	switch (root->token.type())
	{
	case EMPTY:
		break;
	case ASSIGN:
		assign_node = dynamic_cast<Assign*>(root);
		assign(assign_node);
		break;
	default:
		throw string("invaild syntax ");
	}
}

//currnetly we only support integer so we don't consider other object types
void Interpreter::assign(Assign* ass) {
	Token lhs_token = ass->lnode->token;
	string id = unpack_string(lhs_token.data());
	
	Token rhs = expr(ass->rnode);

	Object obj = ObjectPool::query_object(id);
	rhs = type_convert(rhs,obj_2_token(obj.type),this);
	if(Under4Bytes(obj.type))
		ObjectPool::update_object(rhs.value(), id);
	else {
		switch (obj.type) {
		case OBJECT_TYPE::STRING:
			if (rhs.type() != STRING)
				throw("invaild assign operation : fail to convert to string");
			ObjectPool::update_dynamic_object(rhs.data(),obj.name);
			break;
		default:
			throw("invaild assign operation : invaild object type ");
		}
	}

}


void Interpreter::program(ASTNode* node) {
	if (node->token.type() != MAIN_FIELD)
		throw string("a program must have a main field");
	StatementList* lis = dynamic_cast<StatementList*>(node);
	
	for (auto n : lis->statemens)
		statement(n);

}


#pragma endregion

void JVM::command_line(string line) {
	sys_invoke f;
	if (f = check_sys_invoke(line)) {
		f(); return;
	}

	par.reset();
	par.parse_line(line);
	inter.statement(par.get_AST_root());
}

void JVM::excute(vector<string>& lines) {
	par.reset();
	par.parse_program(lines);
	inter.program(par.get_AST_root());
}


