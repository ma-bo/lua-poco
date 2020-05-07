/// Filesystem access.
// File attributes, listings, coping, moving, deletion, etc.
// @module file

#include "File.h"
#include "Timestamp.h"
#include <Poco/Exception.h>
#include <string>

int luaopen_poco_file(lua_State* L)
{
    LuaPoco::TimestampUserdata::registerTimestamp(L);
    LuaPoco::FileUserdata::registerFile(L);
    return LuaPoco::loadConstructor(L, LuaPoco::FileUserdata::File);
}

namespace LuaPoco
{

const char* POCO_FILE_METATABLE_NAME = "Poco.File.metatable";

FileUserdata::FileUserdata(const char* path) : mFile(path)
{
}

FileUserdata::FileUserdata(const Poco::File& file) : mFile(file)
{
}

FileUserdata::~FileUserdata()
{
}

bool FileUserdata::copyToState(lua_State* L)
{
    TimestampUserdata::registerTimestamp(L);
    registerFile(L);
    FileUserdata* fud = new(lua_newuserdata(L, sizeof *fud)) FileUserdata(mFile);
    setupPocoUserdata(L, fud, POCO_FILE_METATABLE_NAME);
    return true;
}

bool FileUserdata::registerFile(lua_State* L)
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
        { NULL, NULL}
    };
    
    setupUserdataMetatable(L, POCO_FILE_METATABLE_NAME, methods);
    
    return true;
}

// lua_Cfunctions registered in the lua_State
// Constructor

/// Constructs a new regex userdata
// @string filePath path to the file
// @return userdata or nil. (error)
// @return error message.
// @function new

