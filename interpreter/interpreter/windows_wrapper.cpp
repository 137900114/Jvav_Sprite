#include "windows_wrapper.h"
#include <Windows.h>
#include <ctime>

void w_sleep(float time) {
	DWORD t = time * 1000;
	Sleep(t);
}

int w_time(){
	return time(nullptr);
}