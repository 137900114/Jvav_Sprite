#include "Token.h"
#include "pack_data.h"
#include <fstream>
#include <ctime>

const char* fbi_warning =
"-----------------------------------------------------------------------------------------------\n"
"|**************************************FBI WARNING********************************************|\n"
"|*****************This program is created just because of some internet meme.*****************|\n"
"|*************************The author has no intention to insult anyone************************|\n"
"|*****************Jvav has no relationship with real life Java or Oracle cop******************|\n"
"-----------------------------------------------------------------------------------------------\n"
"input help() for more information\n"
;

bool check_extent_name(string file) {
	return file.substr(file.size() - 4, 4) == "jspr";
}

int main(int argc,char** args) {
	JVM jvav;

	//compile settings
	bool compile = false;
	bool run = true;
	string target = "";

	if (argc <= 1) {
		//activate command line mode
		printf(fbi_warning);

		while (true) {
			try {
				string exp;
				printf("jspr :>> ");
				getline(cin, exp);
				jvav.command_line(exp);
			}
			catch (string error) {
				error += '\n';
				printf(error.c_str());
			}
		}
	}
	else {
		
		for (int i = 1; i != argc; i++) {
			string cmd = args[i];
			if (cmd[0] != '-') {
				if (target.size() == 0) {
					target = cmd;
				}
				else {
					printf("error : jvav sprite accept one file at once\n");
					return -1;
				}
			}
			else {
				printf("error : currently no compile option is supported\n");
				return -1;
			}

		}

		if (!check_extent_name(target)) {
			printf("jvav sprite file should end with .jspr!!");
			return -1;
		}

		ifstream ifs(target);
		if (!ifs.is_open()) {
			printf("invaild target file name %s\n", target.c_str());
			return -1;
		}

		vector<string> lines;
		string line;
		while (getline(ifs,line)) {
			lines.push_back(line);
		}

		try {
			jvav.excute(lines);
		}
		catch (string e) {
			printf("runtime error : %s",e.c_str());
		}
	}
}