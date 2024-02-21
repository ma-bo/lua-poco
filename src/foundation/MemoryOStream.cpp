/// memoryostream
// An ostream interface for pipe userdata.
// @module memoryostream
#include "MemoryOStream.h"
#include "Buffer.h"
#include "SharedMemory.h"
#include <Poco/Exception.h>

int luaopen_poco_memoryostream(lua_State* L)
{
    LuaPoco::MemoryOStreamUserdata::registerMemoryOStream(L);
    return LuaPoco::loadConstructor(L, LuaPoco::MemoryOStreamUserdata::MemoryOStream);
}

namespace LuaPoco
{

const char* POCO_MEMORYOSTREAM_METATABLE_NAME = "Poco.MemoryOStream.metatable";

MemoryOStreamUserdata::MemoryOStreamUserdata(char* buffer, size_t bufferSize)
    : mMemoryOutputStream(buffer, bufferSize)
    , mUdReference(0)
{
}

MemoryOStreamUserdata::~MemoryOStreamUserdata()
{
}

std::ostream& MemoryOStreamUserdata::ostream()
{
    return mMemoryOutputStream;
}

// register metatable for this class
bool MemoryOStreamUserdata::registerMemoryOStream(lua_State* L)
{
    struct CFunctions methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "write", write },
        { "seek", seek },
        { "flush", flush },
        { "bytesWritten", bytesWritten },
        { NULL, NULL}
    };
    
    setupUserdataMetatable(L, POCO_MEMORYOSTREAM_METATABLE_NAME, methods);
    return true;
}

/// Constructs a new memoryostream userdata.
// memoryostream holds a reference to a buffer/sharedmemory userdata to prevent the buffer from
// being garbage collected while the memoryostream is still trying to use it.
// @tparam userdata buffer buffer userdata or sharedmemory userdata.
// @return ostream userdata or nil. (error)
// @return error message.
// @function new
// @see buffer
// @see sharedmemory
// @see ostream
int MemoryOStreamUserdata::MemoryOStream(lua_State* L)
{
    int rv = 0;
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    
    const char* errorMsg = "invalid userdata, expected: buffer userdata or sharedmemory userdata.";
    char* buffer = NULL;
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
        errorMsg = "read only sharedmemory cannot be used with memoryostream.";
        
        if (smud->mMode == Poco::SharedMemory::AM_WRITE)
        {
            buffer = smud->mSharedMemory.begin();
            bufferSize = smud->mSize;
        }
    }
    
    if (buffer && bufferSize)
    {
        try
        {
            MemoryOStreamUserdata* mosud = new(lua_newuserdata(L, sizeof *mosud))
                MemoryOStreamUserdata(buffer, bufferSize);
            setupPocoUserdata(L, mosud, POCO_MEMORYOSTREAM_METATABLE_NAME);
            // store a reference to the Buffer/SharedMemory to prevent it from being
            // garbage collected while the memoryostream is using it.
            lua_pushvalue(L, 1);
            mosud->mUdReference = luaL_ref(L, LUA_REGISTRYINDEX);
            rv = 1;
        }
        catch (const Poco::Exception& e)
        {
            rv = pushException(L, e);
        }
    }
    else
    {
        lua_pushnil(L);
        lua_pushstring(L, errorMsg);
        rv = 2;
    }
    
    return rv;
}

// metamethod infrastructure
int MemoryOStreamUserdata::metamethod__gc(lua_State* L)
{
    MemoryOStreamUserdata* mosud = checkPrivateUserdata<MemoryOStreamUserdata>(L, 1);

    luaL_unref(L, LUA_REGISTRYINDEX, mosud->mUdReference);
    mosud->~MemoryOStreamUserdata();

    return 0;
}

int MemoryOStreamUserdata::metamethod__tostring(lua_State* L)
{
    MemoryOStreamUserdata* mosud = checkPrivateUserdata<MemoryOStreamUserdata>(L, 1);
    lua_pushfstring(L, "Poco.MemoryOStream (%p)", static_cast<void*>(mosud));
    
    return 1;
}

int MemoryOStreamUserdata::bytesWritten(lua_State* L)
{
    MemoryOStreamUserdata* mosud = checkPrivateUserdata<MemoryOStreamUserdata>(L, 1);
    lua_pushinteger(L, static_cast<lua_Number>(mosud->mMemoryOutputStream.charsWritten()));
    
    return 1;
}

} // LuaPoco
