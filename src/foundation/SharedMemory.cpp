/// SharedMemory memory mapped inter process communication.
// This module that provides access to named shared memory and memory mapped files.
// This is a fairly low level module which requires the use of platform level synchronization primitives
// in order to use safely.  See namedmutex, namedevent, memoryistream, and memoryostream in particular.
//
// Some practical uses cases for this module: Sharing a large amount of data between
// two processes, where making copies across pipes/sockets would incur a performance penalty.  Having
// multiple client processes reading from shared memory without having to copy the data to each
// process.
// @module sharedmemory

#include "SharedMemory.h"
#include "File.h"
#include <cstring>
#include <iostream>

int luaopen_poco_sharedmemory(lua_State* L)
{
    LuaPoco::FileUserdata::registerFile(L);
    LuaPoco::SharedMemoryUserdata::registerSharedMemory(L);
    return LuaPoco::loadConstructor(L, LuaPoco::SharedMemoryUserdata::SharedMemory);
}

namespace LuaPoco
{

const char* POCO_SHAREDMEMORY_METATABLE_NAME = "Poco.SharedMemory.metatable";

/// Creates a new SharedMemory userdata instance.
// @string name specifies the shared memory name, or a file userdata specifing the file to map into memory.
// @string mode access mode either "read" or "write"
// @tparam number size shared memory region size in bytes.
// This parameter is ignored for File userdata.
// @bool[opt] server boolean specifying if this instance should unlink the shared memory on destruction.
// (only POSIX platforms) when true, the shared memory region will be unlinked by calling shm_unlink()
// when the sharedmemory userdata is destroyed.  Parameter is ignored on Windows.
// @return userdata or nil. (error)
// @return error message.
// @function new
int SharedMemoryUserdata::SharedMemory(lua_State* L)
{
    int rv = 0;
    int firstArg = lua_istable(L, 1) ? 2 : 1;

    try
    {
        const char *sharedMemoryName = NULL;
        FileUserdata* fud = NULL;
        Poco::SharedMemory::AccessMode mode = Poco::SharedMemory::AM_READ;
        lua_Integer mappingSize = 0;
        bool serverMapping = false;
        int top = lua_gettop(L);
        
        if (lua_isstring(L, firstArg)) { sharedMemoryName = lua_tostring(L, firstArg); }
        else { fud = checkPrivateUserdata<FileUserdata>(L, firstArg); }
        
        if (top > firstArg && std::strcmp("write", luaL_checkstring(L, firstArg + 1)) == 0)
            { mode = Poco::SharedMemory::AM_WRITE; }
            
        if (top > firstArg + 1) { mappingSize = luaL_checkinteger(L, firstArg + 2); }
        
        if (top > 2) { serverMapping = static_cast<bool>(lua_toboolean(L, firstArg + 3)); }

        SharedMemoryUserdata* smud = NULL;
        if (sharedMemoryName)
        {
            smud = new(lua_newuserdata(L, sizeof *smud))
                SharedMemoryUserdata(sharedMemoryName, mappingSize, mode, serverMapping);
        }
        else
        {
            smud = new(lua_newuserdata(L, sizeof *smud)) SharedMemoryUserdata(fud->getFile(), mode);
        }

        setupPocoUserdata(L, smud, POCO_SHAREDMEMORY_METATABLE_NAME);
        rv = 1;
    }
    catch (const std::exception& e)
    {
        rv = pushException(L, e);
    }
    
    return rv;
}

// register metatable for this class
bool SharedMemoryUserdata::registerSharedMemory(lua_State* L)
{
    struct CFunctions methods[] =
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { NULL, NULL}
    };

    setupUserdataMetatable(L, POCO_SHAREDMEMORY_METATABLE_NAME, methods);
    return true;
}

SharedMemoryUserdata::SharedMemoryUserdata(const Poco::File& f, Poco::SharedMemory::AccessMode mode)
    : mSharedMemory(f, mode)
    , mSize(f.getSize())
    , mMode(mode)
{
}

SharedMemoryUserdata::SharedMemoryUserdata(const char* name, size_t size, Poco::SharedMemory::AccessMode mode, bool server)
    : mSharedMemory(name, size, mode, 0, server)
    , mSize(size)
    , mMode(mode)
{
}

SharedMemoryUserdata::~SharedMemoryUserdata()
{
}

// metamethod infrastructure
int SharedMemoryUserdata::metamethod__tostring(lua_State* L)
{
    SharedMemoryUserdata* smud = checkPrivateUserdata<SharedMemoryUserdata>(L, 1);
    lua_pushfstring(L, "Poco.SharedMemoryUserdata (%p)", static_cast<void*>(smud));

    return 1;
}


///
// @type sharedmemory


} // LuaPoco
