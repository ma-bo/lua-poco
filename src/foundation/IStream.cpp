/// Generic interface from reading from istream userdata.
// @module istream

#include "IStream.h"
#include "Poco/Exception.h"

namespace LuaPoco
{

bool IStream::registerIStream(lua_State* L)
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

// userdata methods
int IStream::read(lua_State* L)
{
    int rv = 0;
    luaL_checktype(L, 1, LUA_TUSERDATA);
    IStream* isud = reinterpret_cast<IStream*>(lua_touserdata(L, 1));
    
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

int IStream::lines(lua_State* L)
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

int IStream::seek(lua_State* L)
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

int IStream::setvbuf(lua_State* L)
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
