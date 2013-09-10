#include "Mutex.h"
#include "Poco/Exception.h"

int luaopen_poco_mutex(lua_State* L)
{
	return LuaPoco::loadConstructor(L, LuaPoco::MutexUserdata::Mutex);
}

namespace LuaPoco
{

MutexUserdata::MutexUserdata() :
	mMutex(new Poco::Mutex())
{
}

// construct new Ud from existing SharedPtr (only useful for 
MutexUserdata::MutexUserdata(const Poco::SharedPtr<Poco::Mutex>& fm) :
	mMutex(fm)
{
}

MutexUserdata::~MutexUserdata()
{
}

UserdataType MutexUserdata::getType()
{
	return Userdata_Mutex;
}

bool MutexUserdata::isCopyable()
{
	return true;
}

bool MutexUserdata::copyToState(lua_State *L)
{
	void* ud = lua_newuserdata(L, sizeof(MutexUserdata));
	luaL_getmetatable(L, "Poco.Mutex.metatable");
	lua_setmetatable(L, -2);
	
	MutexUserdata* mud = new(ud) MutexUserdata(mMutex);
	return true;
}

// register metatable for this class
bool MutexUserdata::registerMutex(lua_State* L)
{
	luaL_newmetatable(L, "Poco.Mutex.metatable");
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, metamethod__gc);
	lua_setfield(L, -2, "__gc");
	lua_pushcfunction(L, metamethod__tostring);
	lua_setfield(L, -2, "__tostring");
	
	lua_pushstring(L, "Poco.Mutex.metatable");
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

int MutexUserdata::Mutex(lua_State* L)
{
	int rv = 0;
	void* ud = lua_newuserdata(L, sizeof(MutexUserdata));
	luaL_getmetatable(L, "Poco.Mutex.metatable");
	lua_setmetatable(L, -2);
	try
	{
		rv = 1;
		MutexUserdata* mud = new(ud) MutexUserdata();
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
int MutexUserdata::metamethod__gc(lua_State* L)
{
	MutexUserdata* mud = reinterpret_cast<MutexUserdata*>(
		luaL_checkudata(L, 1, "Poco.Mutex.metatable"));
	mud->~MutexUserdata();
	
	return 0;
}

int MutexUserdata::metamethod__tostring(lua_State* L)
{
	MutexUserdata* mud = reinterpret_cast<MutexUserdata*>(
		luaL_checkudata(L, 1, "Poco.Mutex.metatable"));
	
	lua_pushfstring(L, "Poco.Mutex (%p)", reinterpret_cast<void*>(mud));
	return 1;
}

// userdata methods
int MutexUserdata::lock(lua_State* L)
{
	int rv = 0;
	MutexUserdata* mud = reinterpret_cast<MutexUserdata*>(
		luaL_checkudata(L, 1, "Poco.Mutex.metatable"));
	int top = lua_gettop(L);
	
	long ms;
	if (top > 1)
		ms = luaL_checkinteger(L, 2);
	
	try
	{
		if (top > 1)
			mud->mMutex->lock(ms);
		else
			mud->mMutex->lock();
		
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

int MutexUserdata::tryLock(lua_State* L)
{
	int rv = 0;
	MutexUserdata* mud = reinterpret_cast<MutexUserdata*>(
		luaL_checkudata(L, 1, "Poco.Mutex.metatable"));
	int top = lua_gettop(L);
	
	long ms;
	if (top > 1)
		ms = luaL_checkinteger(L, 2);
	
	try
	{
		bool result = false;
		if (top > 1)
			result = mud->mMutex->tryLock(ms);
		else
			result = mud->mMutex->tryLock();
		
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

int MutexUserdata::unlock(lua_State* L)
{
	int rv = 0;
	MutexUserdata* mud = reinterpret_cast<MutexUserdata*>(
		luaL_checkudata(L, 1, "Poco.Mutex.metatable"));
	
	try
	{
		mud->mMutex->unlock();
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
