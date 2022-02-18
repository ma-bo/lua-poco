#ifndef LUA_POCO_BASE64DECODER_H
#define LUA_POCO_BASE64DECODER_Hs

#include "LuaPoco.h"
#include "Userdata.h"
#include "IStream.h"
#include <Poco/Base64Decoder.h>

extern "C"
{
LUAPOCO_API int luaopen_poco_base64decoder(lua_State* L);
}

namespace LuaPoco
{

extern const char* POCO_BASE64DECODER_METATABLE_NAME;

class Base64DecoderUserdata : public Userdata, public IStream
{
public:
    Base64DecoderUserdata(std::istream & istr, int options, int ref);
    virtual ~Base64DecoderUserdata();
    virtual std::istream& istream();
    // register metatable for this class
    static bool registerBase64Decoder(lua_State* L);
    // constructor function 
    static int Base64Decoder(lua_State* L);
    
    Poco::Base64Decoder mBase64Decoder;
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
