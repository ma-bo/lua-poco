#ifndef LUA_POCO_DEFLATINGOSTREAM_H
#define LUA_POCO_DEFLATINGOSTREAM_H

#include "LuaPoco.h"
#include "Loader.h"
#include "Userdata.h"
#include "OStream.h"
#include <Poco/DeflatingStream.h>

extern "C"
{
LUAPOCO_API int luaopen_poco_deflatingostream(lua_State* L);
}

namespace LuaPoco
{

extern const char* POCO_DEFLATINGOSTREAM_METATABLE_NAME;

class DeflatingOStreamUserdata : public Userdata, public OStream
{
public:
    DeflatingOStreamUserdata(std::ostream& ostream, Poco::DeflatingStreamBuf::StreamType type, int level, int ref);
    DeflatingOStreamUserdata(std::ostream& ostream, int windowBits, int level, int ref);
    virtual ~DeflatingOStreamUserdata();
    virtual std::ostream& ostream();
    // register metatable for this class
    static bool registerDeflatingOStream(lua_State* L);
    // constructor function 
    static int DeflatingOStream(lua_State* L);
    
    Poco::DeflatingOutputStream mDeflatingOutputStream;
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
