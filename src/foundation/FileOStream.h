#ifndef LUA_POCO_FILEOSTREAM_H
#define LUA_POCO_FILEOSTREAM_H

#include "LuaPoco.h"
#include "Userdata.h"
#include "OStream.h"
#include <Poco/FileStream.h>

extern "C"
{
LUAPOCO_API int luaopen_poco_fileostream(lua_State* L);
}

namespace LuaPoco
{

extern const char* POCO_FILEOSTREAM_METATABLE_NAME;

class FileOStreamUserdata : public Userdata, public OStream
{
public:
    FileOStreamUserdata(const std::string& path);
    virtual ~FileOStreamUserdata();
    virtual std::ostream& ostream();
    // register metatable for this class
    static bool registerFileOStream(lua_State* L);
    // constructor function 
    static int FileOStream(lua_State* L);
    
    Poco::FileOutputStream mFileOutputStream;
private:
    // metamethod infrastructure
    static int metamethod__tostring(lua_State* L);
    // methods
    static int close(lua_State* L);
};

} // LuaPoco

#endif
