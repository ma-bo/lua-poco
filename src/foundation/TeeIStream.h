#ifndef LUA_POCO_TEEISTREAM_H
#define LUA_POCO_TEEISTREAM_H

#include "LuaPoco.h"
#include "Userdata.h"
#include "IStream.h"
#include <Poco/TeeStream.h>

extern "C"
{
LUAPOCO_API int luaopen_poco_teeistream(lua_State* L);
}

namespace LuaPoco
{

extern const char* POCO_TEEISTREAM_METATABLE_NAME;

class TeeIStreamUserdata : public Userdata, public IStream
{
public:
    TeeIStreamUserdata(std::istream& is);
    virtual ~TeeIStreamUserdata();
    virtual std::istream& istream();
    // register metatable for this class
    static bool registerTeeIStream(lua_State* L);
    // constructor function 
    static int TeeIStream(lua_State* L);
    
    Poco::TeeInputStream mTeeInputStream;
private:
    // metamethod infrastructure
    static int metamethod__gc(lua_State* L);
    static int metamethod__tostring(lua_State* L);
    static int addStream(lua_State* L);
    
    std::vector<int> mUdReference;
};

} // LuaPoco

#endif
