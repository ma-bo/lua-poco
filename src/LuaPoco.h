#ifndef LUAPOCO_H
#define LUAPOCO_H

#ifdef _WIN32
    #ifdef poco_EXPORTS
        #define LUAPOCO_API __declspec(dllexport)
    #else
        #define LUAPOCO_API __declspec(dllimport)
    #endif
#else
    #define LUAPOCO_API
#endif

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
