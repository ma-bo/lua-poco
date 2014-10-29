/// Generic interface from reading from istream userdata.
// @module istream

#include "IStream.h"
#include "Poco/Exception.h"
#include <cstring>

namespace LuaPoco
{

// userdata methods

/// Reads from the istream.
// This function behaves identically to Lua file:read() function.
// @string[opt] format "*l" to read a line (default), "*n" to read a number, "*a" to read until end of file.
// @return number/string or nil. (eof/error)
// @return error
// @function read
int IStream::read(lua_State* L)
{
    int rv = 0;
    IStream* isud = checkPrivateUserdata<IStream>(L, 1);
    std::istream& is = isud->istream();
    const char* mode = "*l";
    
    if (lua_isstring(L, 2))
        mode = lua_tostring(L, 2);
    
    try
    {
        if (std::strcmp(mode, "*n") == 0)
        {
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
        }
        else if (std::strcmp(mode, "*a") == 0)
        {
            char buffer[1024];
            size_t got = 0;
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
                total += static_cast<size_t>(is.gcount());
                luaL_addlstring(&lb, buffer, static_cast<size_t>(is.gcount()));
                luaL_pushresult(&lb);
                if (total > 0)
                    rv = 1;
                else
                {
                    lua_pushnil(L);
                    lua_pushstring(L, "eof");
                    rv = 2;
                }
            }
            else
            {
                luaL_pushresult(&lb);
                lua_pop(L, 1);
                lua_pushnil(L);
                lua_pushstring(L, "error");
                rv = 2;
            }
        }
        else
        {
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
        }
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

/// Reads one line at a time from istream.
// This function behaves identically to Lua file:lines() iterator.  To be used in
// a generic for loop.
// @return line iterator function.
// @return userdata.
// @function lines.
int IStream::lines(lua_State* L)
{
    int rv = 3;
    IStream* isud = checkPrivateUserdata<IStream>(L, 1);
    
    lua_pushcfunction(L, line);
    lua_pushvalue(L, 1);
    lua_pushnil(L);
    
    return rv;
}

int IStream::line(lua_State* L)
{
    int rv = 0;
    IStream* isud = checkPrivateUserdata<IStream>(L, 1);
    std::istream& is = isud->istream();
    
    try
    {
        std::string line;
        std::getline(is, line);
        if (is.good())
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

/// Sets and gets the file position.
// Position is measured in bytes from the beginning of the file, to the position given by offset plus a base specified by the string paramter whence.
// @string[opt] whence "set" (default: beginning of file), "cur" (current positition), or "end" (end of file)
// @int[opt] offset (default: 0)
// @return number representing file position in bytes from the beginning of the file or nil. (error)
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
    luaL_newmetatable(L, "Poco.IStream.metatable");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, metamethod__gc);
    lua_setfield(L, -2, "__gc");
    lua_pushcfunction(L, metamethod__tostring);
    lua_setfield(L, -2, "__tostring");
    
    // istream methods
    lua_pushcfunction(L, read);
    lua_setfield(L, -2, "read");
    lua_pushcfunction(L, lines);
    lua_setfield(L, -2, "lines");
    lua_pushcfunction(L, seek);
    lua_setfield(L, -2, "seek");
    
    lua_pop(L, 1);
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
