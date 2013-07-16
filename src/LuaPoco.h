// define variable via CMAKE configure time to 
// link against a C++ compilation of Lua
#ifdef USE_LUA_AS_CPP
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#else
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#endif


