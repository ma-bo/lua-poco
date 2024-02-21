/// Synchronization mechanism used to control access to a shared resource.
// Note: Mutexes are recursive, that is, the same mutex can be locked multiple times by the same thread (but, of course, not by other threads).  Also note that recursive mutexes are heavier and slower than the fastmutex module.
// Note: mutex userdata are copyable/sharable between threads.
// @module mutex

#include "Mutex.h"
#include <Poco/Exception.h>

int luaopen_poco_mutex(lua_State* L)
{
    LuaPoco::MutexUserdata::registerMutex(L);
    return LuaPoco::loadConstructor(L, LuaPoco::MutexUserdata::Mutex);
}

namespace LuaPoco
{

const char* POCO_MUTEX_METATABLE_NAME = "Poco.Mutex.metatable";

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

bool MutexUserdata::copyToState(lua_State *L)
{
    registerMutex(L);
    MutexUserdata* mud = new(lua_newuserdata(L, sizeof *mud)) MutexUserdata(mMutex);
    setupPocoUserdata(L, mud, POCO_MUTEX_METATABLE_NAME);
    return true;
}

// register metatable for this class
bool MutexUserdata::registerMutex(lua_State* L)
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

    setupUserdataMetatable(L, POCO_MUTEX_METATABLE_NAME, methods);
    return true;
}

/// create a new mutex userdata.
// @return userdata or nil. (error)
// @return error message.
// @function new
int MutexUserdata::Mutex(lua_State* L)
{
    int rv = 0;

    try
    {
        MutexUserdata* mud = new(lua_newuserdata(L, sizeof *mud)) MutexUserdata();
        setupPocoUserdata(L, mud, POCO_MUTEX_METATABLE_NAME);
        rv = 1;
    }
    catch (const std::exception& e)
    {
        rv = pushException(L, e);
    }
        return rv;
}

///
// @type mutex

// metamethod infrastructure
int MutexUserdata::metamethod__tostring(lua_State* L)
{
    MutexUserdata* mud = checkPrivateUserdata<MutexUserdata>(L, 1);
    
    lua_pushfstring(L, "Poco.Mutex (%p)", static_cast<void*>(mud));
    return 1;
}

/// Locks the mutex. Blocks if the mutex is held by another thread.
// @function lock

// userdata methods
int MutexUserdata::lock(lua_State* L)
{
    int rv = 0;
    MutexUserdata* mud = checkPrivateUserdata<MutexUserdata>(L, 1);
    
    try
    {
        mud->mMutex->lock();
    }
    catch (const std::exception& e)
    {
        pushException(L, e);
        lua_error(L);
    }
        
    return rv;
}

/// Attempts to lock the mutex.
// @int[opt] ms optional number of milliseconds to try to acquire the mutex.
// @return boolean indicating if lock was acquired or timeout occured.
// @function tryLock
int MutexUserdata::tryLock(lua_State* L)
{
    int rv = 0;
    MutexUserdata* mud = checkPrivateUserdata<MutexUserdata>(L, 1);
        
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
    catch (const std::exception& e)
    {
        rv = pushException(L, e);
    }
        
    return rv;
}

/// Unlocks the mutex so that it can be acquired by other threads. 
// @function unlock
int MutexUserdata::unlock(lua_State* L)
{
    int rv = 0;
    MutexUserdata* mud = checkPrivateUserdata<MutexUserdata>(L, 1);
    
    try
    {
        mud->mMutex->unlock();
        lua_pushboolean(L, 1);
        rv = 1;
    }
    catch (const std::exception& e)
    {
        rv = pushException(L, e);
    }
        
    return rv;
}

} // LuaPoco
