/// Zip file decompression
// Extract zip files to the specified destination.
// @module decompress

#include <ios>
#include <Poco/DateTime.h>
#include <Poco/Delegate.h>
#include <Poco/Exception.h>
#include "Decompress.h"
#include "Path.h"
#include "IStream.h"
#include "Timestamp.h"
#include "LuaPocoUtils.h"

///
// Contains information from C++ class ZipLocalFileHeader.
// https://docs.pocoproject.org/current/Poco.Zip.ZipLocalFileHeader.html
// @field crc crc
// @field compressedSize compressedSize
// @field dataStartPos The compressed data start position.
// @field dataEndPos Points past the last byte of the data.
// @field startPos Pposition of the first byte of the header in the file stream.
// @field endPos Points past the last byte of the file entry header.
// (ie. either the first byte of the next header, or the directory)
// @field headerSize Total size of the header including filename + extra field size.
// @field uncompressedSize
// @field level normal, maximum, fast, superfast --
// Compression level used. Only valid when the compression method is "deflate"
// @field method store, shrunk, factor1, factor2, factor3, factor4, implode, tokenize, deflate,
// enhanceddeflate, datecomprimploding, unused, auto
// @field extraField extraField
// @field fileName fileName
// @field hostSystem fat, amiga, vms, unix, vm_cms, atari, hpfs, macintosh, zsystem, cp_m, tops20,
// ntfs, sms_qdos, acorn, vfat, mvs, beos, tandem, unused
// @field majorVersionNumber majorVersionNumber
// @field minorVersionNumber minorVersionNumber
// @field hasData hasData
// @field hasExtraField hasExtraField
// @field hasSupportedCompressionMethod hasSupportedCompressionMethod
// @field isDirectory isDirectory
// @field isEncrypted isEncrypted
// @field isFile isFile
// @field needsZip64 needsZip64
// @field searchCRCAndSizesAfterData searchCRCAndSizesAfterData
// @field lastModifiedAt timestamp userdata representing the last modified time.
// @table ZipLocalFileHeader
// @see timestamp

int luaopen_poco_zip_decompress(lua_State* L)
{
    LuaPoco::PathUserdata::registerPath(L);
    LuaPoco::TimestampUserdata::registerTimestamp(L);
    LuaPoco::Zip::DecompressUserdata::registerDecompress(L);
    return LuaPoco::loadConstructor(L, LuaPoco::Zip::DecompressUserdata::Decompress);
}

