#include "LuaPoco.h"
#include "File.h"
#include <iostream>

extern "C" 
{

using namespace LuaPoco;

// create all metatables per class
// and load class constructors into the foundation table.
int luaopen_foundation(lua_State* L)
{
	int result = 0;
	
	lua_createtable(L, 0, 5);
	if (FileUserdata::registerFile(L))
		result = 1;
		
	return result;
}

}
