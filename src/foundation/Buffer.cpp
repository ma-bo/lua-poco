/// Mutable buffers.
// A module that implements mutable buffers for use with memoryostream and memoryistream.
// Note: buffer userdata are copyable between threads.
// @module buffer

#include <iostream>
#include "Buffer.h"
#include "Poco/Exception.h"

int luaopen_poco_buffer(lua_State* L)
{
    LuaPoco::BufferUserdata::registerBuffer(L);
    return LuaPoco::loadConstructor(L, LuaPoco::BufferUserdata::Buffer);
}

namespace LuaPoco
{

const char* POCO_BUFFER_METATABLE_NAME = "Poco.Buffer.metatable";

BufferUserdata::BufferUserdata(size_t size) :
    mBuffer(size),
    mCapacity(size)
{
}

BufferUserdata::~BufferUserdata()
{
}

bool BufferUserdata::copyToState(lua_State *L)
{
    registerBuffer(L);
    BufferUserdata* bud = new(lua_newuserdata(L, sizeof *bud)) BufferUserdata(mCapacity);
    setupPocoUserdata(L, bud, POCO_BUFFER_METATABLE_NAME);
    bud->mCapacity = mCapacity;
    std::memcpy(bud->mBuffer.begin(), mBuffer.begin(), mCapacity);
    return true;
}

// register metatable for this class
bool BufferUserdata::registerBuffer(lua_State* L)
{
    struct CFunctions methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "clear", clear },
        { "size", size },
        { "data", data },
        { NULL, NULL }
    };

    setupUserdataMetatable(L, POCO_BUFFER_METATABLE_NAME, methods);
    
    return true;
}

/// constructs a new buffer userdata.
// @param init number specifying the size of the empty buffer in bytes, or a string
// of bytes that will be used to initialize the buffer of the same size as the string.
// @return userdata or nil. (error)
// @return error message
// @function new
int BufferUserdata::Buffer(lua_State* L)
{
    int rv = 0;
    int top = lua_gettop(L);
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    
    luaL_checkany(L, firstArg);
    size_t dataSize = 0;
    const char* dataInit = NULL;
    
    if (lua_type(L, firstArg) == LUA_TSTRING) dataInit = luaL_checklstring(L, firstArg, &dataSize);
    else dataSize = static_cast<size_t>(luaL_checknumber(L, firstArg));

    try
    {
        BufferUserdata* bud = new(lua_newuserdata(L, sizeof *bud)) BufferUserdata(dataSize);
        setupPocoUserdata(L, bud, POCO_BUFFER_METATABLE_NAME);
        
        if (dataInit) std::memcpy(bud->mBuffer.begin(), dataInit, dataSize);
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

///
// @type buffer

// metamethod infrastructure
int BufferUserdata::metamethod__tostring(lua_State* L)
{
    BufferUserdata* bud = checkPrivateUserdata<BufferUserdata>(L, 1);
    lua_pushfstring(L, "Poco.Buffer (%p)", static_cast<void*>(bud));
    return 1;
}

// userdata methods

/// Sets all bytes of the buffer to '\0'.
// @function clear
int BufferUserdata::clear(lua_State* L)
{
    BufferUserdata* bud = checkPrivateUserdata<BufferUserdata>(L, 1);
    std::memset(bud->mBuffer.begin(), '\0', bud->mBuffer.size());
    return 0;
}

/// Gets the capacity of the buffer.
// @return number indicating the capacity of the buffer.
// @function size
int BufferUserdata::size(lua_State* L)
{
    BufferUserdata* bud = checkPrivateUserdata<BufferUserdata>(L, 1);
    lua_pushinteger(L, bud->mCapacity);
    return 1;
}
/// Gets the entire buffer as a string.
// @return string containing all bytes of the buffer.
// @function data
int BufferUserdata::data(lua_State* L)
{
    BufferUserdata* bud = checkPrivateUserdata<BufferUserdata>(L, 1);
    lua_pushlstring(L, bud->mBuffer.begin(), bud->mCapacity);
    return 1;
}

} // LuaPoco
