#include "system_invoke.h"
#include "pack_data.h"
#include "Object.h"
#include "windows_wrapper.h"

map<string, void(*)()> map_sys_invoke = {
	{"oruga()",invoke_oruga},
	{"help()",invoke_help},
	{"clear()",invoke_clear},
	{"exit()",invoke_exit}
};

map<string, sys_par_invoke> map_sys_par_invoke = {
	{"sleep",invoke_sleep},
	{"input",invoke_input},
	{"clear",invoke_clear_par},
	{"exit",invoke_exit_par},
	{"random",invoke_random}
};

map<string, string> map_invoke_describe = {
	{"oruga","oruga : print a king of Mars on the screen(だからよ、止まるんじゃねぇぞ)\n"
	   "And print the version of jvav system (which will be 0.114.514 forever)\n"},
	{"help","help : introduce what's jvav and jvav's grammar birefly\n"},
	{"clear","clear up the console screen\n"}
	,{"exit","exit the jvav command line\n"}
};



const char* oruga =
"  n\n"
"mm |\n▏  n\n"
"▏  n\n"
"█▏　^^\n"
"█▏ {ww}\n█▏ {ww}\n"
"█◣▄██◣\n"
" ◥██████▋\n ◥██████▋▋\n"
"  ◥████ █▎\n"
"    ███▉ █▎\n    ███▉ █\n"
"  ◢████◣[m]\n"
"   ██◥█◣ \n   ██◥█◣\n"
"   ██　◥█◣\n"
"   █▉　　█▊\n   █▉　　█▊\n"
"   █▊　　█▊\n"
"   █▊　　█▋\n   █▊　　█▋\n"
"   █▏　　█▙\n"
"   █\n";

const char* jvav_str =
"|////////////////////////////////////////////////////////////////////////////////|\n"
"|********************************************************************************|\n"
"|********************************************************************************|\n"
"           ██████    ███        ███        █████████        ███          ███\n"
"            ████      ███        ███       ████   ████       ███         ███ \n"
"            ████       ███       ███      ████     ████       ███        ███ \n"
"            ████        ███      ███     ████       ████       ███       ███ \n"
"            ████         ███     ███     ████       ████        ███      ███ \n"
"            ████          ███    ███     ███████████          ███     ███ \n"
"            ████           ███   ███     ███████████            ███    ███ \n"
"            ████            ███  ███     ████       ████           ███   ███ \n"
"   ██      ████              ███ ███     ████       ████            ███ ███ \n"
"    ███████                   ██████     ████       ████            ██████  \n"
"|********************************************************************************|\n"
"|********************************************************************************|\n"
"|////////////////////////////////////////////////////////////////////////////////|\n"
;
const char* jvav_describe =
"This is a dialect of jvav : jvav sprite.For another version of jvav ,visit here\n https://github.com/TZG-official/Jvav  \n"
"welcome to the world of jvav!"
"Jvav ,created by Dr.ZHYis a technoloy which can help you construct software."
"(for some reason the name is represented by TDN name notation system for more  "
"about TDN name notation please reference \n https://zh.moegirl.org/真夏夜之淫梦) \n"
"Jvav can be applied to many fields like web development,video game development(such as minecraft),etc."
"One of the most amazing point of jvav is its portability , you don't have to change your jvav code to run on "
"different plantfrom! You can run jvav on laptop,smart phone,or even toilet!That's implemented by "
"Dr.ZHY's brillant concept : JVM(JVav's Mother 中译 jvav之母) With JVM jvav don't have to worry which "
"plantfrom is it running on and what interfaces does the plantfrom give us."
"Now we are working on making the jvav a Turing Complete language.As you can see,if you can simulate a "
"Turing Machine you can do anything!Which means that jvav will lead the fourth industrial revolution if"
"we finish our work.\n\n"
"Jvav sprite. Ojbkle tm \n\n""@ 2077 all rights reservered\n\n"

"input s for information about system function\n"
"input g for grammar introduction\n"
;

