#ifndef LUA_POCO_DECOMPRESS_H
#define LUA_POCO_DECOMPRESS_H

#include "LuaPoco.h"
#include "Userdata.h"
#include <Poco/Path.h>
#include <Poco/Zip/ZipLocalFileHeader.h>
#include <Poco/Zip/Decompress.h>
#include <vector>
#include <utility>
#include <string>

extern "C"
{
LUAPOCO_API int luaopen_poco_zip_decompress(lua_State* L);
}

namespace LuaPoco
{
namespace Zip
{

class DecompressUserdata : public Userdata
{
public:
    DecompressUserdata(std::istream& istream, int ref, const Poco::Path& dirPath, bool flattenDirs, bool keepIncompleteFiles);
    virtual ~DecompressUserdata();
    // register metatables
    static bool registerDecompress(lua_State* L);
    // constructor lua_CFunction
    static int Decompress(lua_State* L);
private:
    int mUdReference;
    Poco::Zip::Decompress mDecompress;

    static int metamethod__tostring(lua_State* L);
    static int metamethod__gc(lua_State* L);

    static int decompressAll(lua_State* L);
    static int decompressed(lua_State* L);
    static int failed(lua_State* L);
    static int good_iter(lua_State* L);
    static int fail_iter(lua_State* L);

    bool zipLocalFileHeaderToTable(lua_State* L, const Poco::Zip::ZipLocalFileHeader& zflh);
    void onError(const void* dp, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string>& entry);
    void onDecompress(const void* dp, std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path>& entry);

    std::vector<std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path>> mGood;
    std::vector<std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string>> mFail;
    size_t mGoodIdx;
    size_t mFailIdx;
};

}
} // LuaPoco

#endif
