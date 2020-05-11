#include "system_invoke.h"

map<string, void(*)()> map_sys_invoke = {
	{"oruga()",invoke_oruga},
	{"help()",invoke_help},
	{"clear()",invoke_clear},
	{"exit()",invoke_exit}
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
"This is a dialect of jvav : jvav sprite.For another version of jvav ,visite here :https://github.com/TZG-official/Jvav  \n"
"welcome to the world of jvav!"
"Jvav ,created by Dr.ZHYis a technoloy which can help you construct software.\n"
"(for some reason the name is represented by TDN name notation system for more  \n"
"about TDN name notation please reference https://zh.moegirl.org/真夏夜之淫梦) \n\n"
"Jvav can be applied to many fields like web development,video game development(such as minecraft),etc.\n"
"One of the most amazing point of jvav is its portability , you don't have to change your jvav code to run on\n"
"different plantfrom! You can run jvav on laptop,smart phone,or even toilet!That's implemented by    \n"
"Dr.ZHY's brillant concept : JVM(JVav's Mother 中译 jvav之母) With JVM jvav don't have to worry which \n"
"plantfrom is it running on and what interfaces does the plantfrom give us.\n\n"
"Now we are working on making the jvav a Turing Complete language.As you can see,if you can simulate a \n"
"Turing Machine you can do anything!Which means that jvav will lead the fourth industrial revolution if\n"
"we finish our work.\n\n"
"Jvav sprite. Ojbkle tm \n""@ 2077 all rights reservered"
"\n\n\n\n"
"input s for information about system function\n"
"input g for grammar introduction\n"
;

const char* grammar_describe = 
"At present jvav's grammar is not completed . Now it can only run on command line.\n"
"in command line a line is a <statement>,<statement> is the smallest unit to excute on jvav\n"
"<statement> can be a <declare statement>,<assign statement>,<expression>,<integer>,<nothing>\n"
"<nothing> is a empty line.jvav will do nothing literally\n"
"<integer> is a integer number.for exmaple | jvav :> 24\n"
"<object> is a variable to save the value of compute,we will talk about how to create object later\n"
"<expression> is a combintion of <object> and <integer> and <operator> like +-*/,for exmaple | jvav :>19 * 19 + 810\n"
"<assign statement> looks like this : <object> = <expression> ,object's value will\n be expression's value"
"for example| jvav :> kmr = 114 - 514 + 810\n"
"<declare statement> a declare statement creates a <object> variable and assign a value to it\n "
"	it looks like this <declare word> <object> = <expression>\n "
"	<declare word> is a jvav system reversed word that tell the system the type of \n	the <object> "
"	<declare word> now has <int>\n"
"	for exmaple| jvav :> int mur = 1919 + 810\n"
"above are all the grammar jvav currently support\n"
"here is a exmaple program\n"
"jvav :>int a = 114514\n"
"jvav :>int b = 1919\n"
"jvav :>int c = a + b - 919\n"
"jvav :>c + 100000\n"
"215514\n\n"
;

void invoke_oruga() {
	printf(oruga);
	printf("不要停下来啊\ndon't stop\n\n");
	printf("current jvav version (迫真): ver 0.114.514 \n\n");
}

void invoke_help() {
	invoke_clear();
	printf("\n\nwrite in jvav, run every where \n写JVAV 跑全世界\n");
	printf(jvav_str);
	printf(jvav_describe);
	string in;
	getline(cin, in);
	invoke_clear();
	if (in == "g") {
		printf(grammar_describe);
		printf("input anything to continue\n");
		getline(cin, in);
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