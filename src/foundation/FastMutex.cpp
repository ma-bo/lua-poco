/// Non-recursive synchronization mechanism used to control access to a shared resource.
// Note: A deadlock will occur if the same thread tries to lock a mutex that has already locked.
// Note: fastmutex userdata are copyable/sharable between threads.
// @module fastmutex

#include "FastMutex.h"
#include <Poco/Exception.h>

int luaopen_poco_fastmutex(lua_State* L)
{
    LuaPoco::FastMutexUserdata::registerFastMutex(L);
    return LuaPoco::loadConstructor(L, LuaPoco::FastMutexUserdata::FastMutex);
}

namespace LuaPoco
{

const char* POCO_FASTMUTEX_METATABLE_NAME = "Poco.FastMutex.metatable";

FastMutexUserdata::FastMutexUserdata() :
    mFastMutex(new Poco::FastMutex())
{
}

// construct new Ud from existing SharedPtr
FastMutexUserdata::FastMutexUserdata(const Poco::SharedPtr<Poco::FastMutex>& fm) :
    mFastMutex(fm)
{
}

FastMutexUserdata::~FastMutexUserdata()
{
}

bool FastMutexUserdata::copyToState(lua_State *L)
{
    registerFastMutex(L);
    FastMutexUserdata* fmud = NULL;
    void *p = lua_newuserdata(L, sizeof *fmud);
    
    try
    {
        fmud = new(p) FastMutexUserdata(mFastMutex);
    }
    catch (const std::exception& e)
    {
        lua_pop(L, 1);
        return false;
    }
    
    setupPocoUserdata(L, fmud, POCO_FASTMUTEX_METATABLE_NAME);
    return true;
}

// register metatable for this class
bool FastMutexUserdata::registerFastMutex(lua_State* L)
{
    struct CFunctions methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "lock", lock },
        { "tryLock", tryLock },
        { "unlock", unlock },
        { NULL, NULL}
    };

    setupUserdataMetatable(L, POCO_FASTMUTEX_METATABLE_NAME, methods);
    
    return true;
}

/// constructs a new fastmutex userdata.
// @return userdata or nil. (error)
// @return error message
// @function new
int FastMutexUserdata::FastMutex(lua_State* L)
{

    FastMutexUserdata* fmud = NULL;
    void* p = lua_newuserdata(L, sizeof *fmud);
    
    try
    {
        fmud = new(p) FastMutexUserdata();
    }
    catch (const std::exception& e)
    {
        return pushException(L, e);
    }
    
    setupPocoUserdata(L, fmud, POCO_FASTMUTEX_METATABLE_NAME);
    return 1;
}

///
// @type fastmutex

// metamethod infrastructure
int FastMutexUserdata::metamethod__tostring(lua_State* L)
{
    FastMutexUserdata* fmud = checkPrivateUserdata<FastMutexUserdata>(L, 1);
    lua_pushfstring(L, "Poco.FastMutex (%p)", static_cast<void*>(fmud));
    return 1;
}

/// Locks the mutex. Blocks if the mutex is already held.
// @function lock

// userdata methods
int FastMutexUserdata::lock(lua_State* L)
{
    FastMutexUserdata* fmud = checkPrivateUserdata<FastMutexUserdata>(L, 1);
    
    try
    {
        fmud->mFastMutex->lock();
    }
    catch (const std::exception& e)
    {
        pushException(L, e);
        lua_error(L);
    }
        
    return 0;
}

/// Attempts to lock the mutex.
// @int[opt] ms optional number of milliseconds to try to acquire the mutex.
// @return boolean indicating if lock was acquired or timeout occured.
// @function tryLock
int FastMutexUserdata::tryLock(lua_State* L)
{
    FastMutexUserdata* fmud = checkPrivateUserdata<FastMutexUserdata>(L, 1);
    int top = lua_gettop(L);
    long ms = 0;
    
    if (top > 1)
        ms = luaL_checkinteger(L, 2);
    
    bool result = false;
    
    try
    {
        if (ms > 0)
            result = fmud->mFastMutex->tryLock(ms);
        else
            result = fmud->mFastMutex->tryLock();
    }
    catch (const std::exception& e)
    {
        return pushException(L, e);
    }
    
    lua_pushboolean(L, result);
    return 1;
}

/// Unlocks the mutex so that it can be acquired by other threads. 
// @function unlock
int FastMutexUserdata::unlock(lua_State* L)
{
    FastMutexUserdata* fmud = checkPrivateUserdata<FastMutexUserdata>(L, 1);
    
    try
    {
        fmud->mFastMutex->unlock();
    }
    catch (const std::exception& e)
    {
        pushException(L, e);
        lua_error(L);
    }
        
    return 0;
}

} // LuaPoco
