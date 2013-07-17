#include "LuaPoco.h"
#include "File.h"
#include <iostream>

extern "C" 
{

using namespace LuaPoco;

int luaopen_foundation(lua_State* L)
{
	int result = 0;
	
	lua_createtable(L, 0, 5);
	if (LuaPoco::FileUserdata::registerFile(L))
		result = 1;
		
	return result;
}

}
