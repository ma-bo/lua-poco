#ifndef LUA_POCO_COMPRESS_H
#define LUA_POCO_COMPRESS_H

#include "LuaPoco.h"
#include "Userdata.h"
#include <Poco/Zip/Compress.h>

extern "C"
{
LUAPOCO_API int luaopen_poco_zip_compress(lua_State* L);
}

namespace LuaPoco
{
namespace Zip
{

class CompressUserdata : public Userdata
{
public:
    CompressUserdata(std::ostream& ostream, int ref, bool seekable, bool forceZip64);
    virtual ~CompressUserdata();
    // register metatables
    static bool registerCompress(lua_State* L);
    // constructor lua_CFunction
    static int Compress(lua_State* L);
private:
    int mUdReference;
    Poco::Zip::Compress mCompress;
    bool mClosed;

    static int metamethod__tostring(lua_State* L);
    static int metamethod__gc(lua_State* L);

    static int addDirectory(lua_State* L);
    static int addFile(lua_State* L);
    static int addIStream(lua_State* L);
    static int addRecursive(lua_State* L);
    static int close(lua_State* L);
    static int getStoreExtensions(lua_State* L);
    static int getZipComment(lua_State* L);
    static int setStoreExtensions(lua_State* L);
    static int setZipComment(lua_State* L);
};

}
} // LuaPoco

#endif
