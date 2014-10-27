#include "Userdata.h"

namespace LuaPoco
{

int pushPocoException(lua_State* L, const Poco::Exception& e)
{
    lua_pushnil(L);
    lua_pushlstring(L, e.displayText().c_str(), e.displayText().size());
    return 2;
}

int pushUnknownException(lua_State* L)
{
    lua_pushnil(L);
    lua_pushstring(L, "unknown Exception");
    return 2;
}

// public member functions
Userdata::Userdata()
{
}

Userdata::~Userdata()
{
}

bool Userdata::copyToState(lua_State *L)
{
    return false;
}
// @type checksum
int Userdata::metamethod__gc(lua_State* L)
{
    Userdata* ud = getPrivateUserdata(L, 1);
    ud->~Userdata();
    return 0;
}

void setupPrivateUserdata(lua_State* L)
{
    // start private table with 20 slots hash table slots for userdata.
    lua_createtable(L, 0, 20);
    // set weak key mode on table.
    lua_pushstring(L, "__mode");
    lua_pushstring(L, "k");
    lua_settable(L, -3);
    // set the table to be its own metatable
    lua_pushvalue(L, -1);
    lua_setmetatable(L, -2);
    lua_setfield(L, LUA_REGISTRYINDEX, USERDATA_PRIVATE_TABLE);
}

// stores Userdata pointer in a private table with the derived userdata as a weak key.
void setPrivateUserdata(lua_State* L, int userdataIdx, Userdata* ud)
{
    // convert negative relative index to positive.
    userdataIdx = userdataIdx < 0 ? lua_gettop(L) + 1 + userdataIdx : userdataIdx;
    // retrieve the private userdata table.
    lua_getfield(L, LUA_REGISTRYINDEX, USERDATA_PRIVATE_TABLE);
    // private_table[derived_userdata] = base_light_userdata
    lua_pushvalue(L, userdataIdx);
    lua_pushlightuserdata(L, ud);
    lua_settable(L, -3);
    // remove private table from stack
    lua_pop(L, 1);
}
// retrieves the associated Userdata pointer for the derived pointer.
// (provides mechanism for type safe dynamic_cast from Userdata pointer back to the Derived pointer.)
Userdata* getPrivateUserdata(lua_State* L, int userdataIdx)
{
    Userdata* ud = NULL;
    userdataIdx = userdataIdx < 0 ? lua_gettop(L) + 1 + userdataIdx : userdataIdx;
    lua_getfield(L, LUA_REGISTRYINDEX, USERDATA_PRIVATE_TABLE);
    lua_pushvalue(L, userdataIdx);
    lua_gettable(L, -2);
    ud = static_cast<Userdata*>(lua_touserdata(L, -1));
    // pop lightuserdata and private table.
    lua_pop(L, 2);
    return ud;
}

} // LuaPoco
