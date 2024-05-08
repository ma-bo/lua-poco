/// Temporary files.
// Temporary files that are automatically removed.  This module inherits from the File module, and
// all of the functions that operate on a file userdata apply to temporaryfile userdata.
// @module temporaryfile
// @see file

#include "TemporaryFile.h"
#include "Timestamp.h"
#include <Poco/Exception.h>
#include <string>

int luaopen_poco_temporaryfile(lua_State* L)
{
    LuaPoco::TimestampUserdata::registerTimestamp(L);
    LuaPoco::TemporaryFileUserdata::registerFile(L);
    LuaPoco::TemporaryFileUserdata::registerTemporaryFile(L);
    LuaPoco::loadConstructor(L, LuaPoco::TemporaryFileUserdata::TemporaryFile);
    
    struct LuaPoco::CFunctions standaloneFunctions[] =
    {
        { "registerForDeletion", LuaPoco::TemporaryFileUserdata::registerForDeletion },
        { "tempName", LuaPoco::TemporaryFileUserdata::tempName },
        { NULL, NULL }
    };
    LuaPoco::setCFunctions(L, standaloneFunctions);
    return 1;
}

namespace LuaPoco
{

const char* POCO_TEMPORARYFILE_METATABLE_NAME = "Poco.TemporaryFile.metatable";

TemporaryFileUserdata::TemporaryFileUserdata(const char* path) : mTemporaryFile(path)
{
}

TemporaryFileUserdata::TemporaryFileUserdata(const Poco::TemporaryFile& file) : mTemporaryFile(file)
{
}

TemporaryFileUserdata::~TemporaryFileUserdata()
{
}

bool TemporaryFileUserdata::copyToState(lua_State* L)
{
    TimestampUserdata::registerTimestamp(L);
    FileUserdata::registerFile(L);
    TemporaryFileUserdata::registerTemporaryFile(L);
    
    TemporaryFileUserdata* tfud = NULL;
    void* p = lua_newuserdata(L, sizeof *tfud);
    
    try
    {
        tfud = new(p) TemporaryFileUserdata(mTemporaryFile);
    }
    catch (const std::exception& e)
    {
        lua_pop(L, 1);
        return false;
    }
    
    setupPocoUserdata(L, tfud, POCO_TEMPORARYFILE_METATABLE_NAME);
    return true;
}

Poco::File& TemporaryFileUserdata::getFile()
{
    return mTemporaryFile;
}

bool TemporaryFileUserdata::registerTemporaryFile(lua_State* L)
{
    struct CFunctions methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "copyTo", copyTo },
        { "createDirectories", createDirectories },
        { "createDirectory", createDirectory },
        { "createFile", createFile },
        { "listNames", listNames },
        { "listFiles", listFiles },
        { "moveTo", moveTo },
        { "remove", remove },
        { "renameTo", renameTo },
        { "canExecute", canExecute },
        { "canRead", canRead },
        { "canWrite", canWrite },
        { "created", created },
        { "exists", exists },
        { "lastModified", getLastModified },
        { "size", getSize },
        { "isDevice", isDevice },
        { "isDirectory", isDirectory },
        { "isFile", isFile },
        { "isHidden", isHidden },
        { "isLink", isLink },
        { "path", path },
        { "setExecutable", setExecutable },
        { "setLastModified", setLastModified },
        { "setReadOnly", setReadOnly },
        { "setSize", setSize },
        { "setWritable", setWritable },
        { "keep", keep },
        { "keepUntilExit", keepUntilExit },
        { NULL, NULL}
    };
    
    setupUserdataMetatable(L, POCO_TEMPORARYFILE_METATABLE_NAME, methods);
    
    return true;
}

// lua_Cfunctions registered in the lua_State
// Constructor

/// Constructs a new temporaryfile userdata
// @string filePath path to the file
// @return userdata or nil. (error)
// @return error message.
// @function new
int TemporaryFileUserdata::TemporaryFile(lua_State* L)
{
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    const char* inPath = "";
    
    if (lua_isstring(L, firstArg)) { inPath = lua_tostring(L, firstArg); }
    
    TemporaryFileUserdata *tfud = NULL;
    void* p = lua_newuserdata(L, sizeof *tfud);
    
    try
    {
        tfud = new(p) TemporaryFileUserdata(inPath);
    }
    catch (const std::exception& e)
    {
        return pushException(L, e);
    }
    
    setupPocoUserdata(L, tfud, POCO_TEMPORARYFILE_METATABLE_NAME);
    return 1;
}

///
// @type temporaryfile

// metamethods
int TemporaryFileUserdata::metamethod__tostring(lua_State* L)
{
    TemporaryFileUserdata* tfud = checkPrivateUserdata<TemporaryFileUserdata>(L, 1);
    lua_pushfstring(L, "Poco.TemporaryFile (%p)", static_cast<void*>(tfud));
    return 1;
}

// methods

/// Disables automatic deletion of the file on userdata destruction.
// @function keep
int TemporaryFileUserdata::keep(lua_State* L)
{
    TemporaryFileUserdata* tfud = checkPrivateUserdata<TemporaryFileUserdata>(L, 1);
    
    tfud->mTemporaryFile.keep();
    lua_pushboolean(L, 1);
    return 0;
}

/// Registers the file for deletion at process termination instead of userdata destruction.
// @function keepUntilExit
int TemporaryFileUserdata::keepUntilExit(lua_State* L)
{
    TemporaryFileUserdata* tfud = checkPrivateUserdata<TemporaryFileUserdata>(L, 1);

    tfud->mTemporaryFile.keepUntilExit();
    lua_pushboolean(L, 1);
    return 0;
}


/// Registers the given file for deletion at process termination.
// @function registerForDeletion
int TemporaryFileUserdata::registerForDeletion(lua_State* L)
{
    const char* path = luaL_checkstring(L, 1);
    Poco::TemporaryFile::registerForDeletion(path);
    return 0;
}

/// Returns a unique path name for a temporary file in the system's scratch directory.
//  (see Path::temp()) if tempDir is empty or in the directory specified in tempDir otherwise.
// @string[opt] path create temporary name in the specified path.
// @return true or nil. (error)
// @return error message.
// @function createFile
int TemporaryFileUserdata::tempName(lua_State* L)
{
    int rv = 0;
    const char* inPath = "";

    if (lua_isstring(L, 1)) { inPath = lua_tostring(L, 1); }
    
    try
    {
        std::string name = Poco::TemporaryFile::tempName();
        lua_pushlstring(L, name.c_str(), static_cast<lua_Integer>(name.size()));
        rv = 1;
    }
    catch (const std::exception& e)
    {
        rv = pushException(L, e);
    }
        
    return rv;
}

} // LuaPoco

