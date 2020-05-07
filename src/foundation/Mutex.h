#ifndef LUA_POCO_MUTEX_H
#define LUA_POCO_MUTEX_H

#include "LuaPoco.h"
#include "Userdata.h"
#include "Poco/Mutex.h"
#include "Poco/SharedPtr.h"

extern "C"
{
LUAPOCO_API int luaopen_poco_mutex(lua_State* L);
}

namespace LuaPoco
{

extern const char* POCO_MUTEX_METATABLE_NAME;

class MutexUserdata : public Userdata
{
public:
    MutexUserdata();
    MutexUserdata(const Poco::SharedPtr<Poco::Mutex>& mtx);
    virtual ~MutexUserdata();
    virtual bool copyToState(lua_State *L);
    // register metatable for this class
    static bool registerMutex(lua_State* L);
    // constructor function 
    static int Mutex(lua_State* L);
    
private:
    // metamethod infrastructure
    static int metamethod__tostring(lua_State* L);
    
    // userdata methods
    static int lock(lua_State* L);
    static int tryLock(lua_State* L);
    static int unlock(lua_State* L);
    
    Poco::SharedPtr<Poco::Mutex> mMutex;
};

} // LuaPoco

#endif
