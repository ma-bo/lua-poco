#ifndef LUA_POCO_OSTREAM_H
#define LUA_POCO_OSTREAM_H

#include <ostream>
#include "LuaPoco.h"
#include "Loader.h"
#include "Userdata.h"

namespace LuaPoco
{

class OStream
{
public:
    virtual std::ostream& ostream() = 0;
    // userdata methods
    static int write(lua_State* L);
    static int flush(lua_State* L);
    static int seek(lua_State* L);
};

} // LuaPoco

#endif
