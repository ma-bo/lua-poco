#ifndef LUA_POCO_BASE64ENCODER_H
#define LUA_POCO_BASE64ENCODER_H

#include "LuaPoco.h"
#include "Userdata.h"
#include "OStream.h"
#include <Poco/Base64Encoder.h>

extern "C"
{
LUAPOCO_API int luaopen_poco_base64encoder(lua_State* L);
}

namespace LuaPoco
{

extern const char* POCO_BASE64ENCODER_METATABLE_NAME;

class Base64EncoderUserdata : public Userdata, public OStream
{
public:
    Base64EncoderUserdata(std::ostream & ostr, int options, int ref);
    virtual ~Base64EncoderUserdata();
    virtual std::ostream& ostream();
    // register metatable for this class
    static bool registerBase64Encoder(lua_State* L);
    // constructor function 
    static int Base64Encoder(lua_State* L);
    
    Poco::Base64Encoder mBase64Encoder;
private:
    int mUdReference;
    bool mClosed;
    // metamethod infrastructure
    static int metamethod__tostring(lua_State* L);
    static int metamethod__gc(lua_State* L);
    // methods
    static int close(lua_State* L);
};

} // LuaPoco

#endif
