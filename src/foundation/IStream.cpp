/// Generic interface from reading from istream userdata.
// @module istream

#include "IStream.h"
#include "Poco/Exception.h"

namespace LuaPoco
{

bool IStreamUserdata::registerIStream(lua_State* L)
{
    luaL_newmetatable(L, "Poco.IStream.metatable");
    lua_pushvalue(L, -1);
    
    // methods
    lua_pushcfunction(L, read);
    lua_setfield(L, -2, "read");
    lua_pushcfunction(L, lines);
    lua_setfield(L, -2, "lines");
    lua_pushcfunction(L, seek);
    lua_setfield(L, -2, "seek");
    lua_pushcfunction(L, setvbuf);
    lua_setfield(L, -2, "setvbuf");
    lua_pop(L, 1);
    
    return true;
}

static IStreamUserdata* checkIStream(lua_State* L)
{
    IStreamUserdata* isud = NULL;
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
        if (ud->getBaseType() != BaseType_IStream)
            luaL_error(L, "userdata is not an ostream userdata.");
        isud = reinterpret_cast<IStreamUserdata*>(ud);
    }
    
    return isud;
}

BaseType IStreamUserdata::getBaseType()
{
    return BaseType_IStream;
}

// userdata methods
int IStreamUserdata::read(lua_State* L)
{
    int rv = 0;
    IStreamUserdata* isud = checkIStream(L);
    
    try
    {
        std::istream& is = isud->getHandle();
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

int IStreamUserdata::lines(lua_State* L)
{
    int rv = 0;
    IStreamUserdata* isud = checkIStream(L);
    
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

int IStreamUserdata::seek(lua_State* L)
{
    int rv = 0;
    IStreamUserdata* isud = checkIStream(L);
    
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

int IStreamUserdata::setvbuf(lua_State* L)
{
    int rv = 0;
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
