#ifndef LUA_POCO_FILE_H
#define LUA_POCO_FILE_H

#include "LuaPoco.h"
#include "Userdata.h"
#include <Poco/File.h>

extern "C"
{

LUAPOCO_API int luaopen_poco_file(lua_State* L);

}

namespace LuaPoco
{

extern const char* POCO_FILE_METATABLE_NAME;

class FileUserdata : public Userdata
{
public:
    FileUserdata(const char *path);
    FileUserdata(const Poco::File& file);
    virtual ~FileUserdata();
    virtual bool copyToState(lua_State* L);
    Poco::File* getFileInternal();
    
    // register metatable for this class
    static bool registerFile(lua_State* L);
    // constructor
    static int File(lua_State* L);
private:
    // metamethod infrastructure
    static int metamethod__tostring(lua_State* L);
    
    // userdata methods
    static int copyTo(lua_State* L);
    static int createDirectories(lua_State* L);
    static int createDirectory(lua_State* L);
    static int createFile(lua_State* L);
    static int listNames(lua_State* L);
    static int listFiles(lua_State* L);
    static int moveTo(lua_State* L);
    static int remove(lua_State* L);
    static int renameTo(lua_State* L);
    static int canExecute(lua_State* L);
    static int canRead(lua_State* L);
    static int canWrite(lua_State* L);
    static int created(lua_State* L);
    static int exists(lua_State* L);
    static int getLastModified(lua_State* L);
    static int getSize(lua_State* L);
    static int isDevice(lua_State* L);
    static int isDirectory(lua_State* L);
    static int isFile(lua_State* L);
    static int isHidden(lua_State* L);
    static int isLink(lua_State* L);
    static int path(lua_State* L);
    static int setExecutable(lua_State* L);
    static int setLastModified(lua_State* L);
    static int setReadOnly(lua_State* L);
    static int setSize(lua_State* L);
    static int setWritable(lua_State* L);
    
    Poco::File mFile;
};

} // LuaPoco

#endif
