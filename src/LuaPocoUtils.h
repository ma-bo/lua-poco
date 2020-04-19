#include <limits>
#include <type_traits>
#include "LuaPoco.h"

namespace LuaPoco
{

template <typename T>
bool checkUnsignedToLuaInteger(T num, lua_Integer& li)
{
    static_assert(std::numeric_limits<T>::radix == 2,
                  "checkUnsignedToLuaInteger requires binary representation for source type.");
    
    if (num <= std::numeric_limits<lua_Integer>::max())
    {
        li = static_cast<lua_Integer>(num);
        #if LUA_VERSION_NUM < 503
        // corollary to Javascript's MAX_SAFE_INTEGER.
        // As Lua 5.1, and 5.2 use doubles to store numbers, make sure this lua_Integer can be
        // stored without losing precision.
        
        // if the lua_Integer type has more bits than lua_Number, calculate the delta in bits, and
        // shift the maximum lua_Integer value down to match the number of bits available in the
        // lua_Number, and see if the current value is within range.
        const int delta = std::numeric_limits<lua_Integer>::digits - std::numeric_limits<lua_Number>::digits;
        if (delta > 0 && (num < (std::numeric_limits<T>::max() >> delta))) { return true; }
        #else
        return true;
        #endif
    }

    return false;
}

template <typename T>
bool checkSignedToLuaInteger(T num, lua_Integer& li)
{
    static_assert(std::numeric_limits<T>::radix == 2,
                  "checkSignedToLuaInteger requires binary representation for source type.");
    
    if (num > 0) return checkUnsignedToLuaInteger<T>(num, li);
    else if (num >= std::numeric_limits<lua_Integer>::min())
    {
        li = static_cast<lua_Integer>(num);
        #if LUA_VERSION_NUM < 503
        // corollary to Javascript's MIN_SAFE_INTEGER, see decription for checkUnsignedToLuaInteger.
        const int delta = std::numeric_limits<lua_Integer>::digits - std::numeric_limits<lua_Number>::digits;
        if (delta > 0 && (num > (std::numeric_limits<T>::min() >> delta))) { return true; }
        #else
        return true;
        #endif
    }

    return false;
}

}
