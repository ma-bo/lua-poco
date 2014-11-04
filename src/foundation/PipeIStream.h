#ifndef LUA_POCO_PIPEISTREAM_H
#define LUA_POCO_PIPEISTREAM_H

#include "LuaPoco.h"
#include "Loader.h"
#include "Userdata.h"
#include "IStream.h"
#include <Poco/PipeStream.h>

extern "C"
{
LUAPOCO_API int luaopen_poco_pipeistream(lua_State* L);
}

namespace LuaPoco
{

extern const char* POCO_PIPEISTREAM_METATABLE_NAME;

class PipeIStreamUserdata : public Userdata, public IStream
{
public:
    PipeIStreamUserdata(const Poco::Pipe& p);
    virtual ~PipeIStreamUserdata();
    virtual std::istream& istream();
    // register metatable for this class
    static bool registerPipeIStream(lua_State* L);
    // constructor function 
    static int PipeIStream(lua_State* L);
    
    Poco::PipeInputStream mPipeInputStream;
private:
    // metamethod infrastructure
    static int metamethod__gc(lua_State* L);
    static int metamethod__tostring(lua_State* L);
    int mPipeReference;
};

} // LuaPoco

#endif
