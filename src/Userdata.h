#ifndef LUA_POCO_USERDATA_H
#define LUA_POCO_USERDATA_H

#include "LuaPoco.h"
#include "Poco/Exception.h"
#include <typeinfo>

#define USERDATA_PRIVATE_TABLE "poco_userdata_private"

namespace LuaPoco
{

// a utility class for doing RAII style cleanup of a Lua state due to early returns.
class LuaStateHolder
{
public:
    lua_State* state;
    LuaStateHolder(lua_State* L);
    ~LuaStateHolder();
    lua_State* extract();
};

// base class for all userdata
class Userdata
{
public:
    Userdata();
    virtual ~Userdata();
    virtual bool copyToState(lua_State *L);
protected:
    // generic gc metamethod to reduce code duplication between derived classes.
    static int metamethod__gc(lua_State* L);
private:
    Userdata(const Userdata& disabledCopy);
    Userdata& operator=(const Userdata& disabledAssignment);
};

// generic functions to reduce the amount of copy and paste code.
int pushPocoException(lua_State* L, const Poco::Exception& e);
int pushUnknownException(lua_State* L);

struct UserdataMethod
{
    const char* name;
    lua_CFunction fn;
};

// installs methods into table at the top of the stack.
void setMetatableFunctions(lua_State* L, UserdataMethod* methods);

// since the register functions between 5.1, and 5.2 changed, use an agnostic version.
// creates a new metatable with metatableName, and installs methods into it.
void setupUserdataMetatable(lua_State* L, const char* metatableName, UserdataMethod* methods);

// NOTE: requires that the userdata value be on the top of the Lua stack.
// attaches the appropriate metatable to the userdata
// and stores the Userdata* in the private table.
void setupPocoUserdata(lua_State* L, Userdata* ud, const char* metatableName);

// constructs the private table for mapping a specific Userdata pointer to the base Userdata pointer.
void setupPrivateUserdata(lua_State* L);

// sets the association between a specific Userdata pointer and the base Userdata pointer.
void setPrivateUserdata(lua_State* L, int userdataIdx, Userdata* ud);

// gets the base Userdata pointer associated with a specific Userdata pointer.
Userdata* getPrivateUserdata(lua_State* L, int userdataIdx);

// generic to validate that a Userdata* can be dynamic_cast to a specific Userdata pointer, lua_error() if not.
template <typename T>
T* checkPrivateUserdata(lua_State* L, int userdataIdx)
{
    T* derived = NULL;
    userdataIdx = userdataIdx < 0 ? lua_gettop(L) + 1 + userdataIdx : userdataIdx;
    
    luaL_checktype(L, userdataIdx, LUA_TUSERDATA);
    // 1. getPrivateUserdata returns NULL if passed an userdata which is not present in the
    //    poco_userdata_private table in the registry.
    // 2. in getPrivateUserdata, lua_touserdata() returns NULL when passed nil. (ie: not found)
    // 3. dynamic_cast applied to NULL results in NULL.
    derived = dynamic_cast<T*>(getPrivateUserdata(L, userdataIdx));
    if (derived == NULL) luaL_error(L, "invalid userdata, expected: %s", typeid(T).name());
    
    return derived;
}

} // LuaPoco

#endif
