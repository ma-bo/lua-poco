/// Generic interface for reading from istream userdata.
// @module istream

#include "IStream.h"
#include <Poco/Exception.h>
#include <cstring>

namespace LuaPoco
{

const char* POCO_ISTREAM_METATABLE_NAME = "Poco.IStream.metatable";

static int readAmount(lua_State* L, std::istream& is, size_t toRead)
{
    int rv = 0;
    
    if (toRead == 0)
    {
        if (is.good())
        {
            lua_pushstring(L, "");
            rv = 1;
        }
        else if (is.eof())
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
        
        return rv;
    }
    
    char buffer[1024];
    size_t total = 0;
    luaL_Buffer lb;
    luaL_buffinit(L, &lb);
    
    while (toRead > 0)
    {
        // read requested amount if it will fit in the buffer,
        // otherwise, read the size of the buffer instead.
        size_t amount = toRead < sizeof buffer ? toRead : sizeof buffer;
        
        if (is.read(buffer, amount))
        {
            luaL_addlstring(&lb, buffer, amount);
            total += amount;
            toRead -= amount;
        }
        else
            break;
    }
    
    // have read all requested bytes
    if (toRead == 0)
    {
        luaL_pushresult(&lb);
        rv = 1;
    }
    else
    {
        // check for partial read
        size_t lastAmountRead = static_cast<size_t>(is.gcount());
        if (lastAmountRead > 0) luaL_addlstring(&lb, buffer, lastAmountRead);
        
        total += lastAmountRead;
        luaL_pushresult(&lb);
        
        // have some data, add it to the buffer and return it
        if (total > 0)
        {
            rv = 1;
        }
        // have no data for this read request, return: nil, emsg
        else
        {
            // pop finalized luaL_Buffer.
            lua_pop(L, 1);
            
            lua_pushnil(L);
            const char* emsg = is.eof() ? "eof" : "error";
            lua_pushstring(L, emsg);
            rv = 2;
        }
    }
    
    return rv;
}

static int readLine(lua_State* L, std::istream& is)
{
    int rv = 0;
    
    std::string line;
    if (std::getline(is, line))
    {
        lua_pushlstring(L, line.c_str(), line.size());
        rv = 1;
    }
    else if (is.eof())
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
    
    return rv;
}

static int readAll(lua_State* L, std::istream& is)
{
    int rv = 0;
    char buffer[1024];
    size_t total = 0;
    luaL_Buffer lb;
    luaL_buffinit(L, &lb);
    
    while (is.read(buffer, sizeof buffer))
    {
        luaL_addlstring(&lb, buffer, sizeof buffer);
        total += sizeof buffer;
    }
    
    if (is.eof())
    {
        size_t lastReadAmount = static_cast<size_t>(is.gcount());
        if (lastReadAmount > 0) luaL_addlstring(&lb, buffer, lastReadAmount);
        
        total += lastReadAmount;
        luaL_pushresult(&lb);
        
        if (total > 0)
            rv = 1;
        else
        {
            // pop finalized luaL_Buffer.
            lua_pop(L, 1);
            lua_pushnil(L);
            lua_pushstring(L, "eof");
            rv = 2;
        }
    }
    else
    {
        // finalize the luaL_Buffer and pop it off.
        luaL_pushresult(&lb);
        lua_pop(L, 1);
        
        lua_pushnil(L);
        lua_pushstring(L, "error");
        rv = 2;
    }
    
    return rv;
}

static int readNumber(lua_State* L, std::istream& is)
{
    int rv = 0;
    
    lua_Number num;
    if (is >> num)
    {
        lua_pushnumber(L, num);
        rv = 1;
    }
    else if (is.eof())
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
    
    return rv;
}

// userdata methods

/// Reads from the istream.
// This function behaves identically to Lua file:read() function.
// @string[opt] format "*l" to read a line (default), "*n" to read a number, 
// "*a" to read until end of file, or a number to read the specified number of bytes.
// @return number/string or nil. (eof/error)
// @return error
// @function read
int IStream::read(lua_State* L)
{
    int rv = 0;
    IStream* isud = checkPrivateUserdata<IStream>(L, 1);
    std::istream& is = isud->istream();

    try
    {
        // check for an amount of bytes to read.
        if (lua_isnumber(L, 2)) rv = readAmount(L, is, lua_tonumber(L, 2));
        // otherwise a read mode has been specified.
        else if (lua_isstring(L, 2))
        {
            const char* mode = lua_tostring(L, 2);
            
            if (std::strcmp(mode, "*n") == 0) rv = readNumber(L, is);
            else if (std::strcmp(mode, "*a") == 0) rv = readAll(L, is);
            else if (std::strcmp(mode, "*l") == 0) rv = readLine(L, is);
            else
            {
                lua_pushnil(L);
                lua_pushstring(L, "invalid mode");
                rv = 2;
            }
        }
        // no mode specified, read a line as per the default for file:read().
        else rv = readLine(L, is);
    }
    catch (const std::exception& e)
    {
        rv = pushException(L, e);
    }
        
    return rv;
}

/// Returns a line iterator that reads one line at a time from the istream.
// This function behaves identically to Lua file:lines() iterator.  Intended to be used in
// a Lua generic for loop.
// @return line iterator function.
// @return userdata.
// @function lines
int IStream::lines(lua_State* L)
{
    int rv = 3;
    IStream* isud = checkPrivateUserdata<IStream>(L, 1);
    
    lua_pushcfunction(L, line_iterator);
    lua_pushvalue(L, 1);
    lua_pushnil(L);
    
    return rv;
}

int IStream::line_iterator(lua_State* L)
{
    int rv = 0;
    IStream* isud = checkPrivateUserdata<IStream>(L, 1);
    std::istream& is = isud->istream();
    
    try
    {
        rv = readLine(L, is);
    }
    catch (const std::exception& e)
    {
        rv = pushException(L, e);
    }
        
    return rv;
}

/// Sets and gets the istream position.
// Position is measured in bytes from the beginning of the stream, to the position given by offset plus a base specified by the string paramter whence.
// @string[opt] whence "set" (default: beginning of istream), "cur" (current positition), or "end" (end of istream)
// @int[opt] offset (default: 0)
// @return number representing istream position in bytes from the beginning of the file or nil. (error)
// @return error message.
// @function seek
int IStream::seek(lua_State* L)
{
    int rv = 0;
    IStream* isud = checkPrivateUserdata<IStream>(L, 1);
    std::istream& is = isud->istream();

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
        is.seekg(offset, whence);
        if (is.good())
        {
            lua_pushnumber(L, static_cast<lua_Number>(is.tellg()));
            rv = 1;
        }
        else if (is.eof())
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
    catch (const std::exception& e)
    {
        rv = pushException(L, e);
    }
        
    return rv;
}

IStreamUserdata::IStreamUserdata(std::istream& istream, int udReference)
    : mIStream(istream)
    , mIStreamReference(udReference)
{
}

IStreamUserdata::~IStreamUserdata()
{
}

std::istream& IStreamUserdata::istream()
{
    return mIStream;
}

// register metatable for this class
bool IStreamUserdata::registerIStream(lua_State* L)
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
    
    setupUserdataMetatable(L, POCO_ISTREAM_METATABLE_NAME, methods);
    return true;
}

// metamethod infrastructure
int IStreamUserdata::metamethod__gc(lua_State* L)
{
    IStreamUserdata* iud = checkPrivateUserdata<IStreamUserdata>(L, 1);
    
    if (iud->mIStreamReference != LUA_NOREF)
        luaL_unref(L, LUA_REGISTRYINDEX, iud->mIStreamReference);
    
    iud->~IStreamUserdata();

    return 0;
}

int IStreamUserdata::metamethod__tostring(lua_State* L)
{
    IStreamUserdata* iud = checkPrivateUserdata<IStreamUserdata>(L, 1);
    
    lua_pushfstring(L, "Poco.IStream (%p)", static_cast<void*>(iud));
    return 1;
}

} // LuaPoco
