#include "FastMutex.h"
#include "Poco/Exception.h"

int luaopen_poco_fastmutex(lua_State* L)
{
	return LuaPoco::loadConstructor(L, LuaPoco::FastMutexUserdata::FastMutex);
}

namespace LuaPoco
{

FastMutexUserdata::FastMutexUserdata() :
	mFastMutex(new Poco::FastMutex())
{
}

// construct new Ud from existing SharedPtr (only useful for 
FastMutexUserdata::FastMutexUserdata(const Poco::SharedPtr<Poco::FastMutex>& fm) :
	mFastMutex(fm)
{
}

FastMutexUserdata::~FastMutexUserdata()
{
}

UserdataType FastMutexUserdata::getType()
{
	return Userdata_FastMutex;
}

bool FastMutexUserdata::isCopyable()
{
	return true;
}

bool FastMutexUserdata::copyToState(lua_State *L)
{
	void* ud = lua_newuserdata(L, sizeof(FastMutexUserdata));
	luaL_getmetatable(L, "Poco.FastMutex.metatable");
	lua_setmetatable(L, -2);
	
	FastMutexUserdata* fmud = new(ud) FastMutexUserdata(mFastMutex);
	return true;
}

// register metatable for this class
bool FastMutexUserdata::registerFastMutex(lua_State* L)
{
	luaL_newmetatable(L, "Poco.FastMutex.metatable");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, metamethod__gc);
	lua_setfield(L, -2, "__gc");
	lua_pushcfunction(L, metamethod__tostring);
	lua_setfield(L, -2, "__tostring");
	
	lua_pushstring(L, "Poco.FastMutex.metatable");
	lua_setfield(L, -2, "poco.userdata");
	
	// methods
	lua_pushcfunction(L, lock);
	lua_setfield(L, -2, "lock");
	lua_pushcfunction(L, tryLock);
	lua_setfield(L, -2, "tryLock");
	lua_pushcfunction(L, unlock);
	lua_setfield(L, -2, "unlock");
	lua_pop(L, 1);
	
	return true;
}

int FastMutexUserdata::FastMutex(lua_State* L)
{
	int rv = 0;
	void* ud = lua_newuserdata(L, sizeof(FastMutexUserdata));
	luaL_getmetatable(L, "Poco.FastMutex.metatable");
	lua_setmetatable(L, -2);
	try
	{
		rv = 1;
		FastMutexUserdata* fmud = new(ud) FastMutexUserdata();
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
int FastMutexUserdata::metamethod__gc(lua_State* L)
{
	FastMutexUserdata* fmud = reinterpret_cast<FastMutexUserdata*>(
		luaL_checkudata(L, 1, "Poco.FastMutex.metatable"));
	fmud->~FastMutexUserdata();
	
	return 0;
}

int FastMutexUserdata::metamethod__tostring(lua_State* L)
{
	FastMutexUserdata* fmud = reinterpret_cast<FastMutexUserdata*>(
		luaL_checkudata(L, 1, "Poco.FastMutex.metatable"));
	
	lua_pushfstring(L, "Poco.FastMutex (%p)", reinterpret_cast<void*>(fmud));
	return 1;
}

// userdata methods
int FastMutexUserdata::lock(lua_State* L)
{
	int rv = 0;
	FastMutexUserdata* fmud = reinterpret_cast<FastMutexUserdata*>(
		luaL_checkudata(L, 1, "Poco.FastMutex.metatable"));
	int top = lua_gettop(L);
	
	long ms;
	if (top > 1)
		ms = luaL_checkinteger(L, 2);
	
	try
	{
		if (top > 1)
			fmud->mFastMutex->lock(ms);
		else
			fmud->mFastMutex->lock();
		
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

int FastMutexUserdata::tryLock(lua_State* L)
{
	int rv = 0;
	FastMutexUserdata* fmud = reinterpret_cast<FastMutexUserdata*>(
		luaL_checkudata(L, 1, "Poco.FastMutex.metatable"));
	int top = lua_gettop(L);
	
	long ms;
	if (top > 1)
		ms = luaL_checkinteger(L, 2);
	
	try
	{
		bool result = false;
		if (top > 1)
			result = fmud->mFastMutex->tryLock(ms);
		else
			result = fmud->mFastMutex->tryLock();
		
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

int FastMutexUserdata::unlock(lua_State* L)
{
	int rv = 0;
	FastMutexUserdata* fmud = reinterpret_cast<FastMutexUserdata*>(
		luaL_checkudata(L, 1, "Poco.FastMutex.metatable"));
	
	try
	{
		fmud->mFastMutex->unlock();
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
