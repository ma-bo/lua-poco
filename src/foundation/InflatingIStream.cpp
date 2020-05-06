/// inflatingistream
// An istream filter for inflating data.
// @module inflatingistream
#include "InflatingIStream.h"
#include "Poco/Exception.h"
#include <cstring>

int luaopen_poco_inflatingistream(lua_State* L)
{
    return LuaPoco::loadConstructor(L, LuaPoco::InflatingIStreamUserdata::InflatingIStream);
}

namespace LuaPoco
{

const char* POCO_INFLATINGISTREAM_METATABLE_NAME = "Poco.InflatingIStream.metatable";

InflatingIStreamUserdata::InflatingIStreamUserdata(std::istream& istream
                                                    , Poco::InflatingStreamBuf::StreamType type
                                                    , int ref)
    : mInflatingInputStream(istream, type)
    , mUdReference(ref)
{
}

InflatingIStreamUserdata::InflatingIStreamUserdata(std::istream& istream
                                                    , int windowBits
                                                    , int ref)
    : mInflatingInputStream(istream, windowBits)
    , mUdReference(ref)
{
}

InflatingIStreamUserdata::~InflatingIStreamUserdata()
{
}

std::istream& InflatingIStreamUserdata::istream()
{
    return mInflatingInputStream;
}

// register metatable for this class
bool InflatingIStreamUserdata::registerInflatingIStream(lua_State* L)
{
    struct CFunctions methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "read", read },
        { "lines", lines },
        { "seek", seek },
        { "reset", reset},
        { NULL, NULL}
    };
    
    setupUserdataMetatable(L, POCO_INFLATINGISTREAM_METATABLE_NAME, methods);
    return true;
}

/// Constructs a new inflatingistream userdata.
// inflatingistream holds a reference to another istream userdata to prevent the userdata from
// being garbage collected while the inflatingistream is still trying to use it.
// @param istream userdata.
// @param val a number for windowBits or a string for algorithm type.
//
// "STREAM_ZLIB", "STREAM_GZIP", "STREAM_ZIP" (same as STREAM_ZLIB but no checksum is performed.)
// @return userdata or nil. (error)
// @return error message.
// @function new
// @see istream
int InflatingIStreamUserdata::InflatingIStream(lua_State* L)
{
    int rv = 0;
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    IStream* is = checkPrivateUserdata<IStream>(L, firstArg);
    
    lua_pushvalue(L, firstArg);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    
    InflatingIStreamUserdata* iisud = NULL;
    if (lua_isnumber(L, firstArg + 1))
    {
        int num = static_cast<int>(luaL_checkinteger(L, firstArg + 1));
        iisud = new(lua_newuserdata(L, sizeof *iisud)) InflatingIStreamUserdata(is->istream(), num, ref);
    }
    else
    {
        Poco::InflatingStreamBuf::StreamType type = Poco::InflatingStreamBuf::STREAM_ZLIB;
        const char* mode = luaL_checkstring(L, firstArg + 1);
        
        if (std::strcmp(mode, "STREAM_ZLIB") == 0) type = Poco::InflatingStreamBuf::STREAM_ZLIB;
        else if (std::strcmp(mode, "STREAM_GZIP") == 0) type = Poco::InflatingStreamBuf::STREAM_GZIP;
        else if (std::strcmp(mode, "STREAM_GZIP") == 0) type = Poco::InflatingStreamBuf::STREAM_ZIP;
        
        iisud = new(lua_newuserdata(L, sizeof *iisud)) InflatingIStreamUserdata(is->istream(), type, ref);
    }
    setupPocoUserdata(L, iisud, POCO_INFLATINGISTREAM_METATABLE_NAME);
    
    return 1;
}

int InflatingIStreamUserdata::metamethod__gc(lua_State* L)
{
    InflatingIStreamUserdata* iisud = checkPrivateUserdata<InflatingIStreamUserdata>(L, 1);
    
    luaL_unref(L, LUA_REGISTRYINDEX, iisud->mUdReference);
    iisud->~InflatingIStreamUserdata();
    
    return 0;
}

// metamethod infrastructure
int InflatingIStreamUserdata::metamethod__tostring(lua_State* L)
{
    InflatingIStreamUserdata* iisud = checkPrivateUserdata<InflatingIStreamUserdata>(L, 1);
    lua_pushfstring(L, "Poco.InflatingIStream (%p)", static_cast<void*>(iisud));
    
    return 1;
}

// methods

/// Resets the zlib machinery so that another zlib stream can be read from the same underlying input stream.
// @function reset
int InflatingIStreamUserdata::reset(lua_State* L)
{
    InflatingIStreamUserdata* iisud = checkPrivateUserdata<InflatingIStreamUserdata>(L, 1);
    iisud->mInflatingInputStream.reset();
    
    return 0;
}

} // LuaPoco
