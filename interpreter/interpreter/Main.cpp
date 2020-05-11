#include "Token.h"
#include "pack_data.h"

const char* fbi_warning = 
"-----------------------------------------------------------------------------------------------\n"
"|**************************************FBI WARNING********************************************|\n"
"|*****************This program is created just because of some internet meme.*****************|\n"
"|*************************The author has no intention to insult anyone************************|\n"
"|*****************Jvav has no relationship with real life Java or Oracle cop******************|\n"
"-----------------------------------------------------------------------------------------------\n"
"input help() for more information\n"
;

int main() {
	JVM jvav;
	
	printf(fbi_warning);

	while (true) {
		try {
			string exp;
			printf("jvav :>> ");
			getline(cin, exp);

			jvav.command_line(exp);
		}
		catch (string error) {
			error += '\n';
			printf(error.c_str());
		}
	}
}