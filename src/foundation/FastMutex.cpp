/// Non-recursive synchronization mechanism used to control access to a shared resource.
// Note: A deadlock will occur if the same thread tries to lock a mutex that has already locked.
// Note: fastmutex userdata are copyable/sharable between threads.
// @module fastmutex

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

// construct new Ud from existing SharedPtr
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

/// constructs a new fastmutex userdata.
// @return userdata or nil. (error)
// @return error message
// @function new
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

///
// @type fastmutex

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

/// Locks the mutex. Blocks if the mutex is already held.
// @function lock

// userdata methods
int FastMutexUserdata::lock(lua_State* L)
{
    int rv = 0;
    FastMutexUserdata* fmud = reinterpret_cast<FastMutexUserdata*>(
        luaL_checkudata(L, 1, "Poco.FastMutex.metatable"));
    
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

/// Unlocks the mutex so that it can be acquired by other threads. 
// @function unlock
int FastMutexUserdata::unlock(lua_State* L)
{
    int rv = 0;
    FastMutexUserdata* fmud = reinterpret_cast<FastMutexUserdata*>(
        luaL_checkudata(L, 1, "Poco.FastMutex.metatable"));
    
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
