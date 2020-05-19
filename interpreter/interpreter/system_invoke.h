#pragma once
#include "common.h"
#include "Token.h"

void invoke_oruga();

void invoke_help();

void invoke_clear();

void invoke_exit();

using sys_invoke = void(*)();
using sys_par_invoke = void(*)(vector<Token>&,MetaPool*);

sys_invoke check_sys_invoke(string name);
sys_par_invoke check_sys_par_invoke(string name);

//func sleep(float a)
void invoke_sleep(vector<Token>& par, MetaPool* pool);
//func input(ref str c)| func input(ref int a) | func input(ref float b)
void invoke_input(vector<Token>& par, MetaPool* pool);
//func clear()
void invoke_clear_par(vector<Token>& par, MetaPool* pool);
//func exit()
void invoke_exit_par(vector<Token>& par, MetaPool* pool);
// func random(ref float a)
void invoke_random(vector<Token>& par, MetaPool* pool);
