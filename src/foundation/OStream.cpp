/// Generic interface from reading from ostream userdata.
// @module ostream

#include "OStream.h"
#include "Poco/Exception.h"

namespace LuaPoco
{

// userdata methods
int OStream::write(lua_State* L)
{
    int rv = 0;
    OStream* osud = checkPrivateUserdata<OStream>(L, 1);
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
    OStream* osud = checkPrivateUserdata<OStream>(L, 1);
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
    OStream* osud = checkPrivateUserdata<OStream>(L, 1);
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
