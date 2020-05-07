#ifndef LUA_POCO_ISTREAM_H
#define LUA_POCO_ISTREAM_H

#include <istream>
#include "LuaPoco.h"
#include "Userdata.h"

namespace LuaPoco
{

extern const char* POCO_ISTREAM_METATABLE_NAME;

class IStream
{
public:
    virtual std::istream& istream() = 0;
protected:
    // userdata methods
    static int read(lua_State* L);
    static int lines(lua_State* L);
    static int line_iterator(lua_State* L);
    static int seek(lua_State* L);
};

// represents a generic ostream to Lua when a POCO API returns an std::istream
// for i/o purposes.
class IStreamUserdata : public Userdata, public IStream
{
public:
    // as IStreamUserdata has no Lua constructor, it is expected that whichever
    // class creates an instance of this, that it will create a reference in the
    // registry to whatever userdata that owns the std::istream reference in order
    // to prevent usage after collection.
    IStreamUserdata(std::istream& istream, int udReference = LUA_NOREF);
    virtual ~IStreamUserdata();
    virtual std::istream& istream();
    // register metatable for this class
    static bool registerIStream(lua_State* L);
private:
    // metamethod infrastructure
    static int metamethod__gc(lua_State* L);
    static int metamethod__tostring(lua_State* L);
    int mIStreamReference;
    std::istream& mIStream;
};

} // LuaPoco

#endif
