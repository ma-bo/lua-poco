/// deflatingistream
// An istream filter for deflating data.
// @module deflatingistream
#include "DeflatingIStream.h"
#include "Poco/Exception.h"
#include <cstring>

int luaopen_poco_deflatingistream(lua_State* L)
{
    return LuaPoco::loadConstructor(L, LuaPoco::DeflatingIStreamUserdata::DeflatingIStream);
}

namespace LuaPoco
{

const char* POCO_DEFLATINGISTREAM_METATABLE_NAME = "Poco.DeflatingIStream.metatable";

DeflatingIStreamUserdata::DeflatingIStreamUserdata(std::istream& istream
                                                    , Poco::DeflatingStreamBuf::StreamType type
                                                    , int level
                                                    , int ref)
    : mDeflatingInputStream(istream, type, level)
    , mUdReference(ref)
{
}

DeflatingIStreamUserdata::DeflatingIStreamUserdata(std::istream& istream
                                                    , int windowBits
                                                    , int level,
                                                    int ref)
    : mDeflatingInputStream(istream, windowBits, level)
    , mUdReference(ref)
{
}

DeflatingIStreamUserdata::~DeflatingIStreamUserdata()
{
}

std::istream& DeflatingIStreamUserdata::istream()
{
    return mDeflatingInputStream;
}

// register metatable for this class
bool DeflatingIStreamUserdata::registerDeflatingIStream(lua_State* L)
{
    struct UserdataMethod methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "read", read },
        { "lines", lines },
        { "seek", seek },
        { NULL, NULL}
    };
    
    setupUserdataMetatable(L, POCO_DEFLATINGISTREAM_METATABLE_NAME, methods);
    return true;
}

/// Constructs a new deflatingistream userdata.
// deflatingistream holds a reference to another istream userdata to prevent the userdata from
// being garbage collected while the deflatingistream is still trying to use it.
// @param istream userdata.
// @param val a number for windowBits or a string for algorithm type.
//
// "STREAM_ZLIB", or "STREAM_GZIP".
// @int[opt] level 0 for no compression, 1 for fastest, 9 for best compression, (-1 level is default if no parameter is passed.)
// @return userdata or nil. (error)
// @return error message.
// @function new
// @see istream
int DeflatingIStreamUserdata::DeflatingIStream(lua_State* L)
{
    int rv = 0;
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    IStream* os = checkPrivateUserdata<IStream>(L, firstArg);
    
    int level = lua_isnumber(L, firstArg + 2) ? lua_tointeger(L, firstArg + 2) : -1;
    if (level < -1 || level > 9)
    {
        lua_pushnil(L);
        lua_pushstring(L, "invalid level.");
        return 2;
    }
    
    lua_pushvalue(L, firstArg);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    
    DeflatingIStreamUserdata* disud = NULL;
    if (lua_isnumber(L, firstArg + 1))
    {
        int num = static_cast<int>(luaL_checkint(L, firstArg + 1));
        disud = new(lua_newuserdata(L, sizeof *disud)) DeflatingIStreamUserdata(os->istream(), num, level, ref);
    }
    else
    {
        Poco::DeflatingStreamBuf::StreamType type = Poco::DeflatingStreamBuf::STREAM_ZLIB;
        const char* mode = luaL_checkstring(L, firstArg + 1);
        
        if (std::strcmp(mode, "STREAM_ZLIB") == 0) type = Poco::DeflatingStreamBuf::STREAM_ZLIB;
        else if (std::strcmp(mode, "STREAM_GZIP") == 0) type = Poco::DeflatingStreamBuf::STREAM_GZIP;
        
        disud = new(lua_newuserdata(L, sizeof *disud)) DeflatingIStreamUserdata(os->istream(), type, level, ref);
    }
    setupPocoUserdata(L, disud, POCO_DEFLATINGISTREAM_METATABLE_NAME);
    
    return 1;
}

int DeflatingIStreamUserdata::metamethod__gc(lua_State* L)
{
    DeflatingIStreamUserdata* disud = checkPrivateUserdata<DeflatingIStreamUserdata>(L, 1);
    
    luaL_unref(L, LUA_REGISTRYINDEX, disud->mUdReference);
    disud->~DeflatingIStreamUserdata();
    
    return 0;
}

// metamethod infrastructure
int DeflatingIStreamUserdata::metamethod__tostring(lua_State* L)
{
    DeflatingIStreamUserdata* disud = checkPrivateUserdata<DeflatingIStreamUserdata>(L, 1);
    lua_pushfstring(L, "Poco.DeflatingIStream (%p)", static_cast<void*>(disud));
    
    return 1;
}

} // LuaPoco
