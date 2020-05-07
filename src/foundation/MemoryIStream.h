#ifndef LUA_POCO_MEMORYISTREAM_H
#define LUA_POCO_MEMORYISTREAM_H

#include "LuaPoco.h"
#include "Userdata.h"
#include "IStream.h"
#include <Poco/MemoryStream.h>

extern "C"
{
LUAPOCO_API int luaopen_poco_memoryistream(lua_State* L);
}

namespace LuaPoco
{

extern const char* POCO_MEMORYISTREAM_METATABLE_NAME;

class MemoryIStreamUserdata : public Userdata, public IStream
{
public:
    MemoryIStreamUserdata(const char* buffer, size_t size);
    virtual ~MemoryIStreamUserdata();
    virtual std::istream& istream();
    // register metatable for this class
    static bool registerMemoryIStream(lua_State* L);
    // constructor function 
    static int MemoryIStream(lua_State* L);
    
    Poco::MemoryInputStream mMemoryInputStream;
private:
    // metamethod infrastructure
    static int metamethod__gc(lua_State* L);
    static int metamethod__tostring(lua_State* L);
    
    int mUdReference;
};

} // LuaPoco

#endif
