#ifndef LUA_POCO_DEFLATINGISTREAM_H
#define LUA_POCO_DEFLATINGISTREAM_H

#include "LuaPoco.h"
#include "Loader.h"
#include "Userdata.h"
#include "IStream.h"
#include <Poco/DeflatingStream.h>

extern "C"
{
LUAPOCO_API int luaopen_poco_deflatingistream(lua_State* L);
}

namespace LuaPoco
{

extern const char* POCO_DEFLATINGISTREAM_METATABLE_NAME;

class DeflatingIStreamUserdata : public Userdata, public IStream
{
public:
    DeflatingIStreamUserdata(std::istream& istream, Poco::DeflatingStreamBuf::StreamType type, int level, int ref);
    DeflatingIStreamUserdata(std::istream& istream, int windowBits, int level, int ref);
    virtual ~DeflatingIStreamUserdata();
    virtual std::istream& istream();
    // register metatable for this class
    static bool registerDeflatingIStream(lua_State* L);
    // constructor function 
    static int DeflatingIStream(lua_State* L);
    
    Poco::DeflatingInputStream mDeflatingInputStream;
private:
    // metamethod infrastructure
    static int metamethod__gc(lua_State* L);
    static int metamethod__tostring(lua_State* L);
    
    int mUdReference;
};

} // LuaPoco

#endif
