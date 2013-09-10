#include "Semaphore.h"
#include "Poco/Exception.h"

int luaopen_poco_semaphore(lua_State* L)
{
	return LuaPoco::loadConstructor(L, LuaPoco::SemaphoreUserdata::Semaphore);
}

namespace LuaPoco
{

SemaphoreUserdata::SemaphoreUserdata(int n) :
	mSemaphore(new Poco::Semaphore(n))
{
}

SemaphoreUserdata::SemaphoreUserdata(int n, int max) :
	mSemaphore(new Poco::Semaphore(n, max))
{
}

// construct new Ud from existing SharedPtr (only useful for 
SemaphoreUserdata::SemaphoreUserdata(const Poco::SharedPtr<Poco::Semaphore>& sem) :
	mSemaphore(sem)
{
}

SemaphoreUserdata::~SemaphoreUserdata()
{
}

UserdataType SemaphoreUserdata::getType()
{
	return Userdata_Semaphore;
}

bool SemaphoreUserdata::isCopyable()
{
	return true;
}

bool SemaphoreUserdata::copyToState(lua_State *L)
{
	void* ud = lua_newuserdata(L, sizeof(SemaphoreUserdata));
	luaL_getmetatable(L, "Poco.Semaphore.metatable");
	lua_setmetatable(L, -2);
	
	SemaphoreUserdata* sud = new(ud) SemaphoreUserdata(mSemaphore);
	return true;
}

// register metatable for this class
bool SemaphoreUserdata::registerSemaphore(lua_State* L)
{
	luaL_newmetatable(L, "Poco.Semaphore.metatable");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, metamethod__gc);
	lua_setfield(L, -2, "__gc");
	lua_pushcfunction(L, metamethod__tostring);
	lua_setfield(L, -2, "__tostring");
	
	lua_pushstring(L, "Poco.Semaphore.metatable");
	lua_setfield(L, -2, "poco.userdata");
	
	// methods
	lua_pushcfunction(L, set);
	lua_setfield(L, -2, "set");
	lua_pushcfunction(L, tryWait);
	lua_setfield(L, -2, "tryWait");
	lua_pushcfunction(L, wait);
	lua_setfield(L, -2, "wait");
	lua_pop(L, 1);
	
	return true;
}

int SemaphoreUserdata::Semaphore(lua_State* L)
{
	int rv = 0;
	int top = lua_gettop(L);
	int max = 0;
	int n = luaL_checkinteger(L, 1);
	if (top > 1)
		max = luaL_checkinteger(L, 2);

	void* ud = lua_newuserdata(L, sizeof(SemaphoreUserdata));
	luaL_getmetatable(L, "Poco.Semaphore.metatable");
	lua_setmetatable(L, -2);
	try
	{
		rv = 1;
		
		if (top > 1)
			SemaphoreUserdata* sud = new(ud) SemaphoreUserdata(n, max);
		else
			SemaphoreUserdata* sud = new(ud) SemaphoreUserdata(n);
	}
	catch (const Poco::Exception& e)
	{
		rv = pushPocoException(L, e);
	}
	catch (...)
	{
		rv = pushUnknownException(L);
	}
	return rv;
}

// metamethod infrastructure
int SemaphoreUserdata::metamethod__gc(lua_State* L)
{
	SemaphoreUserdata* sud = reinterpret_cast<SemaphoreUserdata*>(
		luaL_checkudata(L, 1, "Poco.Semaphore.metatable"));
	sud->~SemaphoreUserdata();
	
	return 0;
}

int SemaphoreUserdata::metamethod__tostring(lua_State* L)
{
	SemaphoreUserdata* sud = reinterpret_cast<SemaphoreUserdata*>(
		luaL_checkudata(L, 1, "Poco.Semaphore.metatable"));
	
	lua_pushfstring(L, "Poco.Semaphore (%p)", reinterpret_cast<void*>(sud));
	return 1;
}

// userdata methods
int SemaphoreUserdata::set(lua_State* L)
{
	int rv = 0;
	SemaphoreUserdata* sud = reinterpret_cast<SemaphoreUserdata*>(
		luaL_checkudata(L, 1, "Poco.Semaphore.metatable"));
	try
	{
		sud->mSemaphore->set();
		lua_pushboolean(L, 1);
		rv = 1;
	}
	catch (const Poco::Exception& e)
	{
		rv = pushPocoException(L, e);
	}
	catch (...)
	{
		rv = pushUnknownException(L);
	}
	
	return rv;
}

int SemaphoreUserdata::tryWait(lua_State* L)
{
	int rv = 0;
	SemaphoreUserdata* sud = reinterpret_cast<SemaphoreUserdata*>(
		luaL_checkudata(L, 1, "Poco.Semaphore.metatable"));
	long ms = luaL_checkinteger(L, 2);
	
	try
	{
		bool result = sud->mSemaphore->tryWait(ms);
		lua_pushboolean(L, result);
		rv = 1;
	}
	catch (const Poco::Exception& e)
	{
		rv = pushPocoException(L, e);
	}
	catch (...)
	{
		rv = pushUnknownException(L);
	}
	
	return rv;
}

int SemaphoreUserdata::wait(lua_State* L)
{
	int rv = 0;
	SemaphoreUserdata* sud = reinterpret_cast<SemaphoreUserdata*>(
		luaL_checkudata(L, 1, "Poco.Semaphore.metatable"));
	
	try
	{
		sud->mSemaphore->wait();
		lua_pushboolean(L, 1);
		rv = 1;
	}
	catch (const Poco::Exception& e)
	{
		rv = pushPocoException(L, e);
	}
	catch (...)
	{
		rv = pushUnknownException(L);
	}
	
	return rv;
}

} // LuaPoco
