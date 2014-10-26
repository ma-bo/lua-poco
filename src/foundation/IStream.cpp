/// Generic interface from reading from istream userdata.
// @module istream

#include "IStream.h"
#include "Poco/Exception.h"

namespace LuaPoco
{

static IStream* checkIStream(lua_State* L)
{
    IStream* isud = NULL;
    luaL_checktype(L, 1, LUA_TUSERDATA);
    // check for metatable attached to userdata
    if (lua_getmetatable(L, 1))
    {
        // check if it is 
        lua_getfield(L, -1, "poco.userdata");
        if (lua_isnil(L, -1))
            luaL_error(L, "userdata is not a poco userdata.");
        lua_pop(L, 2);
        // broken due to casting from void* to a base class.
        // Userdata* ud = reinterpret_cast<Userdata*>(lua_touserdata(L, 1));
        // if (ud->getBaseType() != BaseType_IStream)
        //     luaL_error(L, "userdata is not an istream userdata.");
        // isud = reinterpret_cast<IStream*>(ud);
    }
    
    return isud;
}

// userdata methods
int IStream::read(lua_State* L)
{
    int rv = 0;
    // this is broken due to casting from void* via Lua to base class type.
    // IStream* isud = checkIStream(L);
    // std::istream& is = isud->istream();
    
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

int IStream::lines(lua_State* L)
{
    int rv = 0;
    // this is broken due to casting from void* via Lua to base class type.
    // IStream* isud = checkIStream(L);
    // std::istream& is = isud->istream();
    
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
    // this is broken due to casting from void* via Lua to base class type.
    // IStream* isud = checkIStream(L);
    // std::istream& is = isud->istream();
    
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
    // this is broken due to casting from void* via Lua to base class type.
    // IStream* isud = checkIStream(L);
    // std::istream& is = isud->istream();
    
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
