/// Filesystem Paths.
// This module represents filesystem paths in a platform-independent manner.
// Unix, Windows and OpenVMS all use a different syntax for filesystem paths.
// This class can work with all three formats.
// A path is made up of an optional node name (only Windows and OpenVMS),
// an optional device name (also only Windows and OpenVMS),
// a list of directory names and an optional filename.
//
// On windows, nodes represent the hostname of an UNC path.
//
// Note: path userdata are copyable between threads.
// @module path

#include "Userdata.h"
#include "Path.h"
#include <Poco/Exception.h>
#include <cstring>

int luaopen_poco_path(lua_State* L)
{
    struct LuaPoco::CFunctions methods[] =
    {
        { "current", LuaPoco::PathUserdata::current },
        { "expand", LuaPoco::PathUserdata::expand },
        { "find", LuaPoco::PathUserdata::find },
        { "home", LuaPoco::PathUserdata::home },
        { "listRoots", LuaPoco::PathUserdata::listRoots },
        { "nullDevice", LuaPoco::PathUserdata::nullDevice },
        { "pathSeparator", LuaPoco::PathUserdata::pathSeparator },
        { "separator", LuaPoco::PathUserdata::separator },
        { "temp", LuaPoco::PathUserdata::temp },
        { "transcode", LuaPoco::PathUserdata::transcode },
        { NULL, NULL}
    };

    int rv = LuaPoco::loadConstructor(L, LuaPoco::PathUserdata::Path);
    if (rv == 1) { setCFunctions(L, methods); }

    LuaPoco::PathUserdata::registerPath(L);

    return rv;
}

namespace LuaPoco
{

const char* POCO_PATH_METATABLE_NAME = "Poco.Path.metatable";

PathUserdata::PathUserdata(const char* path, Poco::Path::Style style, bool absolute) :
    mPath(absolute)
{
    mPath.assign(path, style);
}

PathUserdata::PathUserdata(Poco::Path path) :
    mPath(path)
{
}

PathUserdata::~PathUserdata()
{
}

bool PathUserdata::copyToState(lua_State *L)
{
    registerPath(L);
    PathUserdata* pud = new(lua_newuserdata(L, sizeof *pud)) PathUserdata(mPath);
    setupPocoUserdata(L, pud, POCO_PATH_METATABLE_NAME);
    return true;
}

// register metatable for this class
bool PathUserdata::registerPath(lua_State* L)
{
    struct CFunctions methods[] =
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "absolute", absolute },
        { "append", append },
        { "clear", clear },
        { "depth", depth },
        { "directory", directory },
        { "getBaseName", getBaseName },
        { "getDevice", getDevice },
        { "getExtension", getExtension },
        { "getFileName", getFileName },
        { "getNode", getNode },
        { "isAbsolute", isAbsolute },
        { "isDirectory", isDirectory },
        { "isFile", isFile },
        { "isRelative", isRelative },
        { "makeAbsolute", makeAbsolute },
        { "makeFile", makeFile },
        { "makeParent", makeParent },
        { "parent", parent },
        { "popDirectory", popDirectory },
        { "popFrontDirectory", popFrontDirectory },
        { "pushDirectory", pushDirectory },
        { "setBaseName", setBaseName },
        { "setDevice", setDevice },
        { "setExtension", setExtension },
        { "setFileName", setFileName },
        { "setNode", setNode },
        { "toString", toString },
        { NULL, NULL}
    };

    setupUserdataMetatable(L, POCO_PATH_METATABLE_NAME, methods);
    return true;
}

