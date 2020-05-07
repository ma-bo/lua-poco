#ifndef LUA_POCO_TEEOSTREAM_H
#define LUA_POCO_TEEOSTREAM_H

#include "LuaPoco.h"
#include "Userdata.h"
#include "OStream.h"
#include <Poco/TeeStream.h>

extern "C"
{
LUAPOCO_API int luaopen_poco_teeostream(lua_State* L);
}

namespace LuaPoco
{

extern const char* POCO_TEEOSTREAM_METATABLE_NAME;

class TeeOStreamUserdata : public Userdata, public OStream
{
public:
    TeeOStreamUserdata();
    virtual ~TeeOStreamUserdata();
    virtual std::ostream& ostream();
    // register metatable for this class
    static bool registerTeeOStream(lua_State* L);
    // constructor function 
    static int TeeOStream(lua_State* L);
    
    Poco::TeeOutputStream mTeeOutputStream;
private:
    // metamethod infrastructure
    static int metamethod__gc(lua_State* L);
    static int metamethod__tostring(lua_State* L);
    static int addStream(lua_State* L);
    
    std::vector<int> mUdReference;
};

} // LuaPoco

#endif
