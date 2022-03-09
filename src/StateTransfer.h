#ifndef LUA_POCO_STATETRANSFER_H
#define LUA_POCO_STATETRANSFER_H

#include "LuaPoco.h"

namespace LuaPoco
{

int functionWriter(lua_State* L, const void* p, size_t sz, void* ud);
const char* functionReader(lua_State* L, void* data, size_t* size);
bool transferFunction(lua_State* toL, lua_State* fromL);
bool transferValue(lua_State* toL, lua_State* fromL);

} // LuaPoco


#endif
