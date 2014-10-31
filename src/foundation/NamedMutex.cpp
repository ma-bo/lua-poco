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

const char* POCO_NAMEDMUTEX_METATABLE_NAME = "Poco.NamedMutex.metatable";

NamedMutexUserdata::NamedMutexUserdata(const std::string& name) :
    mNamedMutex(name)
{
}

NamedMutexUserdata::~NamedMutexUserdata()
{
}

// register metatable for this class
bool NamedMutexUserdata::registerNamedMutex(lua_State* L)
{
    struct UserdataMethod methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "lock", lock },
        { "tryLock", tryLock },
        { "unlock", unlock },
        { NULL, NULL}
    };
    
    setupUserdataMetatable(L, POCO_NAMEDMUTEX_METATABLE_NAME, methods);    
    return true;
}

/// constructs a new namedevent userdata.
// @string name global mutex name.
// @return userdata.
// @function new
int NamedMutexUserdata::NamedMutex(lua_State* L)
{
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    const char* name = luaL_checkstring(L, firstArg);
    NamedMutexUserdata* nmud = new(lua_newuserdata(L, sizeof *nmud)) NamedMutexUserdata(name);
    setupPocoUserdata(L, nmud, POCO_NAMEDMUTEX_METATABLE_NAME);
    
    return 1;
}

///
// @type namedmutex

// metamethod infrastructure
int NamedMutexUserdata::metamethod__tostring(lua_State* L)
{
    NamedMutexUserdata* nmud = checkPrivateUserdata<NamedMutexUserdata>(L, 1);
    
    lua_pushfstring(L, "Poco.NamedMutex (%p)", static_cast<void*>(nmud));
    return 1;
}

// userdata methods

/// Locks the namedmutex.
// Blocks if the mutex is held by another thread.
// @function lock
int NamedMutexUserdata::lock(lua_State* L)
{
    NamedMutexUserdata* nmud = checkPrivateUserdata<NamedMutexUserdata>(L, 1);
    
    nmud->mNamedMutex.lock();
    
    return 0;
}

/// Attempts to lock the namedmutex.
// Returns false immediately if the mutex is already held by another process.
// @return boolean indicating if lock was acquired or not.
// @function tryLock
int NamedMutexUserdata::tryLock(lua_State* L)
{
    NamedMutexUserdata* nmud = checkPrivateUserdata<NamedMutexUserdata>(L, 1);
    
    bool locked = nmud->mNamedMutex.tryLock();
    lua_pushboolean(L, locked);
    
    return 1;
}

/// Unlocks the mutex so that it can be acquired by other threads. 
// @function unlock
int NamedMutexUserdata::unlock(lua_State* L)
{
    NamedMutexUserdata* nmud = checkPrivateUserdata<NamedMutexUserdata>(L, 1);

    nmud->mNamedMutex.unlock();
    
    return 0;
}

} // LuaPoco
