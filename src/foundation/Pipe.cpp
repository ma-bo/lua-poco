/// Pipe IPC
// Anonymous pipes.
// Note: pipe's are sharable between threads.
// @module pipe

#include "Pipe.h"
#include "Poco/Exception.h"
#include <cstring>

int luaopen_poco_pipe(lua_State* L)
{
    return LuaPoco::loadConstructor(L, LuaPoco::PipeUserdata::Pipe);
}

namespace LuaPoco
{

PipeUserdata::PipeUserdata() :
    mPipe()
{
}

PipeUserdata::PipeUserdata(const Poco::Pipe& p) :
    mPipe(p)
{
}

PipeUserdata::~PipeUserdata()
{
}

bool PipeUserdata::copyToState(lua_State *L)
{
    void* ud = lua_newuserdata(L, sizeof(PipeUserdata));
    luaL_getmetatable(L, "Poco.Pipe.metatable");
    lua_setmetatable(L, -2);
    
    PipeUserdata* pud = new(ud) PipeUserdata();
    setPrivateUserdata(L, -1, pud);
    return true;
}

// register metatable for this class
bool PipeUserdata::registerPipe(lua_State* L)
{
    luaL_newmetatable(L, "Poco.Pipe.metatable");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, metamethod__gc);
    lua_setfield(L, -2, "__gc");
    lua_pushcfunction(L, metamethod__tostring);
    lua_setfield(L, -2, "__tostring");
    
    lua_pushstring(L, "Poco.Pipe.metatable");
    lua_setfield(L, -2, "poco.userdata");
    
    // methods
    lua_pushcfunction(L, readBytes);
    lua_setfield(L, -2, "readBytes");
    lua_pushcfunction(L, writeBytes);
    lua_setfield(L, -2, "writeBytes");
    lua_pushcfunction(L, close);
    lua_setfield(L, -2, "close");
    lua_pop(L, 1);
    
    return true;
}

/// Constructs a new pipe userdata
// @return userdata or nil. (error)
// @return error message.
// @function new
int PipeUserdata::Pipe(lua_State* L)
{
    void* ud = lua_newuserdata(L, sizeof(PipeUserdata));
    luaL_getmetatable(L, "Poco.Pipe.metatable");
    lua_setmetatable(L, -2);
    
    PipeUserdata* pud = new(ud) PipeUserdata();
    setPrivateUserdata(L, -1, pud);
    return 1;
}

// metamethod infrastructure
int PipeUserdata::metamethod__tostring(lua_State* L)
{
    PipeUserdata* pud = checkPrivateUserdata<PipeUserdata>(L, 1);
    
    lua_pushfstring(L, "Poco.Pipe (%p)", static_cast<void*>(pud));
    return 1;
}

// userdata methods

/// Reads bytes from pipe.
// @int[opt] amount number of bytes to read as a number.
// @return string or nil. (error)
// @return error message.
// @function readBytes
int PipeUserdata::readBytes(lua_State* L)
{
    int rv = 0;
    PipeUserdata* pud = checkPrivateUserdata<PipeUserdata>(L, 1);
    
    char readBuffer[1024];
    size_t bytesToRead = sizeof readBuffer;
    size_t bytesRead = 0;
    
    if (lua_gettop(L) > 1 && lua_isnumber(L, 2))
    {
        size_t requestedAmount = lua_tonumber(L, 2);
        bytesToRead = requestedAmount <= bytesToRead ? requestedAmount : bytesToRead;
    }
    
    try
    {
        bytesRead = pud->mPipe.readBytes(readBuffer, bytesToRead);
        luaL_Buffer lb;
        luaL_buffinit(L, &lb);
        luaL_addlstring(&lb, readBuffer, bytesRead);
        luaL_pushresult(&lb);
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

/// Writes string of bytes to pipe.
// @string string containing bytes to write.
// @return true or nil. (error)
// @return error message.
// @function writeBytes
int PipeUserdata::writeBytes(lua_State* L)
{
    int rv = 0;
    PipeUserdata* pud = checkPrivateUserdata<PipeUserdata>(L, 1);
    
    size_t writeIndex = 0;
    size_t strSize = 0;
    const char* str = luaL_checklstring(L, 2, &strSize);
    
    try
    {
        while (writeIndex != strSize)
        {
            writeIndex += pud->mPipe.writeBytes(&str[writeIndex], strSize - writeIndex);
        }
        lua_pushboolean(L, 1);
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

/// Closes pipe.
// @string[opt] end which end of the pipe to close. "read", "write", or "both" (default)
// @function close
int PipeUserdata::close(lua_State* L)
{
    int rv = 0;
    PipeUserdata* pud = checkPrivateUserdata<PipeUserdata>(L, 1);
    
    const char* closeEnd = "both";
    int top = lua_gettop(L);
    if (top > 1)
        closeEnd = luaL_checkstring(L, 2);
    
    if (std::strcmp(closeEnd, "read") == 0)
        pud->mPipe.close(Poco::Pipe::CLOSE_READ);
    else if (std::strcmp(closeEnd, "write") == 0)
        pud->mPipe.close(Poco::Pipe::CLOSE_WRITE);
    else
        pud->mPipe.close(Poco::Pipe::CLOSE_BOTH);
    
    return rv;
}

} // LuaPoco
