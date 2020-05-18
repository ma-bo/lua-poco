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
    FastMutexUserdata* fmud = new(lua_newuserdata(L, sizeof *fmud)) FastMutexUserdata(mFastMutex);
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
    int rv = 0;

    try
    {
        FastMutexUserdata* fmud = new(lua_newuserdata(L, sizeof *fmud)) FastMutexUserdata();
        setupPocoUserdata(L, fmud, POCO_FASTMUTEX_METATABLE_NAME);
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
    int rv = 0;
    FastMutexUserdata* fmud = checkPrivateUserdata<FastMutexUserdata>(L, 1);
    
    try
    {
        fmud->mFastMutex->lock();
    }
    catch (const Poco::Exception& e)
    {
        pushPocoException(L, e);
        lua_error(L);
    }
    catch (...)
    {
        pushUnknownException(L);
        lua_error(L);
    }
    
    return rv;
}

/// Attempts to lock the mutex.
// @int[opt] ms optional number of milliseconds to try to acquire the mutex.
// @return boolean indicating if lock was acquired or timeout occured.
// @function tryLock
int FastMutexUserdata::tryLock(lua_State* L)
{
    int rv = 0;
    FastMutexUserdata* fmud = checkPrivateUserdata<FastMutexUserdata>(L, 1);
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

/// Unlocks the mutex so that it can be acquired by other threads. 
// @function unlock
int FastMutexUserdata::unlock(lua_State* L)
{
    int rv = 0;
    FastMutexUserdata* fmud = checkPrivateUserdata<FastMutexUserdata>(L, 1);
    
    try
    {
        fmud->mFastMutex->unlock();
    }
    catch (const Poco::Exception& e)
    {
        pushPocoException(L, e);
        lua_error(L);
    }
    catch (...)
    {
        pushUnknownException(L);
        lua_error(L);
    }
    
    return rv;
}

} // LuaPoco