/// Constructs a new path userdata
// @string path A path specifying a file or directory.
// @string[opt] style Style options: UNIX, WINDOWS, VMS, NATIVE, or GUESS.
// NATIVE is the default and uses the host platform's style.
// GUESS will attempt to correctly identify the platform specified by the path.
// @bool[opt] absolute Specify true for an absolute path, false for relative.  Relative is the default.
// @return userdata or nil. (error)
// @return error message.
// @function new
int PathUserdata::Path(lua_State* L)
{
    int rv = 0;
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    const char* pathStr = luaL_checkstring(L, firstArg);
    Poco::Path::Style style = Poco::Path::PATH_NATIVE;
    bool absolute = false;

    int top = lua_gettop(L);
    // new(path, style)
    if (top > firstArg)
    {
        const char* styleStr = luaL_checkstring(L, firstArg + 1);
        if (std::strcmp(styleStr, "UNIX") == 0) style = Poco::Path::PATH_UNIX;
        else if (std::strcmp(styleStr, "WINDOWS") == 0) style = Poco::Path::PATH_WINDOWS;
        else if (std::strcmp(styleStr, "VMS") == 0) style = Poco::Path::PATH_WINDOWS;
        else if (std::strcmp(styleStr, "GUESS") == 0) style = Poco::Path::PATH_WINDOWS;
        else style = Poco::Path::PATH_NATIVE;
    }
    // new(path, style, absolute)
    if (top > firstArg + 1)
        absolute = lua_toboolean(L, firstArg + 2) != 0;

    try
    {
        PathUserdata* pud = new(lua_newuserdata(L, sizeof *pud)) PathUserdata(pathStr, style, absolute);
        setupPocoUserdata(L, pud, POCO_PATH_METATABLE_NAME);
        rv = 1;
    }
    catch (const std::exception& e)
    {
        rv = pushException(L, e);
    }
        return rv;
}

// metamethod infrastructure
int PathUserdata::metamethod__tostring(lua_State* L)
{
    PathUserdata* pud = checkPrivateUserdata<PathUserdata>(L, 1);

    lua_pushfstring(L, "Poco.Path (%p)", static_cast<void*>(pud));
    return 1;
}

// free standing functions

/// Gets the current working directory.
// @return string containing the current working directory.
// @function current
int PathUserdata::current(lua_State* L)
{
    std::string currentdir = Poco::Path::current();
    lua_pushlstring(L, currentdir.c_str(), currentdir.size());

    return 1;
}

/// Expands environment variables inline.
// @string path A path specifying a file or directory containing an environment variable to expand inline.
// On unix platforms, the tilde will be expanded to the users home directory.
// @return string containing the expanded path.
// @function expand
int PathUserdata::expand(lua_State* L)
{
    const char* path = luaL_checkstring(L, 1);
    std::string expanded = Poco::Path::expand(path);
    lua_pushlstring(L, expanded.c_str(), expanded.size());

    return 1;
}

/// Searches a list of paths to find a file or directory.
// @param pathlist an array containing the list of paths to search.
// @string name the name of the file or directory to find in the list of paths.
// @return path userdata or nil (not found)
// @function find
int PathUserdata::find(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TTABLE);
    const char* pathStr = luaL_checkstring(L, 2);
    Poco::Path::StringVec paths;

    for (size_t i = 1; ; ++i)
    {
        lua_rawgeti(L, 1, i);
        if (lua_isstring(L, -1))
        {
            paths.push_back(lua_tostring(L, -1));
            lua_pop(L, 1);
        }
        else if (lua_isnil(L, -1))
        {
            lua_pop(L, 1);
            break;
        }
    }

    Poco::Path path;
    if (Poco::Path::find(paths.begin(), paths.end(), pathStr, path))
    {
        PathUserdata* newpud = new(lua_newuserdata(L, sizeof *newpud)) PathUserdata(path);
        setupPocoUserdata(L, newpud, POCO_PATH_METATABLE_NAME);
    }
    else
    {
        lua_pushnil(L);
    }

    return 1;
}

/// Gets the home directory for the current user.
// @return string containing the home directory for the user.
// @function home
int PathUserdata::home(lua_State* L)
{
    std::string homedir = Poco::Path::home();
    lua_pushlstring(L, homedir.c_str(), homedir.size());

    return 1;
}

