#include "NamedEvent.h"
#include "Poco/Exception.h"
#include <cstring>

namespace LuaPoco
{

NamedEventUserdata::NamedEventUserdata(const std::string& name) :
	mNamedEvent(name)
{
}

NamedEventUserdata::~NamedEventUserdata()
{
}

UserdataType NamedEventUserdata::getType()
{
	return Userdata_NamedEvent;
}

// register metatable for this class
bool NamedEventUserdata::registerNamedEvent(lua_State* L)
{
	bool result = false;
	if (!lua_istable(L, -1))
		return result;
	
	// constructor
	lua_pushcfunction(L, NamedEvent);
	lua_setfield(L, -2, "NamedEvent");
	
	luaL_newmetatable(L, "Poco.NamedEvent.metatable");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, metamethod__gc);
	lua_setfield(L, -2, "__gc");
	lua_pushcfunction(L, metamethod__tostring);
	lua_setfield(L, -2, "__tostring");
	
	lua_pushstring(L, "Poco.NamedEvent.metatable");
	lua_setfield(L, -2, "poco.userdata");
	
	// methods
	lua_pushcfunction(L, set);
	lua_setfield(L, -2, "set");
	lua_pushcfunction(L, wait);
	lua_setfield(L, -2, "wait");
	lua_pop(L, 1);
	result = true;
	
	return result;
}

int NamedEventUserdata::NamedEvent(lua_State* L)
{
	const char* name = luaL_checkstring(L, 1);
	
	void* ud = lua_newuserdata(L, sizeof(NamedEventUserdata));
	luaL_getmetatable(L, "Poco.NamedEvent.metatable");
	lua_setmetatable(L, -2);
	
	NamedEventUserdata* neud = new(ud) NamedEventUserdata(name);
	return 1;
}

// metamethod infrastructure
int NamedEventUserdata::metamethod__gc(lua_State* L)
{
	NamedEventUserdata* neud = reinterpret_cast<NamedEventUserdata*>(
		luaL_checkudata(L, 1, "Poco.NamedEvent.metatable"));
	neud->~NamedEventUserdata();
	
	return 0;
}

int NamedEventUserdata::metamethod__tostring(lua_State* L)
{
	NamedEventUserdata* neud = reinterpret_cast<NamedEventUserdata*>(
		luaL_checkudata(L, 1, "Poco.NamedEvent.metatable"));
	
	lua_pushfstring(L, "Poco.NamedEvent (%p)", reinterpret_cast<void*>(neud));
	return 1;
}

// userdata methods
int NamedEventUserdata::set(lua_State* L)
{
	NamedEventUserdata* neud = reinterpret_cast<NamedEventUserdata*>(
		luaL_checkudata(L, 1, "Poco.NamedEvent.metatable"));
	
	neud->mNamedEvent.set();
	
	return 0;
}

int NamedEventUserdata::wait(lua_State* L)
{
	NamedEventUserdata* neud = reinterpret_cast<NamedEventUserdata*>(
		luaL_checkudata(L, 1, "Poco.NamedEvent.metatable"));
	
	neud->mNamedEvent.wait();
	
	return 0;
}

} // LuaPoco