const char* command_line_grammar = 
"jvav sprite now has two modes command line and script\n"
"in command line, program will wait for user input codes and excute line by line\n"
"in command line one line is a <statement>,<statement> is the smallest unit to excute on jvav\n\n"
"<statement> can be a <declare statement>,<assign statement>,<expression>,<integer>,<nothing>\n\n"
"<nothing> is a empty line.jvav will do nothing literally\n\n"
"<integer> is a integer number.for exmaple | jvav :> 24\n\n"
"<object> is a variable to save the value ,we will talk about how to create object later\n\n"
"<expression> is a combintion of <object> and <integer> and <operator> like +-*/,for exmaple | jvav :>19 * 19 + 810\n"
"At present here are all the operator we support : '+' ,'-','*','/','<','>','==','!=','%'(only integer can use operator %)\n\n"
"<assign statement> looks like this : <object> = <expression> ,object's value will be expression's value\n\n"
"for example| jvav :> kmr = 114 - 514 + 810\n\n"
"<declare statement> a declare statement creates a <object> variable and assign a value to it,"
"it looks like this <declare word> <object> = <expression>,"
"<declare word> is a jvav system reversed word that tell the system the type of the <object>,"
"<declare word> now has <int> <float> two reservered words\n"
"for exmaple| jvav :> int mur = 1919 + 810\n\n"
"above are all the grammar jvav currently support"
"here is a exmaple program\n\n"
"jvav :>int a = 114514\n"
"jvav :>int b = 1919\n"
"jvav :>int c = a + b - 919\n"
"jvav :>c + 100000\n"
"215514\n\n"
;

const char* script_grammar =
"Meanwhile, except for the command line mode,jvav sprite also has the script mode which means that you can write "
"all your code in a file and let jvav sprite excute the codes in that file.\n\n"
"Excute the js program by , js [-options args...] [filename] to let the jvav sprite excute your code in script mode."
"At present no option is supported so you just need to input file name or nothing so that jvav will excute in command line mode\n\n"
"the extension name of the files which can be excuted by jvav sprite should be .jspr.For the files with other extension name,jvav script"
"will deny to excute.\n\n"
"In script files , the entry of a program is the <mian field> which start with word mian and followed by a <field>\n"
"<field> is a block of statements which started and ended with a pair of curly brackets '{' and '}'.You should start a new line"
" after '{' and '}' or you will get a syntax error.\n"
"example : \n"
"correct :				error:\n"
"		mian{			   mian{ statement1\n"
"			statement1			}\n"
"			}\n"
"In <field>s, you can put as many statements as you want in it.You should make sure every line only has one statement so "
"jvav sprite can divid the statements correctly.The <statment>'s grammars in command line are all supported in script mode"
"(include the <single statment>,you can simply put a factor in a line as a fast way to print it)\n"
"Despite that, you can do brach and loop by <branch statement> and <loop statement> in <fields>\n\n"
"<loop statement> includes a 'while' reservered word a <expression> and a <field>,which looks like this : while (<expression>) <field>"
"jvav sprite will excute statements in field for multiple times until the result of the expression is a INTEGER 0.You should be aware of that\n"
"the expression in loop statement should return a INTEGER result or it will produce a error\n\n"
"<branch statement> looks like this : if (<expression>) <field> [else <field>],like the <loop statement>,the expression's result's value "
"should be a INTEGER.the 'else' and the <field> after are alternative.Without that,when the expression return 0,jvav will simply skip the"
"whole statement,with that jvav will excute statements in <field> after 'else' when the expression returns 0.Of course jvav sprite will excute "
"statments in the <field> after 'if' if the expression return value is not 0\n\n"
"Example program about branch and loop is submitted to github : \n https://github.com/137900114/Jvav_Sprite \n\n"
;

const char* speical_grammar = "";

void invoke_oruga() {
	printf(oruga);
	printf("不要停下来啊\ndon't stop\n\n");
	printf("current jvav version (迫真): ver 0.114.514 \n\n");
}

