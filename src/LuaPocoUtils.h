namespace LuaPoco
{

template <typename T>
bool checkUnsignedToLuaInteger(T num, lua_Integer& li)
{
    if (num <= std::numeric_limits<lua_Integer>::max())
    {
        li = static_cast<lua_Integer>(num);
        // corollary to Javascript's MAX_SAFE_INTEGER.
        // Lua 5.1, and 5.2 use doubles to store numbers, make sure this lua_Integer will fit.
        #if LUA_VERSION_NUM < 503
        const int delta = std::numeric_limits<lua_Integer>::digits - std::numeric_limits<lua_Number>::digits;
        if (delta > 0)
        {
            if (num <= ((std::numeric_limits<T>::max() >> delta) - 1))
            {
                return true;
            }
        }
        #else
        return true;
        #endif
    }

    return false;
}

template <typename T>
bool checkSignedToLuaInteger(T num, lua_Integer& li)
{
    bool result = false;
    
    if (num < 0)
    {
        if (num >= std::numeric_limits<lua_Integer>::min())
        {
            li = static_cast<lua_Integer>(num);
            // corollary to Javascript's MIN_SAFE_INTEGER.
            // Lua 5.1, and 5.2 use doubles to store numbers, make sure this lua_Integer will fit.
            #if LUA_VERSION_NUM < 503
            const int delta = std::numeric_limits<lua_Integer>::digits - std::numeric_limits<lua_Number>::digits;
            if (delta > 0)
            {
                if (num >= ((std::numeric_limits<T>::max() >> delta) + 1))
                {
                    return true;
                }
            }
            #else
            return true;
            #endif
        }
    }
    else return checkUnsignedToLuaInteger<T>(num, li);

    return false;
}

}
