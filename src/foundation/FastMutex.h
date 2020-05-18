#ifndef LUA_POCO_FASTMUTEX_H
#define LUA_POCO_FASTMUTEX_H

#include "LuaPoco.h"
#include "Userdata.h"
#include <Poco/Mutex.h>
#include <Poco/SharedPtr.h>

extern "C"
{
LUAPOCO_API int luaopen_poco_fastmutex(lua_State* L);
}

namespace LuaPoco
{

extern const char* POCO_FASTMUTEX_METATABLE_NAME;

class FastMutexUserdata : public Userdata
{
public:
    FastMutexUserdata();
    FastMutexUserdata(const Poco::SharedPtr<Poco::FastMutex>& mtx);
    virtual ~FastMutexUserdata();
    virtual bool copyToState(lua_State *L);
    // register metatable for this class
    static bool registerFastMutex(lua_State* L);
    // constructor function 
    static int FastMutex(lua_State* L);
    
private:
    // metamethod infrastructure
    static int metamethod__tostring(lua_State* L);
    
    // userdata methods
    static int lock(lua_State* L);
    static int tryLock(lua_State* L);
    static int unlock(lua_State* L);
    
    Poco::SharedPtr<Poco::FastMutex> mFastMutex;
};

} // LuaPoco

#endif
