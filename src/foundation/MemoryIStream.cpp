/// memoryistream
// An istream interface for pipe userdata.
// @module memoryistream
#include "MemoryIStream.h"
#include "Buffer.h"
#include "SharedMemory.h"
#include <Poco/Exception.h>

int luaopen_poco_memoryistream(lua_State* L)
{
    LuaPoco::MemoryIStreamUserdata::registerMemoryIStream(L);
    return LuaPoco::loadConstructor(L, LuaPoco::MemoryIStreamUserdata::MemoryIStream);
}

namespace LuaPoco
{

const char* POCO_MEMORYISTREAM_METATABLE_NAME = "Poco.MemoryIStream.metatable";

MemoryIStreamUserdata::MemoryIStreamUserdata(const char* buffer, size_t bufferSize)
    : mMemoryInputStream(buffer, bufferSize)
    , mUdReference(0)
{
}

MemoryIStreamUserdata::~MemoryIStreamUserdata()
{
}

std::istream& MemoryIStreamUserdata::istream()
{
    return mMemoryInputStream;
}

// register metatable for this class
bool MemoryIStreamUserdata::registerMemoryIStream(lua_State* L)
{
    struct CFunctions methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "read", read },
        { "lines", lines },
        { "seek", seek },
        { NULL, NULL}
    };
    
    setupUserdataMetatable(L, POCO_MEMORYISTREAM_METATABLE_NAME, methods);
    return true;
}

/// Constructs a new memoryistream userdata.
// memoryistream holds a reference to a buffer/sharedmemory userdata to prevent the buffer from
// being garbage collected while the memoryistream is still trying to use it.
// @tparam userdata buffer buffer userdata or sharedmemory userdata.
// @return istream userdata or nil. (error)
// @return error message.
// @function new
// @see buffer
// @see sharedmemory
// @see istream
int MemoryIStreamUserdata::MemoryIStream(lua_State* L)
{
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    
    const char* errorMsg = "invalid userdata, expected: buffer userdata or sharedmemory userdata.";
    const char* buffer = NULL;
    size_t bufferSize = 0;
    Userdata* ud = getPrivateUserdata(L, firstArg);
    
    BufferUserdata* bud = dynamic_cast<BufferUserdata*>(ud);
    SharedMemoryUserdata* smud = dynamic_cast<SharedMemoryUserdata*>(ud);
    
    if (bud)
    {
        buffer = bud->mBuffer.begin();
        bufferSize = bud->mCapacity;
    }
    else if (smud)
    {
        buffer = smud->mSharedMemory.begin();
        bufferSize = smud->mSize;
    }
    
    if (buffer == NULL || bufferSize == 0)
    {
        lua_pushnil(L);
        lua_pushstring(L, errorMsg);
        return 2;
    }
       
    MemoryIStreamUserdata* misud = NULL;
    void* p = lua_newuserdata(L, sizeof *misud);
    
    try
    {
        misud = new(p) MemoryIStreamUserdata(buffer, bufferSize);
    }
    catch (const std::exception& e)
    {
        return pushException(L, e);
    }
    
    // store a reference to the Buffer/SharedMemory to prevent it from being
    // garbage collected while the memoryostream is using it.
    lua_pushvalue(L, firstArg);
    misud->mUdReference = luaL_ref(L, LUA_REGISTRYINDEX);
    
    setupPocoUserdata(L, misud, POCO_MEMORYISTREAM_METATABLE_NAME);
    return 1;
}

int MemoryIStreamUserdata::metamethod__gc(lua_State* L)
{
    MemoryIStreamUserdata* misud = checkPrivateUserdata<MemoryIStreamUserdata>(L, 1);

    luaL_unref(L, LUA_REGISTRYINDEX, misud->mUdReference);
    misud->~MemoryIStreamUserdata();

    return 0;
}

// metamethod infrastructure
int MemoryIStreamUserdata::metamethod__tostring(lua_State* L)
{
    MemoryIStreamUserdata* misud = checkPrivateUserdata<MemoryIStreamUserdata>(L, 1);
    lua_pushfstring(L, "Poco.MemoryIStream (%p)", static_cast<void*>(misud));
    
    return 1;
}

} // LuaPoco
