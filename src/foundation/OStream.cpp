/// Generic interface from reading from ostream userdata.
// @module ostream

#include "OStream.h"
#include "Poco/Exception.h"
#include <cstring>

namespace LuaPoco
{

const char* POCO_OSTREAM_METATABLE_NAME = "Poco.OStream.metatable";

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

    std::ios_base::seekdir whence = std::ios_base::beg;
    const char *mode = "set";
    size_t offset = 0;
    
    if (lua_gettop(L) == 2)
    {
        if (lua_isstring(L, 2)) mode = lua_tostring(L, 2);
        else if (lua_isnumber(L, 2)) offset = static_cast<size_t>(lua_tonumber(L, 2));
        else luaL_error(L, "invalid type for parameter 2, %s", lua_typename(L, lua_type(L, 2)));
    }
    else if (lua_gettop(L) == 3)
    {
        mode = luaL_checkstring(L, 2);
        offset = static_cast<size_t>(luaL_checknumber(L, 3));
    }
    
    if (std::strcmp(mode, "cur") == 0)
        whence = std::ios_base::cur;
    else if (std::strcmp(mode, "end") == 0)
        whence = std::ios_base::end;
    
    try
    {
        os.seekp(offset, whence);
        if (os.good())
        {
            lua_pushnumber(L, static_cast<lua_Number>(os.tellp()));
            rv = 1;
        }
        else if (os.eof())
        {
            lua_pushnil(L);
            lua_pushstring(L, "eof");
            rv = 2;
        }
        else
        {
            lua_pushnil(L);
            lua_pushstring(L, "error");
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


OStreamUserdata::OStreamUserdata(std::ostream& ostream, int udReference)
    : mOStream(ostream)
    , mOStreamReference(udReference)
{
}

OStreamUserdata::~OStreamUserdata()
{
}

std::ostream& OStreamUserdata::ostream()
{
    return mOStream;
}

// register metatable for this class
bool OStreamUserdata::registerOStream(lua_State* L)
{
    struct UserdataMethod methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "write", write },
        { "seek", seek },
        { "flush", flush },
        { NULL, NULL}
    };
    
    setupUserdataMetatable(L, POCO_OSTREAM_METATABLE_NAME, methods);
    return true;
}

// metamethod infrastructure
int OStreamUserdata::metamethod__gc(lua_State* L)
{
    OStreamUserdata* oud = checkPrivateUserdata<OStreamUserdata>(L, 1);
    
    if (oud->mOStreamReference != LUA_NOREF)
        luaL_unref(L, LUA_REGISTRYINDEX, oud->mOStreamReference);
    
    oud->~OStreamUserdata();

    return 0;
}

int OStreamUserdata::metamethod__tostring(lua_State* L)
{
    OStreamUserdata* oud = checkPrivateUserdata<OStreamUserdata>(L, 1);
    
    lua_pushfstring(L, "Poco.OStream (%p)", static_cast<void*>(oud));
    return 1;
}

} // LuaPoco