namespace LuaPoco
{
namespace Zip
{

const char* POCO_DECOMPRESS_METATABLE_NAME = "Poco.Zip.Decompress.metatable";

/// Creates a new Decompress userdata instance.
// @tparam userdata istream userdata for reading zip archive input data.
// @tparam userdata outputPath path userdata or a string specifying where to extract zip archive.
// @bool[opt] flattenDirs (default: false) do not recreate directory structure,
// all files are extracted into one single directory.
// @bool[opt] keepIncompleteFiles preserve incompletely decompressed files (default: false)
// @return userdata or nil. (error)
// @return error message.
// @function new
int DecompressUserdata::Decompress(lua_State* L)
{
    int rv = 0;
    int firstArg = lua_istable(L, 1) ? 2 : 1;

    try
    {
        bool flattenDirs = false;
        bool keepIncompleteFiles = false;
        IStream* is = checkPrivateUserdata<IStream>(L, firstArg);
        const char* dirPathStr = NULL;
        PathUserdata* pud = NULL;

        if (lua_isstring(L, firstArg + 1)) { dirPathStr = lua_tostring(L, firstArg + 1); }
        else { pud = checkPrivateUserdata<PathUserdata>(L, firstArg + 1); }

        const Poco::Path dirPath(pud ? pud->mPath : dirPathStr);

        if (lua_isboolean(L, firstArg + 2))
            { flattenDirs = static_cast<bool>(lua_toboolean(L, firstArg + 2)); }

        if (lua_isboolean(L, firstArg + 3))
            { keepIncompleteFiles = static_cast<bool>(lua_toboolean(L, firstArg + 3)); }

        lua_pushvalue(L, firstArg);
        int ref = luaL_ref(L, LUA_REGISTRYINDEX);

        DecompressUserdata* dcud = new(lua_newuserdata(L, sizeof *dcud))
            DecompressUserdata(is->istream(), ref, dirPath, flattenDirs, keepIncompleteFiles);

        setupPocoUserdata(L, dcud, POCO_DECOMPRESS_METATABLE_NAME);
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
bool DecompressUserdata::registerDecompress(lua_State* L)
{
    struct CFunctions methods[] =
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "decompressAll", decompressAll },
        { "decompressed", decompressed },
        { "failed", failed },
        { NULL, NULL}
    };

    setupUserdataMetatable(L, POCO_DECOMPRESS_METATABLE_NAME, methods);
    return true;
}

DecompressUserdata::DecompressUserdata(std::istream& istream
                                   , int ref
                                   , const Poco::Path& dirPath
                                   , bool flattenDirs
                                   , bool keepIncompleteFiles)
    : mUdReference(ref)
    , mDecompress(istream, dirPath, flattenDirs, keepIncompleteFiles)
    , mGoodIdx(0)
    , mFailIdx(0)
{
}

DecompressUserdata::~DecompressUserdata()
{
}

int DecompressUserdata::metamethod__gc(lua_State* L)
{
    DecompressUserdata* dcud = checkPrivateUserdata<DecompressUserdata>(L, 1);
    luaL_unref(L, LUA_REGISTRYINDEX, dcud->mUdReference);
    dcud->~DecompressUserdata();

    return 0;
}

// metamethod infrastructure
int DecompressUserdata::metamethod__tostring(lua_State* L)
{
    DecompressUserdata* dcud = checkPrivateUserdata<DecompressUserdata>(L, 1);
    lua_pushfstring(L, "Poco.DecompressUserdata (%p)", static_cast<void*>(dcud));

    return 1;
}


///
// @type decompress

/// Decompresses all files from Zip archive.
// @return true or nil. (error)
// @return error message in case of error.
// @return count of entries decompressed.
// @return count of entries that failed to decompress.
// @function decompressAll
int DecompressUserdata::decompressAll(lua_State* L)
{
    int rv = 0;
    DecompressUserdata* dcud = checkPrivateUserdata<DecompressUserdata>(L, 1);

    try
    {
        dcud->mDecompress.EError += Poco::delegate(dcud, &DecompressUserdata::onError);
        dcud->mDecompress.EOk += Poco::delegate(dcud, &DecompressUserdata::onDecompress);

        Poco::Zip::ZipArchive za = dcud->mDecompress.decompressAllFiles();

        dcud->mDecompress.EError -= Poco::delegate(dcud, &DecompressUserdata::onError);
        dcud->mDecompress.EOk -= Poco::delegate(dcud, &DecompressUserdata::onDecompress);

        lua_pushboolean(L, 1);
        lua_pushstring(L, "success");
        lua_pushinteger(L, static_cast<lua_Integer>(dcud->mGood.size()));
        lua_pushinteger(L, static_cast<lua_Integer>(dcud->mFail.size()));
        rv = 4;
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

bool DecompressUserdata::zipLocalFileHeaderToTable(lua_State* L, const Poco::Zip::ZipLocalFileHeader& zlfh)
{
    lua_Integer crc = 0;
    lua_Integer compressedSize = 0;
    lua_Integer dataStartPos = 0;
    lua_Integer dataEndPos = 0;
    lua_Integer startPos = 0;
    lua_Integer endPos = 0;
    lua_Integer headerSize = 0;
    lua_Integer uncompressedSize = 0;

    if (checkUnsignedToLuaInteger<Poco::UInt32>(zlfh.getCRC(), crc)
        && checkUnsignedToLuaInteger<Poco::UInt64>(zlfh.getCompressedSize(), compressedSize)
        && checkSignedToLuaInteger<std::streamoff>(zlfh.getDataEndPos(), dataEndPos)
        && checkSignedToLuaInteger<std::streamoff>(zlfh.getDataStartPos(), dataStartPos)
        && checkSignedToLuaInteger<std::streamoff>(zlfh.getEndPos(), endPos)
        && checkUnsignedToLuaInteger<Poco::UInt32>(zlfh.getHeaderSize(), headerSize)
        && checkSignedToLuaInteger<std::streamoff>(zlfh.getStartPos(), startPos)
        && checkUnsignedToLuaInteger<Poco::UInt64>(zlfh.getUncompressedSize(), uncompressedSize))
    {
        lua_pushinteger(L, crc); lua_setfield(L, -2, "crc");
        lua_pushinteger(L, compressedSize); lua_setfield(L, -2, "compressedSize");
        lua_pushinteger(L, dataStartPos); lua_setfield(L, -2, "dataStartPos");
        lua_pushinteger(L, dataEndPos); lua_setfield(L, -2, "dataEndPos");
        lua_pushinteger(L, startPos); lua_setfield(L, -2, "startPos");
        lua_pushinteger(L, endPos); lua_setfield(L, -2, "endPos");
        lua_pushinteger(L, headerSize); lua_setfield(L, -2, "headerSize");
        lua_pushinteger(L, uncompressedSize); lua_setfield(L, -2, "uncompressedSize");
    }
    else throw Poco::RangeException("out of range number");

    const char* level = "unknown";
    switch (zlfh.getCompressionLevel())
    {
        case Poco::Zip::ZipCommon::CL_NORMAL: { level = "normal"; break; }
        case Poco::Zip::ZipCommon::CL_MAXIMUM: { level = "maximum"; break; }
        case Poco::Zip::ZipCommon::CL_FAST: { level = "fast"; break; };
        case Poco::Zip::ZipCommon::CL_SUPERFAST: { level = "superfast"; break; }
        default: level = "unknown";
    }

    lua_pushstring(L, level);
    lua_setfield(L, -2, "level");

    const char* method = "unknown";
    switch (zlfh.getCompressionMethod())
    {
        case Poco::Zip::ZipCommon::CM_STORE: { method = "store"; break; }
        case Poco::Zip::ZipCommon::CM_SHRUNK: { method = "shrunk"; break; }
        case Poco::Zip::ZipCommon::CM_FACTOR1: { method = "factor1"; break; };
        case Poco::Zip::ZipCommon::CM_FACTOR2: { method = "factor2"; break; };
        case Poco::Zip::ZipCommon::CM_FACTOR3: { method = "factor3"; break; };
        case Poco::Zip::ZipCommon::CM_FACTOR4: { method = "factor4"; break; };
        case Poco::Zip::ZipCommon::CM_IMPLODE: { method = "implode"; break; };
        case Poco::Zip::ZipCommon::CM_TOKENIZE: { method = "tokenize"; break; };
        case Poco::Zip::ZipCommon::CM_DEFLATE: { method = "deflate"; break; };
        case Poco::Zip::ZipCommon::CM_ENHANCEDDEFLATE: { method = "enhanceddeflate"; break; };
        case Poco::Zip::ZipCommon::CM_DATECOMPRIMPLODING: { method = "datecomprimploding"; break; };
        case Poco::Zip::ZipCommon::CM_UNUSED: { method = "unused"; break; };
        case Poco::Zip::ZipCommon::CM_AUTO: { method = "auto"; break; };
        default: method = "unknown";
    }

    lua_pushstring(L, method);
    lua_setfield(L, -2, "method");

    lua_pushstring(L, zlfh.getExtraField().c_str());
    lua_setfield(L, -2, "extraField");

    lua_pushstring(L, zlfh.getFileName().c_str());
    lua_setfield(L, -2, "fileName");

    const char* hostsystem = "unknown";
    switch (zlfh.getHostSystem())
    {
        case Poco::Zip::ZipCommon::HS_FAT: { hostsystem = "fat"; break; }
        case Poco::Zip::ZipCommon::HS_AMIGA: { hostsystem = "amiga"; break; }
        case Poco::Zip::ZipCommon::HS_VMS: { hostsystem = "vms"; break; };
        case Poco::Zip::ZipCommon::HS_UNIX: { hostsystem = "unix"; break; };
        case Poco::Zip::ZipCommon::HS_VM_CMS: { hostsystem = "vm_cms"; break; };
        case Poco::Zip::ZipCommon::HS_ATARI: { hostsystem = "atari"; break; };
        case Poco::Zip::ZipCommon::HS_HPFS: { hostsystem = "hpfs"; break; };
        case Poco::Zip::ZipCommon::HS_MACINTOSH: { hostsystem = "macintosh"; break; };
        case Poco::Zip::ZipCommon::HS_ZSYSTEM: { hostsystem = "zsystem"; break; };
        case Poco::Zip::ZipCommon::HS_CP_M: { hostsystem = "cp_m"; break; };
        case Poco::Zip::ZipCommon::HS_TOPS20: { hostsystem = "tops20"; break; };
        case Poco::Zip::ZipCommon::HS_NTFS: { hostsystem = "ntfs"; break; };
        case Poco::Zip::ZipCommon::HS_SMS_QDOS: { hostsystem = "sms_qdos"; break; };
        case Poco::Zip::ZipCommon::HS_ACORN: { hostsystem = "acorn"; break; };
        case Poco::Zip::ZipCommon::HS_VFAT: { hostsystem = "vfat"; break; };
        case Poco::Zip::ZipCommon::HS_MVS: { hostsystem = "mvs"; break; };
        case Poco::Zip::ZipCommon::HS_BEOS: { hostsystem = "beos"; break; };
        case Poco::Zip::ZipCommon::HS_TANDEM: { hostsystem = "tandem"; break; };
        case Poco::Zip::ZipCommon::HS_UNUSED: { hostsystem = "unused"; break; };
        default: hostsystem = "unknown";
    }

    lua_pushstring(L, hostsystem);
    lua_setfield(L, -2, "hostSystem");

    lua_pushinteger(L, static_cast<lua_Integer>(zlfh.getMajorVersionNumber()));
    lua_setfield(L, -2, "majorVersionNumber");

    lua_pushinteger(L, static_cast<lua_Integer>(zlfh.getMinorVersionNumber()));
    lua_setfield(L, -2, "minorVersionNumber");

    lua_pushboolean(L, static_cast<int>(zlfh.hasData()));
    lua_setfield(L, -2, "hasData");

    lua_pushboolean(L, static_cast<int>(zlfh.hasExtraField()));
    lua_setfield(L, -2, "hasExtraField");

    lua_pushboolean(L, static_cast<int>(zlfh.isDirectory()));
    lua_setfield(L, -2, "isDirectory");

    lua_pushboolean(L, static_cast<int>(zlfh.isEncrypted()));
    lua_setfield(L, -2, "isEncrypted");

    lua_pushboolean(L, static_cast<int>(zlfh.isFile()));
    lua_setfield(L, -2, "isFile");

    lua_pushboolean(L, static_cast<int>(zlfh.needsZip64()));
    lua_setfield(L, -2, "needsZip64");

    lua_pushboolean(L, static_cast<int>(zlfh.searchCRCAndSizesAfterData()));
    lua_setfield(L, -2, "searchCRCAndSizesAfterData");

    TimestampUserdata* tsud = new(lua_newuserdata(L, sizeof *tsud))
        TimestampUserdata(zlfh.lastModifiedAt().timestamp());
    setupPocoUserdata(L, tsud, POCO_TIMESTAMP_METATABLE_NAME);
    lua_setfield(L, -2, "lastModifiedAt");

    return true;
}

int DecompressUserdata::good_iter(lua_State* L)
{
    int rv = 0;
    luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
    DecompressUserdata* dcud = static_cast<DecompressUserdata*>(lua_touserdata(L, 1));

    if (dcud->mGoodIdx < dcud->mGood.size())
    {
        try
        {
            const std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path>& entry = dcud->mGood[dcud->mGoodIdx];
            ++dcud->mGoodIdx;
            PathUserdata* pud = new(lua_newuserdata(L, sizeof *pud)) PathUserdata(entry.second);
            setupPocoUserdata(L, pud, POCO_PATH_METATABLE_NAME);
            lua_createtable(L, 0, 25);

            dcud->zipLocalFileHeaderToTable(L, entry.first);
            rv = 2;
        }
        catch (const Poco::Exception& e)
        {
            rv = pushPocoException(L, e);
        }
        catch (...)
        {
            rv = pushUnknownException(L);
        }

        // if an exception is caught, call lua_error as a nil return ends the generic for loop
        // without signaling the user that anything went wrong.
        if (lua_isnil(L, -2)) { rv = lua_error(L); }
    }

    return rv;
}

int DecompressUserdata::fail_iter(lua_State* L)
{
    int rv = 0;
    luaL_checktype(L, 1, LUA_TLIGHTUSERDATA);
    DecompressUserdata* dcud = static_cast<DecompressUserdata*>(lua_touserdata(L, 1));

    if (dcud->mFailIdx < dcud->mFail.size())
    {
        try
        {
            const std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string>& entry = dcud->mFail[dcud->mFailIdx];

            ++dcud->mFailIdx;
            lua_pushstring(L, entry.second.c_str());
            lua_createtable(L, 0, 25);

            dcud->zipLocalFileHeaderToTable(L, entry.first);
            rv = 2;
        }
        catch (const Poco::Exception& e)
        {
            rv = pushPocoException(L, e);
        }
        catch (...)
        {
            rv = pushUnknownException(L);
        }

        // if an exception is caught, call lua_error as a nil return ends the generic for loop
        // without signaling the user that anything went wrong.
        if (lua_isnil(L, -2)) { rv = lua_error(L); }
    }

    return rv;
}

/// Generator function for a generic iterator for decompressed Zip entries.
// The iterator function returns: ZipLocalFileHeader table, userdata (path userdata to decompressed entry).
// @return function or nil. (error)
// @return error message.
// @function decompressed
// @see ZipLocalFileHeader
// @see path
int DecompressUserdata::decompressed(lua_State* L)
{
    DecompressUserdata* dcud = checkPrivateUserdata<DecompressUserdata>(L, 1);
    dcud->mGoodIdx = 0;
    lua_pushcfunction(L, good_iter);
    lua_pushlightuserdata(L, static_cast<void*>(dcud));

    return 2;
}

/// Generator function for a generic iterator for failed Zip entries.
// The iterator function returns: ZipLocalFileHeader table, string (error message).
// @return function or nil. (error)
// @return error message.
// @function failed
// @see ZipLocalFileHeader
int DecompressUserdata::failed(lua_State* L)
{
    DecompressUserdata* dcud = checkPrivateUserdata<DecompressUserdata>(L, 1);
    dcud->mFailIdx = 0;
    lua_pushcfunction(L, fail_iter);
    lua_pushlightuserdata(L, static_cast<void*>(dcud));

    return 2;
}

void DecompressUserdata::onError(const void* dp, std::pair<const Poco::Zip::ZipLocalFileHeader, const std::string>& entry)
{
    mFail.push_back(entry);
}

void DecompressUserdata::onDecompress(const void* dp, std::pair<const Poco::Zip::ZipLocalFileHeader, const Poco::Path>& entry)
{
    mGood.push_back(entry);
}


} // Zip
} // LuaPoco
