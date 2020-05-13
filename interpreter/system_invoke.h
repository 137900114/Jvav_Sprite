#pragma once
#include "common.h"

void invoke_oruga();

void invoke_help();

void invoke_clear();

void invoke_exit();

using sys_invoke = void(*)();

sys_invoke check_sys_invoke(string name);