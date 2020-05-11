#include "Token.h"
#include "pack_data.h"
#include "system_invoke.h"

map<string, TokenType> reserved_words = {
	{"mian",ENTRY},{"int",DCL_INT},{"float",DCL_FLOAT}
};

map<TokenType, OBJECT_TYPE> dcl_type = {
	{DCL_INT,OBJECT_TYPE::INTEGER},{DCL_FLOAT,OBJECT_TYPE::FLOAT},{INVAILD,OBJECT_TYPE::INVAILD}
};

bool is_dcl_type(TokenType type) {
	return dcl_type.find(type) != dcl_type.end();
}

bool is_single_statement(TokenType type) {
	switch (type) {
	case INTEGER:
	case OBJECT:
	case ADD_SUB:
	case MUL_DIV:
		return true;
	}
	return false;
}

TokenType translate_reserved_word(string name) {
	return reserved_words[name];
}

Meta TokenSpliter::allocate_mem(uint size) {
	Meta mem = MetaAllocator::allocate(size);
	mems.push_back(mem);
	return mem;
}

void TokenSpliter::release_mem() {
	for (auto m : mems) {
		MetaAllocator::deallocate(m);
	}
	mems.clear();
}

void TokenSpliter::split_token(string str) {
	token_counter = 0;
	tokens.clear();
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
				tokens.push_back(Token(t));
			}
			else {
				//maybe we need to create a object pool later
				Meta mem = allocate_mem(pack_string_size(word));
				pack_string(word, mem);
				Token obj(OBJECT);
				obj.data() = mem;
				tokens.push_back(obj);
			}
			current_i--;
		}
		else {
			switch (str[current_i]) {
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
			default:
				string error = "invaild expression ";
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

void Parser::parse_line(string str) {
	root = nullptr;
	release_mem();

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

ASTNode* Parser::factor() {

	ASTNode* node;
	switch (peek_token())
	{
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
	if (is_dcl_type(peek_token()))
		return declare_statement();
	switch (peek_token()) {
	case EMPTY:
		return empty();
	case OBJECT:
		switch (peek_token(1)) {
		case ASSIGN:
			return assign_statement();
		}
	}
	return single_statement();
}


// declare_statement : DECLARE assign_statement
ASTNode* Parser::declare_statement() {
	TokenType type = get_next_token().type();
	string id = unpack_string(tokens[token_counter].data());
	ASTNode* node = assign_statement();
	ObjectPool::declare(id, dcl_type[type]);
	return node;
}

//assign_statement : OBJECT ASSIGN expression
ASTNode* Parser::assign_statement() {
	Token obj = get_next_token();
	if (obj.type() != OBJECT)
		throw string("invaild assign expression,the left hand side must be a object");
	if (get_next_token().type() != ASSIGN)
		throw string("invaild declare expression : object must be followed with a assign signal");
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

int Interpreter::expr(ASTNode* node) {

	return calcu(node);
}

int Interpreter::calcu(ASTNode* node) {
	BinOp* opera = nullptr;
	ObjectNode* obj = nullptr;
	Object obj_d;
	int lhs, rhs;

	switch (node->token.type())
	{
	case INTEGER:
		return node->token.value();
	case ADD_SUB:
	case MUL_DIV:
		opera = dynamic_cast<BinOp*>(node);
		lhs = calcu(opera->left);
		rhs = calcu(opera->right);
		return opera->opera()(lhs,rhs);
	case OBJECT:
		obj = dynamic_cast<ObjectNode*>(node);
		obj_d = ObjectPool::query_object(unpack_string(obj->token.data()));
		switch (obj_d.type) {
		case OBJECT_TYPE::INTEGER:
			return obj_d.value;
		default:
			throw string("expression don't support this object type ");
		}
	default:
		throw string("invaild token type in expression which is uncalcuable");
	}
}

void Interpreter::statement(ASTNode* root) {
	if (is_single_statement(root->token.type())) {
		printf("%d \n",expr(root));
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
	
	int rhs = expr(ass->rnode);
	
	ObjectPool::update_object(rhs, id);
}

void JVM::command_line(string line) {
	sys_invoke f;
	if (f = check_sys_invoke(line)) {
		f(); return;
	}
	par.parse_line(line);
	inter.statement(par.get_AST_root());
}