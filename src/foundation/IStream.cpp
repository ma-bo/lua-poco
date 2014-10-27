/// Generic interface from reading from istream userdata.
// @module istream

#include "IStream.h"
#include "Poco/Exception.h"

namespace LuaPoco
{

// userdata methods
int IStream::read(lua_State* L)
{
    int rv = 0;
    IStream* isud = checkPrivateUserdata<IStream>(L, 1);
    std::istream& is = isud->istream();
    
    try
    {
        rv = 1;
    }
    catch (const Poco::Exception& e)
    {
        rv = pushPocoException(L, e);
    }
    catch (...)
    {
        rv = pushUnknownException(L);
    }
    
    return rv;
}

int IStream::lines(lua_State* L)
{
    int rv = 0;
    IStream* isud = checkPrivateUserdata<IStream>(L, 1);
    std::istream& is = isud->istream();
    
    try
    {
        rv = 1;
    }
    catch (const Poco::Exception& e)
    {
        rv = pushPocoException(L, e);
    }
    catch (...)
    {
        rv = pushUnknownException(L);
    }
    
    return rv;
}

int IStream::seek(lua_State* L)
{
    int rv = 0;
    IStream* isud = checkPrivateUserdata<IStream>(L, 1);
    std::istream& is = isud->istream();
    
    try
    {
        rv = 1;
    }
    catch (const Poco::Exception& e)
    {
        rv = pushPocoException(L, e);
    }
    catch (...)
    {
        rv = pushUnknownException(L);
    }
    
    return rv;
}

int IStream::setvbuf(lua_State* L)
{
    int rv = 0;
    IStream* isud = checkPrivateUserdata<IStream>(L, 1);
    std::istream& is = isud->istream();
    
    try
    {
        rv = 1;
    }
    catch (const Poco::Exception& e)
    {
        rv = pushPocoException(L, e);
    }
    catch (...)
    {
        rv = pushUnknownException(L);
    }
    
    return rv;
}

} // LuaPoco
