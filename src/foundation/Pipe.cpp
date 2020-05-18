/// Pipe IPC
// Anonymous pipes.
// Note: pipe's are sharable between threads.
// @module pipe

#include "Pipe.h"
#include <Poco/Exception.h>
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
