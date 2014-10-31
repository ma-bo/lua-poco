#ifndef LUA_POCO_OSTREAM_H
#define LUA_POCO_OSTREAM_H

#include <ostream>
#include "LuaPoco.h"
#include "Loader.h"
#include "Userdata.h"

namespace LuaPoco
{

extern const char* POCO_OSTREAM_METATABLE_NAME;

// for use by other OStream classes to provide their metamethods and access
// to the underlying std::ostream handle.
class OStream
{
public:
    virtual std::ostream& ostream() = 0;
protected:
    // userdata methods
    static int write(lua_State* L);
    static int flush(lua_State* L);
    static int seek(lua_State* L);
};

// represents a generic ostream to Lua when a POCO API returns an std::ostream
// for i/o purposes.
class OStreamUserdata : public Userdata, public OStream
{
public:
    // as OStreamUserdata has no Lua constructor, it is expected that whichever
    // class creates an instance of this, that it will create a reference in the
    // registry to whatever userdata that owns the std::ostream reference in order
    // to prevent usage after collection.
    OStreamUserdata(std::ostream& ostream, int udReference = LUA_NOREF);
    virtual ~OStreamUserdata();
    virtual std::ostream& ostream();
    // register metatable for this class
    static bool registerOStream(lua_State* L);
private:
    // metamethod infrastructure
    static int metamethod__gc(lua_State* L);
    static int metamethod__tostring(lua_State* L);
    int mOStreamReference;
    std::ostream& mOStream;
};

} // LuaPoco

#endif
