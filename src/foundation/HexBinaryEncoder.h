#ifndef LUA_POCO_BASE32ENCODER_H
#define LUA_POCO_BASE32ENCODER_H

#include "LuaPoco.h"
#include "Userdata.h"
#include "OStream.h"
#include <Poco/HexBinaryEncoder.h>

extern "C"
{
LUAPOCO_API int luaopen_poco_hexbinaryencoder(lua_State* L);
}

namespace LuaPoco
{

extern const char* POCO_HEXBINARYENCODER_METATABLE_NAME;

class HexBinaryEncoderUserdata : public Userdata, public OStream
{
public:
    HexBinaryEncoderUserdata(std::ostream & ostr, int ref);
    virtual ~HexBinaryEncoderUserdata();
    virtual std::ostream& ostream();
    // register metatable for this class
    static bool registerHexBinaryEncoder(lua_State* L);
    // constructor function 
    static int HexBinaryEncoder(lua_State* L);
    
    Poco::HexBinaryEncoder mHexBinaryEncoder;
private:
    int mUdReference;
    // metamethod infrastructure
    static int metamethod__tostring(lua_State* L);
    static int metamethod__gc(lua_State* L);
};

} // LuaPoco

#endif
