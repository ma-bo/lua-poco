#ifndef LUA_POCO_DYNAMICANY_H
#define LUA_POCO_DYNAMICANY_H

#include "LuaPoco.h"
#include "Loader.h"
#include "Userdata.h"
#include "Poco/DynamicAny.h"

extern "C"
{
LUAPOCO_API int luaopen_poco_dynamicany(lua_State* L);
}

namespace LuaPoco
{

class DynamicAnyUserdata
{
public:
    template <class T>
    DynamicAnyUserdata(T val) : mDynamicAny(val) {}
    DynamicAnyUserdata(const Poco::DynamicAny& da);
    virtual ~DynamicAnyUserdata();
    virtual UserdataType getType();
    virtual bool isCopyable();
    virtual bool copyToState(lua_State* L);
    // register metatable for this class
    static bool registerDynamicAny(lua_State* L);
    // Lua constructor
    static int DynamicAny(lua_State* L);
    
    Poco::DynamicAny mDynamicAny;
private:
    DynamicAnyUserdata();
    
    // metamethod infrastructure
    static int metamethod__gc(lua_State* L);
    static int metamethod__tostring(lua_State* L);
    
    // methods
    static int convert(lua_State* L);
    static int isNumeric(lua_State* L);
    static int isInteger(lua_State* L);
    static int isSigned(lua_State* L);
    static int isString(lua_State* L);
    static int toNumber(lua_State* L);
    static int toString(lua_State* L);
    static int toBoolean(lua_State* L);
    static int metamethod__add(lua_State* L);
    static int metamethod__sub(lua_State* L);
    static int metamethod__mul(lua_State* L);
    static int metamethod__div(lua_State* L);
    static int metamethod__eq(lua_State* L);
    static int metamethod__lt(lua_State* L);
    static int metamethod__le(lua_State* L);
};

} // LuaPoco


#endif