/// Gets list of filesystem roots.
// Fills the vector with all filesystem roots available on the system.
// On Unix, there is exactly one root, "/".
// On Windows, the roots are the drive letters.
// On OpenVMS, the roots are the mounted disks.
// @return array containing the list of filesystem roots.
// @function listRoots
int PathUserdata::listRoots(lua_State* L)
{
    std::vector<std::string> roots;
    Poco::Path::listRoots(roots);

    lua_createtable(L, 0, roots.size());
    for (size_t i = 0; i < roots.size(); ++i)
    {
        lua_pushlstring(L, roots[i].c_str(), roots[i].size());
        lua_rawseti(L, -2, i + 1);
    }

    return 1;
}

/// Gets the null device for the platform.
// @return string containing the null device for the platform.
// @function nullDevice
int PathUserdata::nullDevice(lua_State* L)
{
    std::string nulldev = Poco::Path::null();
    lua_pushlstring(L, nulldev.c_str(), nulldev.size());

    return 1;
}

/// Gets the full path separator character.
// @return string containing the character used to separate full paths for the platform.
// @function pathSeparator
int PathUserdata::pathSeparator(lua_State* L)
{
    const char pathsep = Poco::Path::pathSeparator();
    lua_pushlstring(L, &pathsep, 1);

    return 1;
}

/// Gets the directory separator character.
// @return string containing the character used to separate directories for the platform.
// @function separator
int PathUserdata::separator(lua_State* L)
{
    const char separator = Poco::Path::separator();
    lua_pushlstring(L, &separator, 1);

    return 1;
}

/// Gets the temporary directory path.
// @return string containing the temporary directory.
// @function temp
int PathUserdata::temp(lua_State* L)
{
    std::string tempdir = Poco::Path::temp();
    lua_pushlstring(L, tempdir.c_str(), tempdir.size());

    return 1;
}

/// Converts a UTF-8 encoded path to the current Windows code page.
// On Windows, if POCO has been compiled with Windows UTF-8 support (POCO_WIN32_UTF8),
// this function converts a string (usually containing a path) encoded in UTF-8
// into a string encoded in the current Windows code page.
// @return string containing the translated path.
// @function transcode
int PathUserdata::transcode(lua_State* L)
{
    const char* path = luaL_checkstring(L, 1);
    std::string transcoded = Poco::Path::transcode(path);
    lua_pushlstring(L, transcoded.c_str(), transcoded.size());

    return 1;
}

// userdata methods

///
// @type path

/// Gets a new absolute path userdata.
// @return path userdata containing the absolute path.
// @function absolute
int PathUserdata::absolute(lua_State* L)
{
    PathUserdata* pud = checkPrivateUserdata<PathUserdata>(L, 1);
    PathUserdata* newpud = new(lua_newuserdata(L, sizeof *newpud)) PathUserdata(pud->mPath.absolute());
    setupPocoUserdata(L, newpud, POCO_PATH_METATABLE_NAME);

    return 1;
}

/// Appends a path to another.
// If the supplied path userdata is absolute, it will replace the path, if it is a relative path, it will be appended.
// @param path path userdata to be appended.
// @function append
int PathUserdata::append(lua_State* L)
{
    PathUserdata* pud = checkPrivateUserdata<PathUserdata>(L, 1);
    PathUserdata* toappend = checkPrivateUserdata<PathUserdata>(L, 2);

    pud->mPath.append(toappend->mPath);

    return 0;
}

/// Clears the path stored in the userdata.
// Clear effectively creates an empty path userdata.
// @function clear
int PathUserdata::clear(lua_State* L)
{
    PathUserdata* pud = checkPrivateUserdata<PathUserdata>(L, 1);
    pud->mPath.clear();

    return 0;
}

/// Gets the number of directories in the directory list.
// @return depth as a number.
// @function depth
int PathUserdata::depth(lua_State* L)
{
    PathUserdata* pud = checkPrivateUserdata<PathUserdata>(L, 1);
    lua_pushinteger(L, pud->mPath.depth());

    return 1;
}

