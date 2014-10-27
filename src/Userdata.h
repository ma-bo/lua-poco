#ifndef LUA_POCO_USERDATA_H
#define LUA_POCO_USERDATA_H

#include "LuaPoco.h"
#include "Poco/Exception.h"

#define USERDATA_PRIVATE_TABLE "poco_userdata_private"

namespace LuaPoco
{

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
    derived = dynamic_cast<T*>(getPrivateUserdata(L, userdataIdx));
    if (derived == NULL) luaL_error(L, "invalid userdata, expected: %s", typeid(T).name());
    
    return derived;
}

} // LuaPoco

#endif
