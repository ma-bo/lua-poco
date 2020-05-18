#ifndef LUA_POCO_BUFFER_H
#define LUA_POCO_BUFFER_H

#include "LuaPoco.h"
#include "Userdata.h"
#include <Poco/Buffer.h>

extern "C"
{
LUAPOCO_API int luaopen_poco_buffer(lua_State* L);
}

namespace LuaPoco
{

extern const char* POCO_BUFFER_METATABLE_NAME;

class BufferUserdata : public Userdata
{
public:
    BufferUserdata(size_t size);
    virtual ~BufferUserdata();
    virtual bool copyToState(lua_State *L);
    // register metatable for this class
    static bool registerBuffer(lua_State* L);
    // constructor function 
    static int Buffer(lua_State* L);
    
    Poco::Buffer<char> mBuffer;
    size_t mCapacity;
private:
    // metamethod infrastructure
    static int metamethod__tostring(lua_State* L);
    
    // userdata methods
    static int clear(lua_State* L);
    static int size(lua_State* L);
    static int data(lua_State* L);
};

} // LuaPoco

#endif
