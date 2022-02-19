#ifndef LUA_POCO_BASE32DECODER_H
#define LUA_POCO_BASE32DECODER_H

#include "LuaPoco.h"
#include "Userdata.h"
#include "IStream.h"
#include <Poco/Base32Decoder.h>

extern "C"
{
LUAPOCO_API int luaopen_poco_base32decoder(lua_State* L);
}

namespace LuaPoco
{

extern const char* POCO_BASE32DECODER_METATABLE_NAME;

class Base32DecoderUserdata : public Userdata, public IStream
{
public:
    Base32DecoderUserdata(std::istream & istr, int ref);
    virtual ~Base32DecoderUserdata();
    virtual std::istream& istream();
    // register metatable for this class
    static bool registerBase32Decoder(lua_State* L);
    // constructor function 
    static int Base32Decoder(lua_State* L);
    
    Poco::Base32Decoder mBase32Decoder;
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
