#ifndef LUA_POCO_SHAREDMEMORY_H
#define LUA_POCO_SHAREDMEMORY_H

#include "LuaPoco.h"
#include "Userdata.h"
#include <Poco/SharedMemory.h>
#include <Poco/File.h>

extern "C"
{
LUAPOCO_API int luaopen_poco_sharedmemory(lua_State* L);
}

namespace LuaPoco
{

class SharedMemoryUserdata : public Userdata
{
public:
    SharedMemoryUserdata();
    virtual ~SharedMemoryUserdata();
    // register metatables
    static bool registerSharedMemory(lua_State* L);
    // constructor lua_CFunction
    static int SharedMemory(lua_State* L);

    size_t mSize;
    Poco::SharedMemory mSharedMemory;
    Poco::SharedMemory::AccessMode mMode;
private:
    SharedMemoryUserdata(const Poco::File& f, Poco::SharedMemory::AccessMode mode);
    SharedMemoryUserdata(const char* name, size_t size, Poco::SharedMemory::AccessMode mode, bool server);

    static int metamethod__tostring(lua_State* L);
};

} // LuaPoco

#endif