/// Gets the n'th directory in the directory list.
// If the index exceeds the number of directories, the filename will be returned.
// @param n index starting at 1, specifiying the item to retrieve.
// @return string containing directory or filename.
// @function directory
int PathUserdata::directory(lua_State* L)
{
    int rv = 0;
    PathUserdata* pud = checkPrivateUserdata<PathUserdata>(L, 1);
    int directoryIndex = luaL_checkinteger(L, 2);

    if (directoryIndex > 0)
    {
        const std::string& entry = pud->mPath.directory(directoryIndex - 1);
        lua_pushlstring(L, entry.c_str(), entry.size());
        rv = 1;
    }
    else
    {
        lua_pushnil(L);
        lua_pushfstring(L, "directory index %d is out of bounds.", directoryIndex);
        rv = 2;
    }

    return rv;
}

/// Gets the basename (the filename sans extension) of the path.
// @return string containing the basename.
// @function getBaseName
int PathUserdata::getBaseName(lua_State* L)
{
    PathUserdata* pud = checkPrivateUserdata<PathUserdata>(L, 1);
    std::string basename = pud->mPath.getBaseName();
    lua_pushlstring(L, basename.c_str(), basename.size());

    return 1;
}

/// Gets the device name.
// Only applies to Windows and VMS.
// @return string containing the devicename.
// @function getDevice
int PathUserdata::getDevice(lua_State* L)
{
    PathUserdata* pud = checkPrivateUserdata<PathUserdata>(L, 1);
    std::string device = pud->mPath.getDevice();
    lua_pushlstring(L, device.c_str(), device.size());

    return 1;
}

/// Gets the file extension.
// @return string containing the extension.
// @function getExtension
int PathUserdata::getExtension(lua_State* L)
{
    PathUserdata* pud = checkPrivateUserdata<PathUserdata>(L, 1);
    std::string extension = pud->mPath.getExtension();
    lua_pushlstring(L, extension.c_str(), extension.size());

    return 1;
}

/// Gets the filename.
// @return string containing the filename.
// @function getFileName
int PathUserdata::getFileName(lua_State* L)
{
    PathUserdata* pud = checkPrivateUserdata<PathUserdata>(L, 1);
    std::string filename = pud->mPath.getFileName();
    lua_pushlstring(L, filename.c_str(), filename.size());

    return 1;
}

/// Gets the node name.
// @return string containing the node name.
// @function getNode
int PathUserdata::getNode(lua_State* L)
{
    PathUserdata* pud = checkPrivateUserdata<PathUserdata>(L, 1);
    std::string node = pud->mPath.getNode();
    lua_pushlstring(L, node.c_str(), node.size());

    return 1;
}

/// Checks for an absolute path.
// @return boolean indicating if the path is absolute.
// @function isAbsolute
int PathUserdata::isAbsolute(lua_State* L)
{
    PathUserdata* pud = checkPrivateUserdata<PathUserdata>(L, 1);
    lua_pushboolean(L, pud->mPath.isAbsolute());

    return 1;
}

/// Checks if the specified path is a directory.
// Note that this function does not use the filesystem to make this determination.
// It is based on the text format supplied when created.
// @return boolean indicating if the path is a directory.
// @function isDirectory
int PathUserdata::isDirectory(lua_State* L)
{
    PathUserdata* pud = checkPrivateUserdata<PathUserdata>(L, 1);
    lua_pushboolean(L, pud->mPath.isDirectory());

    return 1;
}

/// Checks if the specified path is a file.
// Note that this function does not use the filesystem to make this determination.
// It is based on the text format supplied when created.
// @return boolean indicating if the path is a file.
// @function isFile
int PathUserdata::isFile(lua_State* L)
{
    PathUserdata* pud = checkPrivateUserdata<PathUserdata>(L, 1);
    lua_pushboolean(L, pud->mPath.isFile());

    return 1;
}

/// Checks for a relative path.
// @return boolean indicating if the path is relative.
// @function isRelative
int PathUserdata::isRelative(lua_State* L)
{
    PathUserdata* pud = checkPrivateUserdata<PathUserdata>(L, 1);
    lua_pushboolean(L, pud->mPath.isRelative());

    return 1;
}

/// Converts the path from relative to absolute.
// @function makeAbsolute
int PathUserdata::makeAbsolute(lua_State* L)
{
    PathUserdata* pud = checkPrivateUserdata<PathUserdata>(L, 1);
    pud->mPath.makeAbsolute();

    return 0;
}

