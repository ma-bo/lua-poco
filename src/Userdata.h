#ifndef LUA_POCO_USERDATA_H
#define LUA_POCO_USERDATA_H

#include "LuaPoco.h"
#include "Poco/Exception.h"

#define USERDATA_PRIVATE_TABLE "poco_userdata_private"

namespace LuaPoco
{

// base class for all userdata
// the idea is that it will be possible to pass a userdata to another 
// userdata's method, and these types this will enable base class 
// compatability testing.
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

// setup private table for Derived pointer to Userdata pointer mapping.
void setupDerivedToUserdata(lua_State* L);
// stores Userdata pointer in a private table with the derived userdata as a weak key.
void setDerivedtoUserdata(lua_State* L, int userdataIdx, Userdata* ud);
// retrieves the associated Userdata pointer for the derived pointer.
// (provides mechanism for type safe dynamic_cast from Userdata pointer back to the Derived pointer.)
Userdata* getDerivedToUserdata(lua_State* L, int userdataIdx);
// clears the association of the derived pointer with the base userdata pointer from the private table.
void setDerivedtoUserdata(lua_State* L, int userdataIdx);

// function to validate that a Userdata* can be dynamic_cast to Derived* and lua_error() if not.
template <typename T>
T* checkDerivedFromUserdata(lua_State* L, int userdataIdx)
{
    T* derived = NULL;
    userdataIdx = userdataIdx < 0 ? lua_gettop(L) + 1 + userdataIdx : userdataIdx;
    
    luaL_checktype(L, userdataIdx, LUA_TUSERDATA);
    derived = dynamic_cast<T*>(getDerivedToUserdata(L, userdataIdx));
    if (derived == NULL) luaL_error(L, "invalid userdata, expected: %s", typeid(T).name());
    
    return derived;
}

} // LuaPoco

#endif
