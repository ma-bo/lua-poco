/// Pipe IPC
// Anonymous pipes.
// Note: pipe's are sharable between threads.
// @module pipe

#include "Pipe.h"
#include <Poco/Exception.h>
#include <Poco/Buffer.h>
#include <cstring>

int luaopen_poco_pipe(lua_State* L)
{
    LuaPoco::PipeUserdata::registerPipe(L);
    return LuaPoco::loadConstructor(L, LuaPoco::PipeUserdata::Pipe);
}

namespace LuaPoco
{

const char* POCO_PIPE_METATABLE_NAME = "Poco.Pipe.metatable";

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
    registerPipe(L);
    PipeUserdata* pud = new(lua_newuserdata(L, sizeof *pud)) PipeUserdata(mPipe);
    setupPocoUserdata(L, pud, POCO_PIPE_METATABLE_NAME);
    return true;
}

// register metatable for this class
bool PipeUserdata::registerPipe(lua_State* L)
{
    struct CFunctions methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "readBytes", readBytes },
        { "writeBytes", writeBytes },
        { "close", close },
        { NULL, NULL}
    };
    
    setupUserdataMetatable(L, POCO_PIPE_METATABLE_NAME, methods);
    return true;
}

/// Constructs a new pipe userdata
// @return userdata or nil. (error)
// @return error message.
// @function new
int PipeUserdata::Pipe(lua_State* L)
{
    PipeUserdata* pud = new(lua_newuserdata(L, sizeof *pud)) PipeUserdata();
    setupPocoUserdata(L, pud, POCO_PIPE_METATABLE_NAME);
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
// Reads and returns the specified number of bytes from the pipe.
// If no amount is specified, a single default sized read of 1024 bytes is performed.
// Whatever amount of data is flushed from the other side of the pipe is returned up to the max.
// @int[opt] amount number of bytes to read as a number.
// @return number of bytes read or nil. (error)
// @return string data read or error message.
// @function readBytes
int PipeUserdata::readBytes(lua_State* L)
{
    int rv = 0;
    lua_Integer bytesToRead = 1024;
    lua_Integer bytesRead = 0;
    lua_Integer readIndex = 0;
    bool oneshot = true;
    
    PipeUserdata* pud = checkPrivateUserdata<PipeUserdata>(L, 1);
    if (lua_gettop(L) > 1) { oneshot = false; bytesToRead = luaL_checkinteger(L, 2); }

    try
    {
        Poco::Buffer<char> buffer(bytesToRead);

        do
        {
            bytesRead = pud->mPipe.readBytes(buffer.begin() + readIndex, bytesToRead);
            readIndex += bytesRead;
            bytesToRead -= bytesRead;
            // stop looping if this is a oneshot read, the pipe was closed,
            // or the requested amount was satisfied.
        } while (!oneshot && bytesRead > 0 && bytesToRead > 0);
        
        lua_pushinteger(L, readIndex);
        lua_pushlstring(L, buffer.begin(), static_cast<size_t>(readIndex));
        
        rv = 2;
    }
    catch (const Poco::Exception& e)
    {
        rv = pushException(L, e);
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
    lua_Integer bytesWritten = 0;
    const char* str = luaL_checklstring(L, 2, &strSize);
    
    try
    {
        while (writeIndex != strSize)
        {
            bytesWritten = pud->mPipe.writeBytes(&str[writeIndex], strSize - writeIndex);
            writeIndex += bytesWritten;
        }
        
        lua_pushinteger(L, static_cast<lua_Integer>(writeIndex));
        rv = 1;
    }
    catch (const Poco::Exception& e)
    {
        rv = pushException(L, e);
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
