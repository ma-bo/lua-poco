#ifndef LUA_POCO_FILEISTREAM_H
#define LUA_POCO_FILEISTREAM_H

#include "LuaPoco.h"
#include "Userdata.h"
#include "IStream.h"
#include <Poco/FileStream.h>

extern "C"
{
LUAPOCO_API int luaopen_poco_fileistream(lua_State* L);
}

namespace LuaPoco
{

extern const char* POCO_FILEISTREAM_METATABLE_NAME;

class FileIStreamUserdata : public Userdata, public IStream
{
public:
    FileIStreamUserdata(const std::string& path);
    virtual ~FileIStreamUserdata();
    virtual std::istream& istream();
    // register metatable for this class
    static bool registerFileIStream(lua_State* L);
    // constructor function 
    static int FileIStream(lua_State* L);
    
    Poco::FileInputStream mFileInputStream;
private:
    // metamethod infrastructure
    static int metamethod__tostring(lua_State* L);
    // methods
    static int close(lua_State* L);
};

} // LuaPoco

#endif
