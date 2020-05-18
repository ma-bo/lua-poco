#ifndef LUA_POCO_PATH_H
#define LUA_POCO_PATH_H

#include "LuaPoco.h"
#include "Userdata.h"
#include <Poco/Path.h>

extern "C"
{
LUAPOCO_API int luaopen_poco_path(lua_State* L);
}

namespace LuaPoco
{

extern const char* POCO_PATH_METATABLE_NAME;

class PathUserdata : public Userdata
{
public:
    PathUserdata();
    PathUserdata(const char* path, Poco::Path::Style style, bool absolute);
    PathUserdata(Poco::Path path);
    virtual ~PathUserdata();
    virtual bool copyToState(lua_State *L);
    // register metatable for this class
    static bool registerPath(lua_State* L);
    // constructor function 
    static int Path(lua_State* L);
    
    // external static functions
    static int current(lua_State* L);
    static int expand(lua_State* L);
    static int find(lua_State* L);
    static int home(lua_State* L);
    static int listRoots(lua_State* L);
    static int nullDevice(lua_State* L);
    static int pathSeparator(lua_State* L);
    static int separator(lua_State* L);
    static int temp(lua_State* L);
    static int transcode(lua_State* L);
    
    // public for append
    Poco::Path mPath;
private:
    // metamethod infrastructure
    static int metamethod__tostring(lua_State* L);
    
    // userdata methods
    static int absolute(lua_State* L);
    static int append(lua_State* L);
    static int clear(lua_State* L);
    static int depth(lua_State* L);
    static int directory(lua_State* L);
    static int getBaseName(lua_State* L);
    static int getDevice(lua_State* L);
    static int getExtension(lua_State* L);
    static int getFileName(lua_State* L);
    static int getNode(lua_State* L);
    static int isAbsolute(lua_State* L);
    static int isDirectory(lua_State* L);
    static int isFile(lua_State* L);
    static int isRelative(lua_State* L);
    static int makeAbsolute(lua_State* L);
    static int makeFile(lua_State* L);
    static int makeParent(lua_State* L);
    static int parent(lua_State* L);
    static int popDirectory(lua_State* L);
    static int popFrontDirectory(lua_State* L);
    static int pushDirectory(lua_State* L);
    static int setBaseName(lua_State* L);
    static int setDevice(lua_State* L);
    static int setExtension(lua_State* L);
    static int setFileName(lua_State* L);
    static int setNode(lua_State* L);
    static int toString(lua_State* L);
};

} // LuaPoco

#endif
