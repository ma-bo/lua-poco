/// Zip file compression
// Add files, directories, and recursive directories to a zip file.
// @module compress

#include <cstring>
#include "Compress.h"
#include <OStream.h>
#include <FileOStream.h>
#include <IStream.h>
#include <Path.h>
#include <Timestamp.h>
#include <MemoryOStream.h>
#include <Poco/Zip/Compress.h>
#include <Poco/Zip/Decompress.h>
#include <Poco/Zip/ZipArchive.h>
#include <Poco/Zip/ZipCommon.h>
#include <Poco/DateTime.h>

int luaopen_poco_zip_compress(lua_State* L)
{
    LuaPoco::Zip::CompressUserdata::registerCompress(L);
    return LuaPoco::loadConstructor(L, LuaPoco::Zip::CompressUserdata::Compress);
}

namespace LuaPoco
{
namespace Zip
{

const char* POCO_COMPRESS_METATABLE_NAME = "Poco.Zip.Compress.metatable";

/// Creates a new compress userdata instance.
// @param ostream userdata for writing zip output
// @bool[opt] forceZip64 (default false) require the file header be allocated with zip64 extension in case the
// compressed or uncompressed size exceeds 32 bits.
// @return table or nil. (error)
// @return error message.
// @function new
int CompressUserdata::Compress(lua_State* L)
{
    int rv = 0;
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    
    try
    {
        bool seekable = false;
        bool forceZip64 = false;
        OStream* os = checkPrivateUserdata<OStream>(L, firstArg);
        
        if (dynamic_cast<FileOStreamUserdata*>(os) ||
            dynamic_cast<MemoryOStreamUserdata*>(os)) { seekable = true; }
        
        if (lua_isboolean(L, firstArg + 1))
            { forceZip64 = static_cast<bool>(lua_toboolean(L, firstArg + 1)); }
        
        lua_pushvalue(L, firstArg);
        int ref = luaL_ref(L, LUA_REGISTRYINDEX);
        
        CompressUserdata* cud = new(lua_newuserdata(L, sizeof *cud))
            CompressUserdata(os->ostream(), ref, seekable, forceZip64);
        
        setupPocoUserdata(L, cud, POCO_COMPRESS_METATABLE_NAME);
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

// register metatable for this class
bool CompressUserdata::registerCompress(lua_State* L)
{
    struct CFunctions methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "addDirectory", addDirectory },
        { "addFile", addFile },
        { "addIStream", addIStream },
        { "addRecursive", addRecursive },
        { "close", close},
        { "getStoreExtensions", getStoreExtensions},
        { "getZipComment", getZipComment},
        { "setStoreExtensions", setStoreExtensions},
        { "setZipComment", setZipComment},
        { NULL, NULL}
    };
    
    setupUserdataMetatable(L, POCO_COMPRESS_METATABLE_NAME, methods);
    return true;
}

CompressUserdata::CompressUserdata(std::ostream& ostream
                                   , int ref
                                   , bool seekable
                                   , bool forceZip64)
    : mUdReference(ref)
    , mCompress(ostream, seekable, forceZip64)
    , mClosed(false)
{
}

CompressUserdata::~CompressUserdata()
{
}

int CompressUserdata::metamethod__gc(lua_State* L)
{
    CompressUserdata* cud = checkPrivateUserdata<CompressUserdata>(L, 1);
    if (!cud->mClosed) { cud->mCompress.close(); }
    luaL_unref(L, LUA_REGISTRYINDEX, cud->mUdReference);
    cud->~CompressUserdata();
    
    return 0;
}

// metamethod infrastructure
int CompressUserdata::metamethod__tostring(lua_State* L)
{
    CompressUserdata* cud = checkPrivateUserdata<CompressUserdata>(L, 1);
    lua_pushfstring(L, "Poco.CompressUserdata (%p)", static_cast<void*>(cud));
    
    return 1;
}

// utility functions to avoid code duplication for converting to CompressionMethod/CompressionLevel
Poco::Zip::ZipCommon::CompressionMethod strToCompressionMethod(const char* method)
{
    Poco::Zip::ZipCommon::CompressionMethod cm = Poco::Zip::ZipCommon::CompressionMethod::CM_DEFLATE;

    if (std::strcmp(method, "auto") == 0) { cm = Poco::Zip::ZipCommon::CompressionMethod::CM_AUTO; }
    else if (std::strcmp(method, "store") == 0) { cm = Poco::Zip::ZipCommon::CompressionMethod::CM_STORE; }
    else { cm = Poco::Zip::ZipCommon::CompressionMethod::CM_DEFLATE; }

    return cm;
}

Poco::Zip::ZipCommon::CompressionLevel strToCompressionLevel(const char* level)
{
    Poco::Zip::ZipCommon::CompressionLevel cl = Poco::Zip::ZipCommon::CompressionLevel::CL_MAXIMUM;
    
    if (std::strcmp(level, "normal") == 0) { cl = Poco::Zip::ZipCommon::CompressionLevel::CL_NORMAL; }
    else if (std::strcmp(level, "fast") == 0) { cl = Poco::Zip::ZipCommon::CompressionLevel::CL_FAST; }
    else if (std::strcmp(level, "superfast") == 0) { cl = Poco::Zip::ZipCommon::CompressionLevel::CL_SUPERFAST; }
    else { cl = Poco::Zip::ZipCommon::CompressionLevel::CL_MAXIMUM; }

    return cl;
}

///
// @type compress

/// Adds a directory entry name to the zip file.
// @param entry path userdata (or string) to be added to the zip file.
// this directory does not need to exist on the file system.
// @param[opt] timestamp provided by os.time() or a timestamp userdata.  uses current time if no
// timestamp is provided.
// @return true or nil. (error)
// @return error message.
// @function addDirectory
// @see path
// @see timestamp
int CompressUserdata::addDirectory(lua_State* L)
{
    int rv = 0;
    CompressUserdata* cud = checkPrivateUserdata<CompressUserdata>(L, 1);

    const char* dirPathStr = NULL;
    time_t dateTimeT = 0;
    PathUserdata* pud = NULL;
    TimestampUserdata* tsud = NULL;
    
    if (lua_isstring(L, 2)) { dirPathStr = lua_tostring(L, 2); }
    else { pud = checkPrivateUserdata<PathUserdata>(L, 2); }

    // optional, if none supplied, use current DateTime.
    if (lua_isnumber(L, 3)) { dateTimeT = static_cast<time_t>(lua_tointeger(L, 3)); }
    else if (lua_isuserdata(L, 3)) { tsud = checkPrivateUserdata<TimestampUserdata>(L, 3); }

    try
    {
        Poco::Path dirPath(pud ? pud->mPath : dirPathStr);
        Poco::DateTime entryDateTime;
        if (lua_gettop(L) > 2) { entryDateTime = tsud ? tsud->mTimestamp : Poco::Timestamp(dateTimeT); }
        
        cud->mCompress.addDirectory(dirPath, entryDateTime);
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

/// Adds a file to the zip file.
// @param file path userdata (or string) to the file to be added.
// @param entry path userdata (or string) for the name of the entry in the zip file.
// @param[opt] timestamp provided by os.time() or a timestamp userdata.  uses current time if no
// timestamp is provided.
// @string[opt] method (default: deflate) "auto", "store", or "deflate".
// auto uses the list of file extensions to automatically decide if a file should be "store" or "deflate".
// @string[opt] level (default: maximum) "normal", "fast", "superfast", "maximum".
// levels correspond to a zlib configuration: normal=Z_DEFAULT_COMPRESSION, fast/superfast=Z_BEST_SPEED, maximum=Z_BEST_COMPRESSION.
// @return true or nil. (error)
// @return error message.
// @function addFile
// @see path
// @see timestamp
int CompressUserdata::addFile(lua_State* L)
{
    int rv = 0;
    CompressUserdata* cud = checkPrivateUserdata<CompressUserdata>(L, 1);

    const char* dirPathStr = NULL;
    PathUserdata* dirPathUd = NULL;
    const char* filePathStr = NULL;
    PathUserdata* filePathUd = NULL;
    time_t dateTimeT = 0;
    TimestampUserdata *tsud = NULL;
   
    if (lua_isstring(L, 2)) { dirPathStr = lua_tostring(L, 2); }
    else { dirPathUd = checkPrivateUserdata<PathUserdata>(L, 2); }

    if (lua_isstring(L, 3)) { filePathStr = lua_tostring(L, 3); }
    else { filePathUd = checkPrivateUserdata<PathUserdata>(L, 3); }
   
    try
    {
        Poco::Path dirPath(dirPathUd ? dirPathUd->mPath : dirPathStr);
        Poco::Path filePath(filePathUd ? filePathUd->mPath : filePathStr);
        Poco::Zip::ZipCommon::CompressionMethod cm = Poco::Zip::ZipCommon::CompressionMethod::CM_DEFLATE;
        Poco::Zip::ZipCommon::CompressionLevel cl = Poco::Zip::ZipCommon::CompressionLevel::CL_MAXIMUM;
        
        if (lua_isstring(L, 4)) { cm = strToCompressionMethod(lua_tostring(L, 4)); }
        if (lua_isstring(L, 5)) { cl = strToCompressionLevel(lua_tostring(L, 5)); }
        
        cud->mCompress.addFile(dirPath, filePath, cm, cl);
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

/// Adds istream data to the zip file.
// @param istream any istream userdata to be used as the source data to be added.
// @param entry path userdata (or string) for the name of the entry in the zip file.
// @param[opt] timestamp provided by os.time() or a timestamp userdata.  uses current time if no
// timestamp is provided.
// @string[opt] method (default: deflate) "auto", "store", or "deflate".
// auto uses the list of file extensions to automatically decide if a file should be "store" or "deflate".
// @string[opt] level (default: maximum) "normal", "fast", "superfast", "maximum".
// levels correspond to a zlib configuration: normal=Z_DEFAULT_COMPRESSION, fast/superfast=Z_BEST_SPEED, maximum=Z_BEST_COMPRESSION.
// @return true or nil. (error)
// @return error message.
// @function addIStream
// @see istream
// @see path
// @see timestamp
int CompressUserdata::addIStream(lua_State* L)
{
    int rv = 0;
    CompressUserdata* cud = checkPrivateUserdata<CompressUserdata>(L, 1);
    IStream* is = checkPrivateUserdata<IStream>(L, 2);

    time_t entryDateTimeT = 0;
    const char* pathStr = NULL;
    TimestampUserdata* tsud = NULL;
    PathUserdata* pud = NULL;
         
    if (lua_isstring(L, 3)) { pathStr = lua_tostring(L, 3); }
    else { pud = checkPrivateUserdata<PathUserdata>(L, 3); }

    if (lua_isnumber(L, 4)) { entryDateTimeT = static_cast<time_t>(lua_tointeger(L, 4)); }
    else if (lua_isuserdata(L, 4)) { tsud = checkPrivateUserdata<TimestampUserdata>(L, 4); }

    try
    {
        Poco::Path dirPath(pud ? pud->mPath : pathStr);
        Poco::DateTime entryDateTime;
        Poco::Zip::ZipCommon::CompressionMethod cm = Poco::Zip::ZipCommon::CompressionMethod::CM_DEFLATE;
        Poco::Zip::ZipCommon::CompressionLevel cl = Poco::Zip::ZipCommon::CompressionLevel::CL_MAXIMUM;
        
        if (lua_gettop(L) > 3) { entryDateTime =  tsud ? tsud->mTimestamp : entryDateTimeT; }
        if (lua_isstring(L, 5)) { cm = strToCompressionMethod(lua_tostring(L, 5)); }
        if (lua_isstring(L, 6)) { cl = strToCompressionLevel(lua_tostring(L, 6)); }
      
        cud->mCompress.addFile(is->istream(), entryDateTime, dirPath, cm, cl);
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

/// Recursively adds all children file and directories to the zip file.
// @param directory path userdata (or string) to the file to be added.
// @param entry path userdata (or string) for the name of the directory entry in the zip file.
// if an empty string or path userdata is supplied, the files will be added directly to the zip without a parent directory.
// @string[opt] method (default: deflate) "auto", "store", or "deflate".
// auto uses the list of file extensions to automatically decide if a file should be "store" or "deflate".
// @string[opt] level (default: maximum) "normal", "fast", "superfast", "maximum".
// levels correspond to a zlib configuration: normal=Z_DEFAULT_COMPRESSION, fast/superfast=Z_BEST_SPEED, maximum=Z_BEST_COMPRESSION.
// @return true or nil. (error)
// @return error message.
// @function addIStream
// @see path
int CompressUserdata::addRecursive(lua_State* L)
{
    int rv = 0;
    CompressUserdata* cud = checkPrivateUserdata<CompressUserdata>(L, 1);

    const char* dirPathStr = NULL;
    PathUserdata* dirPathUd = NULL;
    const char* entryPathStr = NULL;
    PathUserdata* entryPathUd = NULL;
    
    if (lua_isstring(L, 2)) { dirPathStr = lua_tostring(L, 2); }
    else { dirPathUd = checkPrivateUserdata<PathUserdata>(L, 2); }

    if (lua_isstring(L, 3)) { entryPathStr = lua_tostring(L, 3); }
    else { entryPathUd = checkPrivateUserdata<PathUserdata>(L, 3); }
        
    try
    {
        Poco::Zip::ZipCommon::CompressionMethod cm = Poco::Zip::ZipCommon::CompressionMethod::CM_DEFLATE;
        Poco::Zip::ZipCommon::CompressionLevel cl = Poco::Zip::ZipCommon::CompressionLevel::CL_MAXIMUM;
        bool excludeRoot = true;
        
        Poco::Path dirPath(dirPathUd ? dirPathUd->mPath : dirPathStr);
        Poco::Path entryPath(entryPathUd ? entryPathUd->mPath : entryPathStr);

        if (lua_isstring(L, 4)) { cm = strToCompressionMethod(lua_tostring(L, 4)); }
        if (lua_isstring(L, 5)) { cl = strToCompressionLevel(lua_tostring(L, 5)); }
        
        cud->mCompress.addRecursive(dirPath, cm, cl, excludeRoot, entryPath);
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

/// Finalizes and writes the zip file to the supplied ostream.
// If not called explicitly, close will be called when the compress userdata is garbage collected.
// @function close
// @see ostream
int CompressUserdata::close(lua_State* L)
{
    CompressUserdata* cud = checkPrivateUserdata<CompressUserdata>(L, 1);
    cud->mCompress.close();
    cud->mClosed = true;
    return 0;
}

int CompressUserdata::getStoreExtensions(lua_State* L)
{
    CompressUserdata* cud = checkPrivateUserdata<CompressUserdata>(L, 1);
    const std::set<std::string>& extensions = cud->mCompress.getStoreExtensions();

    std::set<std::string>::const_iterator iter = extensions.begin();
    std::set<std::string>::const_iterator end = extensions.end();

    lua_createtable(L, static_cast<int>(extensions.size()), 0);
    int idx = 1;

    while (iter != end)
    {
        lua_pushlstring(L, iter->c_str(), iter->size());
        lua_rawseti(L, -2, idx);
        ++idx;
        ++iter;
    }
    
    return 1;
}

int CompressUserdata::getZipComment(lua_State* L)
{
    CompressUserdata* cud = checkPrivateUserdata<CompressUserdata>(L, 1);
    const std::string comment = cud->mCompress.getZipComment();
    lua_pushlstring(L, comment.c_str(), comment.size());
    return 1;
}

int CompressUserdata::setStoreExtensions(lua_State* L)
{
    CompressUserdata* cud = checkPrivateUserdata<CompressUserdata>(L, 1);
    luaL_checktype(L, 2, LUA_TTABLE);
    
    std::set<std::string> extensions;
    int idx = 1;
    for (lua_rawgeti(L, 2, idx); lua_isstring(L, -1); lua_rawgeti(L, 2, ++idx))
    {
        extensions.insert(lua_tostring(L, -1));
    }

    cud->mCompress.setStoreExtensions(extensions);

    return 0;
}

int CompressUserdata::setZipComment(lua_State* L)
{
    CompressUserdata* cud = checkPrivateUserdata<CompressUserdata>(L, 1);
    cud->mCompress.setZipComment(luaL_checkstring(L, 2));
    return 0;
}

} // Zip
} // LuaPoco