char* str_organize(const char* str) {
	const static int line_num = 60;
	const static int line_length = 100;
	const static int buffer_size = line_num * (line_length + 2);
	static char buffer[buffer_size];
	memset(buffer, ' ', buffer_size);

	int x = 0, y = 0, is = 0;
	while (str[is] != '\0' && y != line_num) {
		if (str[is] == '\n') {
			x = 0, y++, is++;
			buffer[y * (line_length + 2) - 1] = '\n';
			continue;
		}
		buffer[y * (line_length + 2) + x++] = str[is++];
		if (x == line_length) {
			if (str[is] != '\0' && str[is] != ' ')
				buffer[y * (line_length + 2) + x] = '-';
			buffer[y * (line_length + 2) + x + 1] = '\n';
			x = 0, y++;
		}
	}
	buffer[y * (line_length + 2) + x] = '\0';

	return buffer;
}


void invoke_help() {
	invoke_clear();
	printf("\n\nwrite in jvav, run every where \n写JVAV 跑全世界\n");
	printf(jvav_str);
	printf(str_organize(jvav_describe));
	string in;
	getline(cin, in);
	invoke_clear();
	if (in == "g") {
		printf(str_organize(command_line_grammar));
		printf("input anything to continue\n");
		getline(cin, in);
		invoke_clear();
		printf(str_organize(script_grammar));
		printf("input anything to continue\n");
		getline(cin,in);
		invoke_clear();
	}
	else if (in == "s") {
		printf("here is all the system functions supported:\n");
		for (auto i : map_invoke_describe) {
			printf("%s\n", i.second.c_str());
		}
		printf("input anything to continue\n");
		getline(cin,in);
		invoke_clear();
	}
	
}

void invoke_clear() {
	system("cls");
}

sys_invoke check_sys_invoke(string name) {
	if (map_sys_invoke.find(name) == map_sys_invoke.end())
		return nullptr;
	return map_sys_invoke[name];
}

void invoke_exit() {
	exit(0);
}

sys_par_invoke check_sys_par_invoke(string name) {
	auto query = map_sys_par_invoke.find(name);
	if (query != map_sys_par_invoke.end()) {
		return query->second;
	}
	return nullptr;
}

//func sleep(float a)
void invoke_sleep(vector<Token>& par, MetaPool* pool) {
	if (par.size() != 1) throw string("wrong parameter number for sleep. 1 parameter expected");
	Object obj;
	switch (par[0].type()) {
	case TokenType::FLOAT:
		w_sleep(unpack_float(obj.value));
		return;
	case OBJECT:
		obj = ObjectPool::query_object(unpack_string(par[0].data()));
		if (obj.type == OBJECT_TYPE::FLOAT) {
			w_sleep(unpack_float(obj.value));
			return;
		}
	default:
		throw string("invaild parameter for sleep function ");
	}
}
//func input(ref str c)| func input(ref int a) | func input(ref float b)
void invoke_input(vector<Token>& par, MetaPool* pool) {
	if (par.size() != 1) throw string("wrong parameter number for input .1 parameter expected ");
	if (par[0].type() != OBJECT) throw string("wrong parameter type for input ,only object is acceptable");
	string id = unpack_string(par[0].data());
	Object obj = ObjectPool::query_object(id);
	string line;
	getline(cin,line);
	try {
		switch (obj.type) {
		case OBJECT_TYPE::INTEGER:
			ObjectPool::update_object(stoi(line),id);
			break;
		case OBJECT_TYPE::FLOAT:
			ObjectPool::update_object(pack_float(stof(line)), id);
			break;
		case OBJECT_TYPE::STRING:
			ObjectPool::update_dynamic_object(pack_string(line,pool),id);
			break;
		default:
			throw 0;
		}
	}
	catch (...) {
		throw string("invaild input : ") + line;
	}
}

void invoke_clear_par(vector<Token>& par, MetaPool* pool) {
	system("cls");
}

void invoke_exit_par(vector<Token>& par,MetaPool* pool) {
	exit(0);
}

//func random(ref float a)
void invoke_random(vector<Token>& par,MetaPool* pool) {
	static int seed = w_time();
	if (par.size() != 0 && par[0].type() != OBJECT)
		throw string("invaild parameter for random ");
	string id = unpack_string(par[0].data());
	Object obj = ObjectPool::query_object(id);
	float result = sin(seed++) * 1000;
	result = result - floor(result);
	switch (obj.type) {
	case OBJECT_TYPE::FLOAT:
		ObjectPool::update_object(pack_float(result), id);
		break;
	default:
		throw string("invaild parameter object type for random function");
	}
}