#ifndef LUA_POCO_PIPEOSTREAM_H
#define LUA_POCO_PIPEOSTREAM_H

#include "LuaPoco.h"
#include "Loader.h"
#include "Userdata.h"
#include "OStream.h"
#include <Poco/PipeStream.h>

extern "C"
{
LUAPOCO_API int luaopen_poco_pipeostream(lua_State* L);
}

namespace LuaPoco
{

class PipeOStreamUserdata : public Userdata, public OStream
{
public:
    PipeOStreamUserdata(const Poco::Pipe& p);
    virtual ~PipeOStreamUserdata();
    virtual UserdataType getType();
    virtual BaseType getBaseType();
    virtual bool isCopyable();
    virtual std::ostream& ostream();
    // register metatable for this class
    static bool registerPipeOStream(lua_State* L);
    // constructor function 
    static int PipeOStream(lua_State* L);
    
    Poco::PipeOutputStream mPipeOutputStream;
private:
    // metamethod infrastructure
    static int metamethod__gc(lua_State* L);
    static int metamethod__tostring(lua_State* L);
    int mPipeReference;
};

} // LuaPoco

#endif
