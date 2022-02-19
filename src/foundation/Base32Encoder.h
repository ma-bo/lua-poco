#ifndef LUA_POCO_BASE32ENCODER_H
#define LUA_POCO_BASE32ENCODER_H

#include "LuaPoco.h"
#include "Userdata.h"
#include "OStream.h"
#include <Poco/Base32Encoder.h>

extern "C"
{
LUAPOCO_API int luaopen_poco_base32encoder(lua_State* L);
}

namespace LuaPoco
{

extern const char* POCO_BASE32ENCODER_METATABLE_NAME;

class Base32EncoderUserdata : public Userdata, public OStream
{
public:
    Base32EncoderUserdata(std::ostream & ostr, bool padding, int ref);
    virtual ~Base32EncoderUserdata();
    virtual std::ostream& ostream();
    // register metatable for this class
    static bool registerBase32Encoder(lua_State* L);
    // constructor function 
    static int Base32Encoder(lua_State* L);
    
    Poco::Base32Encoder mBase32Encoder;
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
