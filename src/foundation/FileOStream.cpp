/// fileostream
// An ostream interface for reading files.
// @module fileostream
#include "FileOStream.h"
#include "Poco/Exception.h"

int luaopen_poco_fileostream(lua_State* L)
{
    return LuaPoco::loadConstructor(L, LuaPoco::FileOStreamUserdata::FileOStream);
}

namespace LuaPoco
{

const char* POCO_FILEOSTREAM_METATABLE_NAME = "Poco.FileOStream.metatable";

FileOStreamUserdata::FileOStreamUserdata(const std::string& path) 
    : mFileOutputStream(path)
{
}

FileOStreamUserdata::~FileOStreamUserdata()
{
}

std::ostream& FileOStreamUserdata::ostream()
{
    return mFileOutputStream;
}

// register metatable for this class
bool FileOStreamUserdata::registerFileOStream(lua_State* L)
{
    struct CFunctions methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "write", write },
        { "flush", flush },
        { "seek", seek },
        { "close", close },
        { NULL, NULL}
    };
    
    setupUserdataMetatable(L, POCO_FILEOSTREAM_METATABLE_NAME, methods);
    return true;
}

/// Constructs a new fileostream userdata.
// @string path to file.
// @return userdata or nil. (error)
// @return error message.
// @function new
// @see ostream
int FileOStreamUserdata::FileOStream(lua_State* L)
{
    int rv = 0;
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    const char* path = luaL_checkstring(L, firstArg);
    
    try
    {
        FileOStreamUserdata* fosud = new(lua_newuserdata(L, sizeof *fosud)) FileOStreamUserdata(path);
        setupPocoUserdata(L, fosud, POCO_FILEOSTREAM_METATABLE_NAME);
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
int FileOStreamUserdata::metamethod__tostring(lua_State* L)
{
    FileOStreamUserdata* fosud = checkPrivateUserdata<FileOStreamUserdata>(L, 1);
    lua_pushfstring(L, "Poco.FileOStream (%p)", static_cast<void*>(fosud));
    
    return 1;
}

// methods

/// Closes fileostream.
// @function close
int FileOStreamUserdata::close(lua_State* L)
{
    FileOStreamUserdata* fosud = checkPrivateUserdata<FileOStreamUserdata>(L, 1);
    fosud->mFileOutputStream.close();
    
    return 0;
}

} // LuaPoco