/// Converts the path from a directory to a file.
// @function makeFile
int PathUserdata::makeFile(lua_State* L)
{
    PathUserdata* pud = checkPrivateUserdata<PathUserdata>(L, 1);
    pud->mPath.makeFile();

    return 0;
}

/// Strips off the last directory or file resulting in a path representing the parent.
// @function makeParent
int PathUserdata::makeParent(lua_State* L)
{
    PathUserdata* pud = checkPrivateUserdata<PathUserdata>(L, 1);
    pud->mPath.makeParent();

    return 0;
}

/// Gets a new path userdata representing the parent directory.
// @return path userdata.
// @function parent
int PathUserdata::parent(lua_State* L)
{
    PathUserdata* pud = checkPrivateUserdata<PathUserdata>(L, 1);
    PathUserdata* newpud = new(lua_newuserdata(L, sizeof *newpud)) PathUserdata(pud->mPath.parent());
    setupPocoUserdata(L, newpud, POCO_PATH_METATABLE_NAME);
    return 1;
}

/// Removes the last directory from the path.
// @function popDirectory
int PathUserdata::popDirectory(lua_State* L)
{
    PathUserdata* pud = checkPrivateUserdata<PathUserdata>(L, 1);

    pud->mPath.popDirectory();

    return 0;
}

/// Removes the first directory from the path.
// @function popFrontDirectory
int PathUserdata::popFrontDirectory(lua_State* L)
{
    PathUserdata* pud = checkPrivateUserdata<PathUserdata>(L, 1);

    pud->mPath.popFrontDirectory();

    return 0;
}

/// Adds a directory to the end of the path.
// @function popDirectory
int PathUserdata::pushDirectory(lua_State* L)
{
    PathUserdata* pud = checkPrivateUserdata<PathUserdata>(L, 1);
    const char* newdir = luaL_checkstring(L, 2);
    pud->mPath.pushDirectory(newdir);

    return 0;
}

/// Sets the basename of the path.
// @string basename the basename of the path.
// @function setBaseName
int PathUserdata::setBaseName(lua_State* L)
{
    PathUserdata* pud = checkPrivateUserdata<PathUserdata>(L, 1);
    const char* basename = luaL_checkstring(L, 2);
    pud->mPath.setBaseName(basename);

    return 0;
}

/// Sets the device portion of the path.
// @string device the device portion of the path.
// @function setDevice
int PathUserdata::setDevice(lua_State* L)
{
    PathUserdata* pud = checkPrivateUserdata<PathUserdata>(L, 1);
    const char* device = luaL_checkstring(L, 2);
    pud->mPath.setDevice(device);

    return 0;
}

/// Sets the file extention portion of the path.
// @string extension the file extension portion of the path.
// @function setExtension
int PathUserdata::setExtension(lua_State* L)
{
    PathUserdata* pud = checkPrivateUserdata<PathUserdata>(L, 1);
    const char* extension = luaL_checkstring(L, 2);
    pud->mPath.setExtension(extension);

    return 0;
}

/// Sets the filename portion of the path.
// @string filename the filename portion of the path.
// @function setFileName
int PathUserdata::setFileName(lua_State* L)
{
    PathUserdata* pud = checkPrivateUserdata<PathUserdata>(L, 1);
    const char* filename = luaL_checkstring(L, 2);
    pud->mPath.setFileName(filename);

    return 0;
}

/// Sets the node portion of the path.
// @string node the node portion of the path.
// @function setNode
int PathUserdata::setNode(lua_State* L)
{
    PathUserdata* pud = checkPrivateUserdata<PathUserdata>(L, 1);
    const char* node = luaL_checkstring(L, 2);
    pud->mPath.setNode(node);

    return 0;
}

/// Convert the path userdata to string format.
// @return string containing the path.
// @function setNode
int PathUserdata::toString(lua_State* L)
{
    PathUserdata* pud = checkPrivateUserdata<PathUserdata>(L, 1);
    lua_pushlstring(L, pud->mPath.toString().c_str(), pud->mPath.toString().size());
    return 1;
}

} // LuaPoco
