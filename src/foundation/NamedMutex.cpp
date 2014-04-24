/// Global synchronization mechanism used to control access to a shared resource.
// Note: There should not be more than one instance of namedmutex for a given name in a process. Otherwise, the instances may interfere with each other.
// @module namedmutex

#include "NamedMutex.h"
#include "Poco/Exception.h"

int luaopen_poco_namedmutex(lua_State* L)
{
    return LuaPoco::loadConstructor(L, LuaPoco::NamedMutexUserdata::NamedMutex);
}

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
    
    return true;
}

/// constructs a new namedevent userdata.
// @string name global mutex name.
// @return userdata.
// @function new
int NamedMutexUserdata::NamedMutex(lua_State* L)
{
    const char* name = luaL_checkstring(L, 1);
    
    void* ud = lua_newuserdata(L, sizeof(NamedMutexUserdata));
    luaL_getmetatable(L, "Poco.NamedMutex.metatable");
    lua_setmetatable(L, -2);
    
    NamedMutexUserdata* nmud = new(ud) NamedMutexUserdata(name);
    return 1;
}

///
// @type namedmutex

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

/// Locks the namedmutex.
// Blocks if the mutex is held by another thread.
// @function lock
int NamedMutexUserdata::lock(lua_State* L)
{
    NamedMutexUserdata* nmud = reinterpret_cast<NamedMutexUserdata*>(
        luaL_checkudata(L, 1, "Poco.NamedMutex.metatable"));
    
    nmud->mNamedMutex.lock();
    
    return 0;
}

/// Attempts to lock the namedmutex.
// Returns false immediately if the mutex is already held by another process.
// @return boolean indicating if lock was acquired or not.
// @function tryLock
int NamedMutexUserdata::tryLock(lua_State* L)
{
    NamedMutexUserdata* nmud = reinterpret_cast<NamedMutexUserdata*>(
        luaL_checkudata(L, 1, "Poco.NamedMutex.metatable"));
    
    bool locked = nmud->mNamedMutex.tryLock();
    lua_pushboolean(L, locked);
    
    return 1;
}

/// Unlocks the mutex so that it can be acquired by other threads. 
// @function unlock
int NamedMutexUserdata::unlock(lua_State* L)
{
    NamedMutexUserdata* nmud = reinterpret_cast<NamedMutexUserdata*>(
        luaL_checkudata(L, 1, "Poco.NamedMutex.metatable"));
    
    nmud->mNamedMutex.unlock();
    
    return 0;
}

} // LuaPoco
