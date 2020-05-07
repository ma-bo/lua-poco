#include "LuaPoco.h"
#include "Loader.h"
#include "Userdata.h"

namespace LuaPoco
{

// create all metatables per class
// and load class constructors into the poco table.
bool loadMetatables(lua_State* L)
{
    bool rv = false;
    lua_getfield(L, LUA_REGISTRYINDEX, "poco.metatables.registered");
    if (lua_isnil(L, -1))
    {
            setupPrivateUserdata(L);
            lua_pushboolean(L, 1);
            lua_setfield(L, LUA_REGISTRYINDEX, "poco.metatables.registered");
            rv = true;
    }
    else
        rv = true;
    
    lua_pop(L, 1);
    
    return rv;
}

int loadConstructor(lua_State*L, lua_CFunction cons)
{
    int rv = 0;
    
    if (LuaPoco::loadMetatables(L))
    {
        lua_createtable(L, 0, 3);
        lua_pushcfunction(L, cons);
        lua_pushvalue(L, -1);
        lua_setfield(L, -3, "new");
        lua_setfield(L, -2, "__call");
        lua_pushvalue(L, -1);
        lua_setmetatable(L, -2);
        rv = 1;
    }
    else
    {
        lua_pushnil(L);
        lua_pushstring(L, "failed to create required poco metatables");
        rv = 2;
    }
    
    return rv;
}

} // LuaPoco
