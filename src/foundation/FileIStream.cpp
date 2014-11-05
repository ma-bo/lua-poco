/// fileistream
// An istream interface for reading files.
// @module fileistream
#include "FileIStream.h"
#include "Poco/Exception.h"
#include <iostream>

int luaopen_poco_fileistream(lua_State* L)
{
    return LuaPoco::loadConstructor(L, LuaPoco::FileIStreamUserdata::FileIStream);
}

namespace LuaPoco
{

const char* POCO_FILEISTREAM_METATABLE_NAME = "Poco.FileIStream.metatable";

FileIStreamUserdata::FileIStreamUserdata(const std::string& path) 
    : mFileInputStream(path)
{
}

FileIStreamUserdata::~FileIStreamUserdata()
{
}

std::istream& FileIStreamUserdata::istream()
{
    return mFileInputStream;
}

// register metatable for this class
bool FileIStreamUserdata::registerFileIStream(lua_State* L)
{
    struct UserdataMethod methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "read", read },
        { "lines", lines },
        { "seek", seek },
        { "close", close },
        { NULL, NULL}
    };
    
    setupUserdataMetatable(L, POCO_FILEISTREAM_METATABLE_NAME, methods);
    return true;
}

/// Constructs a new fileistream userdata.
// @string path to file.
// @return userdata or nil. (error)
// @return error message.
// @function new
// @see istream
int FileIStreamUserdata::FileIStream(lua_State* L)
{
    int rv = 0;
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    const char* path = luaL_checkstring(L, firstArg);
    
    try
    {
        FileIStreamUserdata* fisud = new(lua_newuserdata(L, sizeof *fisud)) FileIStreamUserdata(path);
        setupPocoUserdata(L, fisud, POCO_FILEISTREAM_METATABLE_NAME);
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

// metamethod infrastructure
int FileIStreamUserdata::metamethod__tostring(lua_State* L)
{
    FileIStreamUserdata* fisud = checkPrivateUserdata<FileIStreamUserdata>(L, 1);
    lua_pushfstring(L, "Poco.FileIStream (%p)", static_cast<void*>(fisud));
    
    return 1;
}

// methods

/// Closes fileinputstream.
// @function close
int FileIStreamUserdata::close(lua_State* L)
{
    FileIStreamUserdata* fisud = checkPrivateUserdata<FileIStreamUserdata>(L, 1);
    fisud->mFileInputStream.close();
    
    return 0;
}

} // LuaPoco
