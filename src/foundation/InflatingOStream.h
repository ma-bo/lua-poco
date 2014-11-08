#ifndef LUA_POCO_INFLATINGOSTREAM_H
#define LUA_POCO_INFLATINGOSTREAM_H

#include "LuaPoco.h"
#include "Loader.h"
#include "Userdata.h"
#include "OStream.h"
#include <Poco/InflatingStream.h>

extern "C"
{
LUAPOCO_API int luaopen_poco_inflatingostream(lua_State* L);
}

namespace LuaPoco
{

extern const char* POCO_INFLATINGOSTREAM_METATABLE_NAME;

class InflatingOStreamUserdata : public Userdata, public OStream
{
public:
    InflatingOStreamUserdata(std::ostream& ostream, Poco::InflatingStreamBuf::StreamType type, int ref);
    InflatingOStreamUserdata(std::ostream& ostream, int windowBits, int ref);
    virtual ~InflatingOStreamUserdata();
    virtual std::ostream& ostream();
    // register metatable for this class
    static bool registerInflatingOStream(lua_State* L);
    // constructor function 
    static int InflatingOStream(lua_State* L);
    
    Poco::InflatingOutputStream mInflatingOutputStream;
private:
    // metamethod infrastructure
    static int metamethod__gc(lua_State* L);
    static int metamethod__tostring(lua_State* L);
    // methods
    static int close(lua_State* L);
    int mUdReference;
};

} // LuaPoco

#endif
