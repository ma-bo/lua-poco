#ifndef LUA_POCO_HEXBINARYDECODER_H
#define LUA_POCO_HEXBINARYDECODER_H

#include "LuaPoco.h"
#include "Userdata.h"
#include "IStream.h"
#include <Poco/HexBinaryDecoder.h>

extern "C"
{
LUAPOCO_API int luaopen_poco_hexbinarydecoder(lua_State* L);
}

namespace LuaPoco
{

extern const char* POCO_HEXBINARYDECODER_METATABLE_NAME;

class HexBinaryDecoderUserdata : public Userdata, public IStream
{
public:
    HexBinaryDecoderUserdata(std::istream & istr, int ref);
    virtual ~HexBinaryDecoderUserdata();
    virtual std::istream& istream();
    // register metatable for this class
    static bool registerHexBinaryDecoder(lua_State* L);
    // constructor function 
    static int HexBinaryDecoder(lua_State* L);
    
    Poco::HexBinaryDecoder mHexBinaryDecoder;
private:
    int mUdReference;
    // metamethod infrastructure
    static int metamethod__tostring(lua_State* L);
    static int metamethod__gc(lua_State* L);
};

} // LuaPoco

#endif