int FileUserdata::File(lua_State* L)
{
    int rv = 0;
    size_t pathLen = 0;
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    const char* path = luaL_checklstring(L, firstArg, &pathLen);
    
    if (pathLen == 0)
    {
        lua_pushnil(L);
        lua_pushstring(L, "invalid path");
        return 2;
    }
    
    try
    {
        FileUserdata *fud = new(lua_newuserdata(L, sizeof *fud)) FileUserdata(path);
        setupPocoUserdata(L, fud, POCO_FILE_METATABLE_NAME);
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

///
// @type file

// metamethods
int FileUserdata::metamethod__tostring(lua_State* L)
{
    FileUserdata* fud = checkPrivateUserdata<FileUserdata>(L, 1);
    lua_pushfstring(L, "Poco.File (%p)", static_cast<void*>(fud));
    return 1;
}

// methods

/// Copies the file (or directory) to the given path. The target path can be a directory.
// A directory is copied recursively.
// @string destination path to copy the entry to.
// @return userdata or nil. (error)
// @return error message.
// @function copyTo
int FileUserdata::copyTo(lua_State* L)
{
    int rv = 0;
    FileUserdata* fud = checkPrivateUserdata<FileUserdata>(L, 1);
        
    size_t pathLen = 0;
    const char* path = luaL_checklstring(L, 2, &pathLen);
    
    if (pathLen == 0)
    {
        lua_pushnil(L);
        lua_pushstring(L, "invalid path");
        return 2;
    }
    
    try
    {
        fud->mFile.copyTo(path);
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

/// Creates a directory (and all parent directories if necessary).
// @return true or nil. (error)
// @return error message.
// @function createDirectories
int FileUserdata::createDirectories(lua_State* L)
{
    int rv = 0;
    FileUserdata* fud = checkPrivateUserdata<FileUserdata>(L, 1);
        
    try
    {
        fud->mFile.createDirectories();
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


/// Creates a directory. Returns true if the directory has been created and false if it already exists.
// @return true or nil. (error)
// @return error message.
// @function createDirectory
int FileUserdata::createDirectory(lua_State* L)
{
    int rv = 0;
    FileUserdata* fud = checkPrivateUserdata<FileUserdata>(L, 1);
    
    try
    {
        int created = fud->mFile.createDirectory();
        lua_pushboolean(L, created);
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

/// Creates a new, empty file in an atomic operation. Returns true if the file has been created and false if the file already exists.
// @return true or nil. (error)
// @return error message.
// @function createFile
int FileUserdata::createFile(lua_State* L)
{
    int rv = 0;
    FileUserdata* fud = checkPrivateUserdata<FileUserdata>(L, 1);
    
    try
    {
        int created = fud->mFile.createFile();
        lua_pushboolean(L, created);
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

/// Get a table (array) of strings for each entry in the directory.
// @return table or nil. (error)
// @return error message.
// @function listNames
int FileUserdata::listNames(lua_State* L)
{
    int rv = 0;
    FileUserdata* fud = checkPrivateUserdata<FileUserdata>(L, 1);
    
    try
    {
        std::vector<std::string> fileNames;
        fud->mFile.list(fileNames);
        int tableIndex = 1;
        lua_createtable(L, fileNames.size(), 0);
        std::vector<std::string>::iterator i = fileNames.begin();
        
        while (i != fileNames.end())
        {
            lua_pushlstring(L, i->c_str(), i->size());
            lua_rawseti(L, -2, tableIndex);
            ++i;
            ++tableIndex;
        }
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

/// Get a table (array) of file userdata for each entry in the directory.
// @return table or nil. (error)
// @return error message.
// @function listFiles
int FileUserdata::listFiles(lua_State* L)
{
    int rv = 0;
    FileUserdata* fud = checkPrivateUserdata<FileUserdata>(L, 1);
    
    try
    {
        std::vector<Poco::File> files;
        fud->mFile.list(files);
        int tableIndex = 1;
        lua_createtable(L, files.size(), 0);
        std::vector<Poco::File>::iterator i = files.begin();
        
        while (i != files.end())
        {
            FileUserdata* newFud = new(lua_newuserdata(L, sizeof *newFud)) FileUserdata(*i);
            setupPocoUserdata(L, newFud, POCO_FILE_METATABLE_NAME);
            lua_rawseti(L, -2, tableIndex);
            ++i;
            ++tableIndex;
        }
        
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

/// Copies the file (or directory) to the given path and removes the original file.
// The target path can be a directory.
// @string destination the file or directory path to move the file to.
// @return true or nil. (error)
// @return error message.
// @function moveTo
int FileUserdata::moveTo(lua_State* L)
{
    int rv = 0;
    FileUserdata* fud = checkPrivateUserdata<FileUserdata>(L, 1);
    
    size_t pathLen = 0;
    const char* path = luaL_checklstring(L, 2, &pathLen);
    
    if (pathLen == 0)
    {
        lua_pushnil(L);
        lua_pushstring(L, "invalid path");
        return 2;
    }
    
    try
    {
        fud->mFile.moveTo(path);
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

/// Delete's the file or directory.
// @bool[opt] recursive delete all sub files and directories recursively.
// @return true or nil. (error)
// @return error message.
// @function remove
int FileUserdata::remove(lua_State* L)
{
    int rv = 0;
    FileUserdata* fud = checkPrivateUserdata<FileUserdata>(L, 1);
    
    // default false as per defaul parameter to Poco::File::remove().
    int recursive = 0;
    if (lua_gettop(L) > 1)
        recursive = lua_toboolean(L, 2);
    
    try
    {
        fud->mFile.remove(recursive);
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

/// Renames the file to the new name. 
// @string destination the file or directory path to move the file to.
// @return true or nil. (error)
// @return error message.
// @function renameTo
int FileUserdata::renameTo(lua_State* L)
{
    int rv = 0;
    FileUserdata* fud = checkPrivateUserdata<FileUserdata>(L, 1);
        
    size_t pathLen = 0;
    const char* path = luaL_checklstring(L, 2, &pathLen);
    
    if (pathLen == 0)
    {
        lua_pushnil(L);
        lua_pushstring(L, "invalid path");
        return 2;
    }
    
    try
    {
        fud->mFile.renameTo(path);
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

/// Checks if the file is executable.
// On Windows and OpenVMS, the file must have the extension ".EXE" to be executable. On Unix platforms, the executable permission bit must be set.
// @return boolean
// @function canExecute
int FileUserdata::canExecute(lua_State* L)
{
    int rv = 0;
    FileUserdata* fud = checkPrivateUserdata<FileUserdata>(L, 1);
    
    try
    {
        int executable = fud->mFile.canExecute();
        lua_pushboolean(L, executable);
        rv = 1;
    }
    catch (const Poco::Exception& e)
    {
        pushPocoException(L, e);
        lua_error(L);
    }
    catch (...)
    {
        rv = pushUnknownException(L);
        lua_error(L);
    }
    
    return rv;
}

/// Checks if the file can be read.
// @return boolean
// @function canRead
int FileUserdata::canRead(lua_State* L)
{
    int rv = 0;
    FileUserdata* fud = checkPrivateUserdata<FileUserdata>(L, 1);
    
    try
    {
        int readable = fud->mFile.canRead();
        lua_pushboolean(L, readable);
        rv = 1;
    }
    catch (const Poco::Exception& e)
    {
        pushPocoException(L, e);
        lua_error(L);
    }
    catch (...)
    {
        pushUnknownException(L);
        lua_error(L);
    }
    
    return rv;
}

/// Checks if the file can be read.
// @return boolean
// @function canWrite
int FileUserdata::canWrite(lua_State* L)
{
    int rv = 0;
    FileUserdata* fud = checkPrivateUserdata<FileUserdata>(L, 1);
    
    try
    {
        int writable = fud->mFile.canWrite();
        lua_pushboolean(L, writable);
        rv = 1;
    }
    catch (const Poco::Exception& e)
    {
        pushPocoException(L, e);
        lua_error(L);
    }
    catch (...)
    {
        pushUnknownException(L);
        lua_error(L);
    }
    
    return rv;
}

/// Gets the creation date of the file userdata as a timestamp userdata.
// @see timestamp
// @return timestamp userdata or nil. (error)
// @return error message.
// @function created
int FileUserdata::created(lua_State* L)
{
    int rv = 0;
    FileUserdata* fud = checkPrivateUserdata<FileUserdata>(L, 1);
    
    try
    {
        Poco::Timestamp ts = fud->mFile.created();
        TimestampUserdata* tsud = new(lua_newuserdata(L, sizeof *tsud)) TimestampUserdata(ts);
        setupPocoUserdata(L, tsud, POCO_TIMESTAMP_METATABLE_NAME);
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

/// Checks if the file path exists.
// @return boolean
// @function exists
int FileUserdata::exists(lua_State* L)
{
    int rv = 0;
    FileUserdata* fud = checkPrivateUserdata<FileUserdata>(L, 1);
    
    try
    {
        int exists = fud->mFile.exists();
        lua_pushboolean(L, exists);
        rv = 1;
    }
    catch (const Poco::Exception& e)
    {
        pushPocoException(L, e);
        lua_error(L);
    }
    catch (...)
    {
        pushUnknownException(L);
        lua_error(L);
    }
    
    return rv;
}

/// Gets the last modified date of the file userdata as a timestamp userdata.
// @see timestamp
// @return timestamp userdata or nil. (error)
// @return error message.
// @function lastModified
int FileUserdata::getLastModified(lua_State* L)
{
    int rv = 0;
    FileUserdata* fud = checkPrivateUserdata<FileUserdata>(L, 1);
    
    try
    {
        Poco::Timestamp ts = fud->mFile.getLastModified();
        TimestampUserdata* tsud = new(lua_newuserdata(L, sizeof *tsud)) TimestampUserdata(ts);
        setupPocoUserdata(L, tsud, POCO_TIMESTAMP_METATABLE_NAME);
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

/// Gets the size of the file path entry as a number.
// @return number file size.
// @function size
int FileUserdata::getSize(lua_State* L)
{
    int rv = 0;
    FileUserdata* fud = checkPrivateUserdata<FileUserdata>(L, 1);
    
    try
    {
        lua_Number num = fud->mFile.getSize();
        lua_pushnumber(L, num);
        rv = 1;
    }
    catch (const Poco::Exception& e)
    {
        pushPocoException(L, e);
        lua_error(L);
    }
    catch (...)
    {
        pushUnknownException(L);
        lua_error(L);
    }
    
    return rv;
}

/// Checks if the file path is a device.
// @return boolean
// @function isDevice
int FileUserdata::isDevice(lua_State* L)
{
    int rv = 0;
    FileUserdata* fud = checkPrivateUserdata<FileUserdata>(L, 1);
    
    try
    {
        int device = fud->mFile.isDevice();
        lua_pushboolean(L, device);
        rv = 1;
    }
    catch (const Poco::Exception& e)
    {
        pushPocoException(L, e);
        lua_error(L);
    }
    catch (...)
    {
        pushUnknownException(L);
        lua_error(L);
    }
    
    return rv;
}

/// Checks if the file path is a directory.
// @return boolean
// @function isDirectory
int FileUserdata::isDirectory(lua_State* L)
{
    int rv = 0;
    FileUserdata* fud = checkPrivateUserdata<FileUserdata>(L, 1);
    
    try
    {
        int directory = fud->mFile.isDirectory();
        lua_pushboolean(L, directory);
        rv = 1;
    }
    catch (const Poco::Exception& e)
    {
        pushPocoException(L, e);
        lua_error(L);
    }
    catch (...)
    {
        pushUnknownException(L);
        lua_error(L);
    }
    
    return rv;
}

/// Checks if the file path is a file.
// @return boolean
// @function isFile
int FileUserdata::isFile(lua_State* L)
{
    int rv = 0;
    FileUserdata* fud = checkPrivateUserdata<FileUserdata>(L, 1);
    
    try
    {
        int file = fud->mFile.isFile();
        lua_pushboolean(L, file);
        rv = 1;
    }
    catch (const Poco::Exception& e)
    {
        pushPocoException(L, e);
        lua_error(L);
    }
    catch (...)
    {
        pushUnknownException(L);
        lua_error(L);
    }
    
    return rv;
}

/// Checks if the file path is hidden.
// On Windows platforms, the file's hidden attribute is set for this to be true.
// On Unix platforms, the file name must begin with a period for this to be true.
// @return boolean
// @function isHidden
int FileUserdata::isHidden(lua_State* L)
{
    int rv = 0;
    FileUserdata* fud = checkPrivateUserdata<FileUserdata>(L, 1);
    
    try
    {
        int hidden = fud->mFile.isHidden();
        lua_pushboolean(L, hidden);
        rv = 1;
    }
    catch (const Poco::Exception& e)
    {
        pushPocoException(L, e);
        lua_error(L);
    }
    catch (...)
    {
        pushUnknownException(L);
        lua_error(L);
    }
    
    return rv;
}

/// Checks if the file path is a symbolic link.
// @return boolean
// @function isLink
int FileUserdata::isLink(lua_State* L)
{
    int rv = 0;
    FileUserdata* fud = checkPrivateUserdata<FileUserdata>(L, 1);
    
    try
    {
        int link = fud->mFile.isLink();
        lua_pushboolean(L, link);
        rv = 1;
    }
    catch (const Poco::Exception& e)
    {
        pushPocoException(L, e);
        lua_error(L);
    }
    catch (...)
    {
        pushUnknownException(L);
        lua_error(L);
    }
    
    return rv;
}

/// Gets the file path as a string.
// @return path as a string.
// @function path
int FileUserdata::path(lua_State* L)
{
    FileUserdata* fud = checkPrivateUserdata<FileUserdata>(L, 1);
    
    std::string path = fud->mFile.path();
    lua_pushlstring(L, path.c_str(), path.size());
    
    return 1;
}

/// Makes the file executable (if flag is true), or non-executable (if flag is false).
// Does nothing on Windows and OpenVMS.
// @bool[opt] executable indicates if file should be executable. (default = true)
// @return true or nil. (error)
// @return error message.
// @function setExecutable
int FileUserdata::setExecutable(lua_State* L)
{
    int rv = 0;
    int executable = 1;
    FileUserdata* fud = checkPrivateUserdata<FileUserdata>(L, 1);
    
    if (lua_gettop(L) > 1)
        executable = lua_toboolean(L, 2);
    try
    {
        fud->mFile.setExecutable(executable);
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

/// Sets the modification date of the file path.
// @see timestamp
// @param timestamp userdata representing the last modified time.
// @return true or nil. (error)
// @return error message.
// @function setLastModified
int FileUserdata::setLastModified(lua_State* L)
{
    int rv = 0;
    FileUserdata* fud = checkPrivateUserdata<FileUserdata>(L, 1);
    TimestampUserdata* tsud = checkPrivateUserdata<TimestampUserdata>(L, 2);
    
    try
    {
        fud->mFile.setLastModified(tsud->mTimestamp);
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

/// Makes the file non-writeable (if flag is true), or writable (if flag is false).
// @bool readOnly boolean indicating if the file should be marked read only or not. (default = true)
// @return true or nil. (error)
// @return error message.
// @function setReadOnly
int FileUserdata::setReadOnly(lua_State* L)
{
    int rv = 0;
    int readOnly = 1;
    FileUserdata* fud = checkPrivateUserdata<FileUserdata>(L, 1);
    
    if (lua_gettop(L) > 1)
        readOnly = lua_toboolean(L, 2);
    
    try
    {
        fud->mFile.setReadOnly(readOnly);
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

/// sets the size of the file in bytes. Can be used to truncate a file. 
// @int size the new size of the file path.
// @return true or nil. (error)
// @return error message.
// @function setSize
int FileUserdata::setSize(lua_State* L)
{
    int rv = 0;
    FileUserdata* fud = checkPrivateUserdata<FileUserdata>(L, 1);
    lua_Number size = luaL_checknumber(L, 2);
    
    try
    {
        fud->mFile.setSize(size);
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

/// Makes the file writeable (if flag is true), or non-writeable (if flag is false).
// @bool[opt] writable boolean indicating if the file should be marked writable. (default = true)
// @return true or nil. (error)
// @return error message.
// @function setWritable
int FileUserdata::setWritable(lua_State* L)
{
    int rv = 0;
    int writable = 1;
    FileUserdata* fud = checkPrivateUserdata<FileUserdata>(L, 1);
    
    if (lua_gettop(L) > 1)
        writable = lua_toboolean(L, 2);
    
    try
    {
        fud->mFile.setWriteable(writable);
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

} // LuaPoco
