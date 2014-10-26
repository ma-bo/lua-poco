#ifndef LUA_POCO_ISTREAM_H
#define LUA_POCO_ISTREAM_H

#include <istream>
#include "LuaPoco.h"
#include "Loader.h"
#include "Userdata.h"

namespace LuaPoco
{

class IStream
{
public:
    virtual std::istream& istream() = 0;
private:
    // userdata methods
    static int read(lua_State* L);
    static int lines(lua_State* L);
    static int seek(lua_State* L);
    static int setvbuf(lua_State* L);
};

} // LuaPoco

#endif
