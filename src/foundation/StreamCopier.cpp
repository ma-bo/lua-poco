/// StreamCopier copying istreams to ostreams.
// @module streamcopier

#include "StreamCopier.h"
#include "Userdata.h"
#include "LuaPocoUtils.h"
#include "IStream.h"
#include "OStream.h"
#include <Poco/StreamCopier.h>

int luaopen_poco_streamcopier(lua_State* L)
{
    struct LuaPoco::CFunctions methods[] =
    {
        { "copyStream", LuaPoco::StreamCopier::copyStream },
        { "copyStreamUnbuffered", LuaPoco::StreamCopier::copyStreamUnbuffered },
        { "copyToString", LuaPoco::StreamCopier::copyToString },
        { NULL, NULL}
    };

    lua_createtable(L, 0, 3);
    setCFunctions(L, methods);

    return 1;
}

namespace LuaPoco
{

/// Copies all data from an istream to an ostream.
// @tparam userdata istream source of data to copy from.
// @tparam userdata ostream destination for data to be copied to.
// @tparam[opt] number buffer_size internal buffer size to use. (default: 8192)
// @return bytes number of bytes copied, or nil. (error)
// @return error message.
// @function copyStream
int StreamCopier::copyStream(lua_State* L)
{
    int rv = 0;
    size_t bufferSize = 8192;
    
    IStream* isud = checkPrivateUserdata<IStream>(L, 1);
    OStream* osud = checkPrivateUserdata<OStream>(L, 2);
    int top = lua_gettop(L);
    if (top > 2) { bufferSize = static_cast<size_t>(luaL_checkinteger(L, 3)); }

    try
    {
        lua_Integer result = 0;
        Poco::UInt64 bytesCopied = Poco::StreamCopier::copyStream64(isud->istream(), osud->ostream(), bufferSize);
        if (checkUnsignedToLuaInteger<Poco::UInt64>(bytesCopied, result))
        {
            lua_pushinteger(L, result);
            rv = 1;
        }
        else
        {
            lua_pushnil(L);
            lua_pushstring(L, "bytesCopied value is out of range for lua_Integer.");
            rv = 2;
        }
    }
    catch (const std::exception& e)
    {
        rv = pushException(L, e);
    }
    
    return rv;
}

int copyToString (lua_State* L);

/// Copies all data from an istream to an ostream without internal buffering.
// @tparam userdata istream source of data to copy from.
// @tparam userdata ostream destination for data to be copied to.
// @return bytes number of bytes copied, or nil. (error)
// @return error message.
// @function copyStream
int StreamCopier::copyStreamUnbuffered(lua_State* L)
{
    int rv = 0;
    
    IStream* isud = checkPrivateUserdata<IStream>(L, 1);
    OStream* osud = checkPrivateUserdata<OStream>(L, 2);

    try
    {
        lua_Integer result = 0;
        Poco::UInt64 bytesCopied = Poco::StreamCopier::copyStreamUnbuffered64(isud->istream(), osud->ostream());
        if (checkUnsignedToLuaInteger<Poco::UInt64>(bytesCopied, result))
        {
            lua_pushinteger(L, result);
            rv = 1;
        }
        else
        {
            lua_pushnil(L);
            lua_pushstring(L, "bytesCopied value is out of range for lua_Integer.");
            rv = 2;
        }
    }
    catch (const std::exception& e)
    {
        rv = pushException(L, e);
    }
    
    return rv;
}

/// Copies an istream's data to a string.
// @tparam userdata istream source of data to copy from.
// @tparam[opt] number buffer_size internal buffer size to use. (default: 8192)
// @return string containing data copied or nil. (error)
// @return error message.
// @function copyToString
int StreamCopier::copyToString(lua_State* L)
{
    int rv = 0;
    size_t bufferSize = 8192;
    
    IStream* isud = checkPrivateUserdata<IStream>(L, 1);
    int top = lua_gettop(L);
    if (top > 1) { bufferSize = static_cast<size_t>(luaL_checkinteger(L, 2)); }

    try
    {
        lua_Integer result = 0;
        std::string data;
        
        Poco::UInt64 bytesCopied = Poco::StreamCopier::copyToString(isud->istream(), data);
        if (checkUnsignedToLuaInteger<Poco::UInt64>(bytesCopied, result))
        {
            lua_pushlstring(L, data.c_str(), static_cast<size_t>(result));
            rv = 1;
        }
        else
        {
            lua_pushnil(L);
            lua_pushstring(L, "bytesCopied value is out of range for lua_Integer.");
            rv = 2;
        }
    }
    catch (const std::exception& e)
    {
        rv = pushException(L, e);
    }
    
    return rv;
}

} // LuaPoco
