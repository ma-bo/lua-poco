/// Generic interface from reading from ostream userdata.
// @module ostream

#include "OStream.h"
#include "Poco/Exception.h"

namespace LuaPoco
{

static OStream* checkOStream(lua_State* L)
{
    OStream* osud = NULL;
    luaL_checktype(L, 1, LUA_TUSERDATA);
    // check for metatable attached to userdata
    if (lua_getmetatable(L, 1))
    {
        // check if it is 
        lua_getfield(L, -1, "poco.userdata");
        if (lua_isnil(L, -1))
            luaL_error(L, "userdata is not a poco userdata.");
        lua_pop(L, 2);
        //broken due to casting from a void* (derived class) to a base class.
        //Userdata* ud = reinterpret_cast<Userdata*>(lua_touserdata(L, 1));
        //if (ud->getBaseType() != BaseType_OStream)
            //luaL_error(L, "userdata is not an ostream userdata.");
        //osud = dynamic_cast<OStream*>(ud);
    }
    
    return osud;
}

// userdata methods
int OStream::write(lua_State* L)
{
    int rv = 0;
    OStream* osud = checkOStream(L);
    std::ostream& os = osud->ostream();
    luaL_checkany(L, 2);
    
    try
    {
        size_t dataLen = 0;
        const char* data = lua_tolstring(L, 2, &dataLen);
        if (data)
        {
            os.write(data, dataLen);
            if (!os.fail())
            {
                lua_pushboolean(L, 1);
                rv = 1;
            }
            else
            {
                lua_pushboolean(L, 0);
                lua_pushstring(L, "write failed.");
                rv = 2;
            }
        }
        else
        {
            lua_pushnil(L);
            lua_pushstring(L, "cannot convert second argument to string.");
            rv = 2;
        }
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

/// Flushes any written data to stream.
// @return true or nil. (error)
// @return error message.
// @function flush
int OStream::flush(lua_State* L)
{
    int rv = 0;
    OStream* osud = checkOStream(L);
    std::ostream& os = osud->ostream();
    
    try
    {
        std::ostream& os = osud->ostream();
        os.flush();
        if (!os.fail())
        {
            lua_pushboolean(L, 1);
            rv = 1;
        }
        else
        {
            lua_pushboolean(L, 0);
            lua_pushstring(L, "flush failed.");
            rv = 2;
        }
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

/// Sets and gets the file position.
// Position is measured in bytes from the beginning of the file, to the position given by offset plus a base specified by the string paramter whence.
// @string[opt] whence "set" (default: beginning of file), "cur" (current positition), or "end" (end of file)
// @int[opt] offset (default: 0)
// @return number representing file position in bytes from the beginning of the file or nil. (error)
// @return error message.
// @function seek
int OStream::seek(lua_State* L)
{
    int rv = 0;
    OStream* osud = checkOStream(L);
    std::ostream& os = osud->ostream();
    
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
