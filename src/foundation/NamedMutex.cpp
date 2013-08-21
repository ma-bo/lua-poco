#include "NamedMutex.h"
#include "Poco/Exception.h"

namespace LuaPoco
{

NamedMutexUserdata::NamedMutexUserdata(const std::string& name) :
	mNamedMutex(name)
{
}

NamedMutexUserdata::~NamedMutexUserdata()
{
}

UserdataType NamedMutexUserdata::getType()
{
	return Userdata_NamedMutex;
}

// register metatable for this class
bool NamedMutexUserdata::registerNamedMutex(lua_State* L)
{
	bool result = false;
	if (!lua_istable(L, -1))
		return result;
	
	// constructor
	lua_pushcfunction(L, NamedMutex);
	lua_setfield(L, -2, "NamedMutex");
	
	luaL_newmetatable(L, "Poco.NamedMutex.metatable");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, metamethod__gc);
	lua_setfield(L, -2, "__gc");
	lua_pushcfunction(L, metamethod__tostring);
	lua_setfield(L, -2, "__tostring");
	
	lua_pushstring(L, "Poco.NamedMutex.metatable");
	lua_setfield(L, -2, "poco.userdata");
	
	// methods
	lua_pushcfunction(L, lock);
	lua_setfield(L, -2, "lock");
	lua_pushcfunction(L, tryLock);
	lua_setfield(L, -2, "tryLock");
	lua_pushcfunction(L, unlock);
	lua_setfield(L, -2, "unlock");
	lua_pop(L, 1);
	result = true;
	
	return result;
}

int NamedMutexUserdata::NamedMutex(lua_State* L)
{
	const char* name = luaL_checkstring(L, 1);
	
	void* ud = lua_newuserdata(L, sizeof(NamedMutexUserdata));
	luaL_getmetatable(L, "Poco.NamedMutex.metatable");
	lua_setmetatable(L, -2);
	
	NamedMutexUserdata* nmud = new(ud) NamedMutexUserdata(name);
	return 1;
}

// metamethod infrastructure
int NamedMutexUserdata::metamethod__gc(lua_State* L)
{
	NamedMutexUserdata* nmud = reinterpret_cast<NamedMutexUserdata*>(
		luaL_checkudata(L, 1, "Poco.NamedMutex.metatable"));
	nmud->~NamedMutexUserdata();
	
	return 0;
}

int NamedMutexUserdata::metamethod__tostring(lua_State* L)
{
	NamedMutexUserdata* nmud = reinterpret_cast<NamedMutexUserdata*>(
		luaL_checkudata(L, 1, "Poco.NamedMutex.metatable"));
	
	lua_pushfstring(L, "Poco.NamedMutex (%p)", reinterpret_cast<void*>(nmud));
	return 1;
}

// userdata methods
int NamedMutexUserdata::lock(lua_State* L)
{
	NamedMutexUserdata* nmud = reinterpret_cast<NamedMutexUserdata*>(
		luaL_checkudata(L, 1, "Poco.NamedMutex.metatable"));
	
	nmud->mNamedMutex.lock();
	
	return 0;
}

int NamedMutexUserdata::tryLock(lua_State* L)
{
	NamedMutexUserdata* nmud = reinterpret_cast<NamedMutexUserdata*>(
		luaL_checkudata(L, 1, "Poco.NamedMutex.metatable"));
	
	bool locked = nmud->mNamedMutex.tryLock();
	lua_pushboolean(L, locked);
	
	return 1;
}

int NamedMutexUserdata::unlock(lua_State* L)
{
	NamedMutexUserdata* nmud = reinterpret_cast<NamedMutexUserdata*>(
		luaL_checkudata(L, 1, "Poco.NamedMutex.metatable"));
	
	nmud->mNamedMutex.unlock();
	
	return 0;
}

} // LuaPoco
