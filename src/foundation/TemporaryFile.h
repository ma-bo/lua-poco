#ifndef LUA_POCO_TEMPORARYFILE_H
#define LUA_POCO_TEMPORARYFILE_H

#include "LuaPoco.h"
#include "Userdata.h"
#include "File.h"
#include <Poco/TemporaryFile.h>

extern "C"
{

LUAPOCO_API int luaopen_poco_temporaryfile(lua_State* L);

}

namespace LuaPoco
{
class TemporaryFileUserdata : public FileUserdata
{
public:
    TemporaryFileUserdata(const char *path);
    TemporaryFileUserdata(const Poco::TemporaryFile& file);
    virtual ~TemporaryFileUserdata();
    virtual bool copyToState(lua_State* L);
    virtual Poco::File& getFile();
    // constructor
    static int TemporaryFile(lua_State* L);
    static bool registerTemporaryFile(lua_State* L);
    static int registerForDeletion(lua_State* L);
    static int tempName(lua_State* L);
private:
    // metamethod infrastructure
    static int metamethod__tostring(lua_State* L);
    // userdata methods
    static int keep(lua_State* L);
    static int keepUntilExit(lua_State* L);

    Poco::TemporaryFile mTemporaryFile;
};

} // LuaPoco

#endif
