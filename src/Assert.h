#pragma once

#ifdef _DEBUG
	#include "Log.h"
	#define QUACK_ASSERT(expresion, ...) { if(!(expresion)) { QUACK_ERROR(__VA_ARGS__); __debugbreak(); } }
#else
	#define QUACK_ASSERT(expresion, ...)
#endif
