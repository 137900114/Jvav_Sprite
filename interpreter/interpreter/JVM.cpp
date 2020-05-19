#include "JVM.h"
#include "pack_data.h"
#include "system_invoke.h"

#pragma region Type Translation

//------------------------Need to optimize-----------------------------//

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
//----------------------------------------------------------------------//

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
	case CMP_OP:
	case STRING:
		return true;
	}
	return false;
}

template<typename T>
Token to_meta_string(T val, MetaPool* pool) {
	string str = to_string(val);
	Meta mem = pack_string(str, pool);
	Token t(STRING);
	t.data() = mem;
	return t;
}


Token type_convert(Token from, TokenType to, MetaPool* pool) {
	string str_buf;
	Token t;
	switch (from.type()) {
	case INTEGER:
		switch (to) {
		case INTEGER:
			return from;
		case FLOAT:
			from = Token(FLOAT, pack_float((float)(int)from.value()));
			return from;
		case STRING:
			return to_meta_string((int)from.value(), pool);
		default:
			throw string("invaild type convert ");
		}
	case FLOAT:
		switch (to) {
		case INTEGER:
			from = Token(INTEGER, (int)unpack_float(from.value()));
			return from;
		case FLOAT:
			return from;
		case STRING:
			return to_meta_string(unpack_float(from.value()), pool);
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

void type_promotion(Token& a, Token& b, MetaPool* pool) {
	switch (a.type()) {
	case FLOAT:
		switch (b.type()) {
		case INTEGER:
			b = Token(FLOAT, pack_float((float)(int)b.value()));
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
			a = type_convert(a, STRING, pool);
			return;
		default:
			return;
		}

	case STRING:
		switch (b.type()) {
		case INTEGER:
			b = type_convert(b, STRING, pool);
			return;
		case FLOAT:
			b = type_convert(b, STRING, pool);
			return;
		default:
			return;
		}
	}
}

bool is_field(TokenType type) {
	switch (type) {
	case FIELD_START:
	case MAIN_FIELD:
		return true;
	}
	return false;
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

ASTNode* Parser::express3() {
	ASTNode* node = express2();
	if (peek_token(CMP_OP)) {
		BinOp* p = create_node<BinOp>(get_next_token());

		p->left = node;
		p->right = express2();
		node = p;
	}
	return node;
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
		ASTNode* exp = expression();
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
		node = expression();
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
		return ndcl_statement();
	}
	eat(END, "invaild statement : statement don't end with empty");
	return result;
}


// declare_statement : DECLARE assign_statement
ASTNode* Parser::declare_statement() {
	TokenType type = get_next_token().type();
	if (!peek_token(OBJECT))
		throw string("in a declare statement a declare sign must be followed by a assign statment");
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
	ASTNode* rhs = expression();
	Assign* ass_node = create_node<Assign>(Token(ASSIGN));
	ass_node->rnode = rhs;
	ass_node->lnode = lhs;
	return ass_node;
}

//single_statement : OBJECT | INTEGER | expression 
ASTNode* Parser::single_statement() {
	return expression();
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
	
	while (peek_token(DCL_FUNC) || peek_token(EMPTY)) {
		FuncSignature sig;
		if (peek_token(EMPTY)) {
			get_next_token();
			get_next_token();
			continue;
		}
		ASTNode* func = function_declare_statement(sig);
		sig.func = func;
		ObjectPool::define_func(sig);
	}

	eat(ENTRY, "Expect Entry at the start of the program");
	eat(FIELD_START, "Expect field start after entry ");
	eat(END, "You should start new line after field_start");

	StatementList* slist = create_node<StatementList>(Token(MAIN_FIELD));

	while (!peek_token(FIELD_END)) {
		if (peek_token(IF)) {
			slist->statemens.push_back(branch());
		}
		else
			slist->statemens.push_back(statement());
	}

	while (peek_token(DCL_FUNC) || peek_token(EMPTY)) {
		FuncSignature sig;
		if (peek_token(EMPTY)) {
			get_next_token();
			get_next_token();
			continue;
		}
		ASTNode* func = function_declare_statement(sig);
		sig.func = func;
		ObjectPool::define_func(sig);
	}

	root = slist;
}

ASTNode* Parser::branch() {
	eat(IF, "a branch statement must start with if!!!");
	eat(LPAREN, "after if should be a expression start with a (");
	ASTNode* expr = expression();
	eat(RPAREN, "after a expression should be a ) ");
	eat(FIELD_START, "a field start is required");
	eat(END, "you should start an other line for after a if declare");
	StatementList* if_node = create_node<StatementList>(Token(FIELD_START));
	StatementList* else_node = nullptr;
	while (!peek_token(FIELD_END)) {
		if_node->statemens.push_back(ndcl_statement());
	}
	get_next_token();
	if (peek_token(ELSE)) {
		get_next_token();
		eat(FIELD_START, "a field start is required ");
		eat(END, "you should start an other line after else declare ");
		else_node = create_node<StatementList>(Token(FIELD_START));
		while (!peek_token(FIELD_END)) {
			else_node->statemens.push_back(ndcl_statement());
		}
		get_next_token();
	}
	eat(END, "a if expression must end with a END ");
	Branch* node = create_node<Branch>(Token(IF));
	node->if_node = if_node;
	node->else_node = else_node;
	node->expr = expr;
	return node;
}

ASTNode* Parser::ndcl_statement() {
	ASTNode* result;
	if (is_dcl_type(peek_token())) {
		throw string("you shouldn't start with a declare type in a undeclarable ");
	}
	switch (peek_token()) {
	case EMPTY:
		result = empty();
		get_next_token();
		break;
	case OBJECT:
		switch (peek_token(1)) {
		case ASSIGN:
			result = assign_statement();
			break;
		case LPAREN:
			result = invoke();
			break;
		default:
			result = single_statement();
		}
		break;
	case IF:
		return branch();
	case LOOP:
		return loop();
	default:
		result = single_statement();
	}
	eat(END, "invaild statement : statement don't end with empty");
	return result;
}


//loop : LOOP LPAREN expression RPAREN FIELD_START ndcl_statement_list FIELD_END END
ASTNode* Parser::loop() {
	eat(LOOP, "loop statement should start with loop symbol");
	eat(LPAREN, "loop statement should has a left paren after loop symbol ");
	ASTNode* node = expression();
	eat(RPAREN, "can't find loop statement's expression's right paren ");
	eat(FIELD_START, "after expression loop should start with a field start");
	eat(END, "loop's expression should start a new line");
	StatementList* list = create_node<StatementList>(FIELD_START);
	while (!peek_token(FIELD_END)) {
		list->statemens.push_back(ndcl_statement());
	}
	get_next_token();
	eat(END, "after a loop statement,you should start a new line ");

	Loop* loop_node = create_node<Loop>(Token(LOOP));
	loop_node->list = list;
	loop_node->expr = node;
	return loop_node;
}

//function_declare_statement : DCL_FUNC func_name LPAREN ((REF) DCL_TYPE object_name COMMA)* RPAREN field 
ASTNode* Parser::function_declare_statement(FuncSignature& sig) {
	eat(DCL_FUNC, "declaration of a function should start with a 'func' ");
	Token func_obj = get_next_token();
	sig.name = unpack_string(func_obj.data());
	eat(LPAREN, "the start of the parameter list should start with a left paren");
	while (!peek_token(RPAREN)) {
		bool ref_flag = false;
		if (peek_token(REF)) {
			ref_flag = true;
			get_next_token();
		}
		OBJECT_TYPE type = dcl_type_2_object_type[get_next_token().type()];
		if (!peek_token(OBJECT)) throw string("a parameter name is expected after a declaration ");
		string obj_name = unpack_string(get_next_token().data());
		uint obj_type = ref_flag ? pack_param_type(OBJECT_TYPE::REF,type) : pack_param_type(type);
		sig.ParamNames.push_back(obj_name);
		sig.ParamTypes.push_back(obj_type);
		if (peek_token(COMMA)) get_next_token();
	}
	get_next_token();
	eat(FIELD_START,"expect a field after a parameter list ");
	eat(END, "expect a end of a line after a field start ");
	StatementList* func = create_node<StatementList>(Token(FIELD_START));
	while (!peek_token(FIELD_END)) {
		func->statemens.push_back(function_statement(sig));
	}
	eat(FIELD_END, "expect a field end after a function");
	eat(END,"expect a end of line after a function ");
	sig.func = func;
	return func;
}

ASTNode* Parser::function_statement(FuncSignature& sig) {
	if (is_dcl_type(peek_token())) {
		return declare_in_func(sig);
	}
	return ndcl_statement();
}

ASTNode* Parser::declare_in_func(FuncSignature& sig) {
	sig.DeclareType.push_back(dcl_type_2_object_type[get_next_token().type()]);
	Token obj_t = tokens[token_counter];
	if (obj_t.type() != OBJECT) throw string("a object name is expected after a declaration ");
	sig.DeclareName.push_back(unpack_string(obj_t.data()));
	ASTNode* node = assign_statement();

	eat(END, "a end of line is expected after a statement ");
	return node;
}

ASTNode* Parser::invoke() {
	Token inv = get_next_token();
	inv.type() = INVOKE;
	eat(LPAREN,"a invoke should start with a left paren");
	Invoke* node = create_node<Invoke>(inv);
	while (!peek_token(RPAREN)) {
		if (peek_token(OBJECT) && (peek_token(COMMA,1) || (peek_token(RPAREN,1)))) {
			node->params.push_back(create_node<ObjectNode>(get_next_token()));
		}
		else {
			node->params.push_back(expression());
		}
		if (peek_token(COMMA)) get_next_token();
	}
	get_next_token();
	return node;
}

#pragma endregion

#pragma region Interpreter

int Interpreter::cmd_counter;

Token Interpreter::expr(ASTNode* node) {

	return calcu(node);
}

Token unpack_Iobject_2_token(IndexedObject& obj) {
	Meta m = { obj.access_data(),obj.size() };
	Token t(obj_2_token(obj.type));
	t.data() = m;
	return t;
}

Token Interpreter::calcu(ASTNode* node) {
	BinOp* opera = nullptr;
	ObjectNode* obj = nullptr;
	Object obj_d;
	Token lhs, rhs;

	uint i;

	TokenType type;
	if (is_dcl_type(type = node->token.type())) {
		TypeConvert* conv = dynamic_cast<TypeConvert*>(node);
		TokenType target_type = dcl_type_2_type[conv->token.type()];
		return type_convert(calcu(conv->target), target_type, this);
	}

	switch (node->token.type())
	{
	case FLOAT:
	case INTEGER:
	case STRING:
		return node->token;
	case ADD_SUB:
	case MUL_DIV:
	case CMP_OP:
		opera = dynamic_cast<BinOp*>(node);
		lhs = calcu(opera->left);
		rhs = calcu(opera->right);
		type_promotion(lhs, rhs, this);
		return opera->opera()(lhs, rhs, this);
	case OBJECT:
		obj = dynamic_cast<ObjectNode*>(node);
		obj_d = ObjectPool::query_object(unpack_string(obj->token.data()));
		switch (obj_d.type) {
		case OBJECT_TYPE::INTEGER:
			return Token(INTEGER, obj_d.value);
		case OBJECT_TYPE::FLOAT:
			return Token(FLOAT, obj_d.value);
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
			cout << ObjectPool::query_object(id);
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
	case INVOKE:
		call(dynamic_cast<Invoke*>(root));
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
	rhs = type_convert(rhs, obj_2_token(obj.type), this);
	if (Under4Bytes(obj.type))
		ObjectPool::update_object(rhs.value(), id);
	else {
		switch (obj.type) {
		case OBJECT_TYPE::STRING:
			if (rhs.type() != STRING)
				throw("invaild assign operation : fail to convert to string");
			ObjectPool::update_dynamic_object(rhs.data(), id);
			break;
		default:
			throw("invaild assign operation : invaild object type ");
		}
	}

}


void Interpreter::statement_list(StatementList* node) {
	if (!is_field(node->token.type()))
		throw string("a program must have a field");

	for (auto n : node->statemens) {
		if (n->token.type() == LOOP) {
			Loop* list = dynamic_cast<Loop*>(n);
			while (true) {
				Token val = expr(list->expr);
				if (val.type() != INTEGER)
					val = type_convert(val, INTEGER, this);
				if (val.value() == 0)
					break;
				statement_list(list->list);
			}
		}
		else if (n->token.type() == IF) {
			Branch* branch_node = dynamic_cast<Branch*>(n);
			Token val = expr(branch_node->expr);
			if (val.type() != INTEGER)
				val = type_convert(val, INTEGER, this);
			if ((int)val.value() != 0)
				statement_list(branch_node->if_node);
			else if (branch_node->else_node)
				statement_list(branch_node->else_node);
		}
		else
			statement(n);
	}

}

void Interpreter::program(ASTNode* node) {
	if (node->token.type() != MAIN_FIELD)
		throw string("a program must have a mian field");
	statement_list(dynamic_cast<StatementList*>(node));
}

void Interpreter::call(Invoke* inv) {
	string func_name = unpack_string(inv->token.data());
	vector<Token> params;
	for (ASTNode* node : inv->params) {
		if (node->token.type() == OBJECT) {
			params.push_back(node->token);
		}
		else
			params.push_back(expr(node));
	}
	ASTNode* func = ObjectPool::invoke_func(func_name,params);
	if (func) {
		statement_list(dynamic_cast<StatementList*>(func));
		ObjectPool::end_invoke();
	}
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

