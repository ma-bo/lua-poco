/// Generic interface from reading from ostream userdata.
// @module ostream

#include "OStream.h"
#include "Poco/Exception.h"

namespace LuaPoco
{

bool OStreamUserdata::registerOStream(lua_State* L)
{
    luaL_newmetatable(L, "Poco.OStream.metatable");
    lua_pushvalue(L, -1);
    
    // methods
    lua_pushcfunction(L, write);
    lua_setfield(L, -2, "write");
    lua_pushcfunction(L, flush);
    lua_setfield(L, -2, "flush");
    lua_pushcfunction(L, seek);
    lua_setfield(L, -2, "seek");
    lua_pushcfunction(L, close);
    lua_setfield(L, -2, "close");
    lua_pop(L, 1);
    
    return true;
}

static OStreamUserdata* checkOStream(lua_State* L)
{
    OStreamUserdata* osud = NULL;
    luaL_checktype(L, 1, LUA_TUSERDATA);
    // check for metatable attached to userdata
    if (lua_getmetatable(L, 1))
    {
        // check if it is 
        lua_getfield(L, -1, "poco.userdata");
        if (lua_isnil(L, -1))
            luaL_error(L, "userdata is not an ostream userdata.");
        lua_pop(L, 2);
        Userdata* ud = reinterpret_cast<Userdata*>(lua_touserdata(L, 1));
        if (ud->getBaseType() != BaseType_OStream)
            luaL_error(L, "userdata is not an ostream userdata.");
        osud = reinterpret_cast<OStreamUserdata*>(ud);
    }
    
    return osud;
}

// userdata methods
int OStreamUserdata::write(lua_State* L)
{
    int rv = 0;
    OStreamUserdata* osud = checkOStream(L);
    
    try
    {
        std::ostream& is = osud->getHandle();
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

int OStreamUserdata::flush(lua_State* L)
{
    int rv = 0;
    OStreamUserdata* osud = checkOStream(L);
    
    try
    {
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

int OStreamUserdata::seek(lua_State* L)
{
    int rv = 0;
    OStreamUserdata* osud = checkOStream(L);
    
    try
    {
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

int OStreamUserdata::close(lua_State* L)
{
    int rv = 0;
    OStreamUserdata* osud = checkOStream(L);
    
    try
    {
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

} // LuaPoco
