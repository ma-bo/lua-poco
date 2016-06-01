#ifndef LUA_POCO_MEMORYOSTREAM_H
#define LUA_POCO_MEMORYOSTREAM_H

#include "LuaPoco.h"
#include "Loader.h"
#include "Userdata.h"
#include "OStream.h"
#include <Poco/MemoryStream.h>

extern "C"
{
LUAPOCO_API int luaopen_poco_memoryostream(lua_State* L);
}

namespace LuaPoco
{

extern const char* POCO_MEMORYISTREAM_METATABLE_NAME;

class MemoryOStreamUserdata : public Userdata, public OStream
{
public:
    MemoryOStreamUserdata(char* buffer, size_t size);
    virtual ~MemoryOStreamUserdata();
    virtual std::ostream& ostream();
    // register metatable for this class
    static bool registerMemoryOStream(lua_State* L);
    // constructor function 
    static int MemoryOStream(lua_State* L);
    
    Poco::MemoryOutputStream mMemoryOutputStream;
private:
    // metamethod infrastructure
    static int metamethod__gc(lua_State* L);
    static int metamethod__tostring(lua_State* L);
    static int bytesWritten(lua_State* L);
    
    int mUdReference;
};

} // LuaPoco

#endif
