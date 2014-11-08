#ifndef LUA_POCO_INFLATINGISTREAM_H
#define LUA_POCO_INFLATINGISTREAM_H

#include "LuaPoco.h"
#include "Loader.h"
#include "Userdata.h"
#include "IStream.h"
#include <Poco/InflatingStream.h>

extern "C"
{
LUAPOCO_API int luaopen_poco_inflatingistream(lua_State* L);
}

namespace LuaPoco
{

extern const char* POCO_INFLATINGISTREAM_METATABLE_NAME;

class InflatingIStreamUserdata : public Userdata, public IStream
{
public:
    InflatingIStreamUserdata(std::istream& istream, Poco::InflatingStreamBuf::StreamType type, int ref);
    InflatingIStreamUserdata(std::istream& istream, int windowBits, int ref);
    virtual ~InflatingIStreamUserdata();
    virtual std::istream& istream();
    // register metatable for this class
    static bool registerInflatingIStream(lua_State* L);
    // constructor function 
    static int InflatingIStream(lua_State* L);
    
    Poco::InflatingInputStream mInflatingInputStream;
private:
    // metamethod infrastructure
    static int metamethod__gc(lua_State* L);
    static int metamethod__tostring(lua_State* L);
    // methods
    static int reset(lua_State* L);
    int mUdReference;
};

} // LuaPoco

#endif
