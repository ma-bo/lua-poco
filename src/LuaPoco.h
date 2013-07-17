#ifndef LUAPOCO_H
#define LUAPOCO_H

#ifdef USE_LUA_AS_CPP
// define variable via CMAKE configure time to 
// link against a C++ compilation of Lua
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"

#else
// include as a C library
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#endif

#endif
