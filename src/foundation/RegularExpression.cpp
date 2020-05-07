/// Regular expressions
// Regular expressions implemented by the PCRE library.
// @module regex

#include "RegularExpression.h"
#include "Poco/Exception.h"

int luaopen_poco_regex(lua_State* L)
{
    LuaPoco::RegularExpressionUserdata::registerRegularExpression(L);
    return LuaPoco::loadConstructor(L, LuaPoco::RegularExpressionUserdata::RegularExpression);
}

namespace
{
    
int parseRegexOptions(const std::string& options)
{
    int opts = 0;
    
    if (options.find("RE_CASELESS") != std::string::npos)
        opts |= Poco::RegularExpression::RE_CASELESS;
    if (options.find("RE_MULTILINE") != std::string::npos)
        opts |= Poco::RegularExpression::RE_MULTILINE;
    if (options.find("RE_DOTALL") != std::string::npos)
        opts |= Poco::RegularExpression::RE_DOTALL;
    if (options.find("RE_EXTENDED") != std::string::npos)
        opts |= Poco::RegularExpression::RE_EXTENDED;
    if (options.find("RE_ANCHORED") != std::string::npos)
        opts |= Poco::RegularExpression::RE_ANCHORED;
    if (options.find("RE_EXTRA") != std::string::npos)
        opts |= Poco::RegularExpression::RE_EXTRA;
    if (options.find("RE_NOTBOL") != std::string::npos)
        opts |= Poco::RegularExpression::RE_NOTBOL;
    if (options.find("RE_NOTEOL") != std::string::npos)
        opts |= Poco::RegularExpression::RE_NOTEOL;
    if (options.find("RE_UNGREEDY") != std::string::npos)
        opts |= Poco::RegularExpression::RE_UNGREEDY;
    if (options.find("RE_NOTEMPTY") != std::string::npos)
        opts |= Poco::RegularExpression::RE_NOTEMPTY;
    if (options.find("RE_UTF8") != std::string::npos)
        opts |= Poco::RegularExpression::RE_UTF8;
    if (options.find("RE_NO_AUTO_CAPTURE") != std::string::npos)
        opts |= Poco::RegularExpression::RE_NO_AUTO_CAPTURE;
    if (options.find("RE_NO_UTF8_CHECK") != std::string::npos)
        opts |= Poco::RegularExpression::RE_NO_UTF8_CHECK;
    if (options.find("RE_FIRSTLINE") != std::string::npos)
        opts |= Poco::RegularExpression::RE_FIRSTLINE;
    if (options.find("RE_DUPNAMES") != std::string::npos)
        opts |= Poco::RegularExpression::RE_DUPNAMES;
    if (options.find("RE_NEWLINE_CR") != std::string::npos)
        opts |= Poco::RegularExpression::RE_NEWLINE_CR;
    if (options.find("RE_NEWLINE_LF") != std::string::npos)
        opts |= Poco::RegularExpression::RE_NEWLINE_LF;
    if (options.find("RE_NEWLINE_CRLF") != std::string::npos)
        opts |= Poco::RegularExpression::RE_NEWLINE_CRLF;
    if (options.find("RE_NEWLINE_ANY") != std::string::npos)
        opts |= Poco::RegularExpression::RE_NEWLINE_ANY;
    if (options.find("RE_NEWLINE_ANYCRLF") != std::string::npos)
        opts |= Poco::RegularExpression::RE_NEWLINE_ANYCRLF;
    if (options.find("RE_GLOBAL") != std::string::npos)
        opts |= Poco::RegularExpression::RE_GLOBAL;
    if (options.find("RE_NO_VARS") != std::string::npos)
        opts |= Poco::RegularExpression::RE_NO_VARS;

    return opts;
}

}

namespace LuaPoco
{

const char* POCO_REGULAREXPRESSION_METATABLE_NAME = "Poco.RegularExpression.metatable";

RegularExpressionUserdata::RegularExpressionUserdata(const std::string& pattern, int options, bool study)
    : mRegularExpression(pattern, options, study)
{
}

RegularExpressionUserdata::~RegularExpressionUserdata()
{
}

// register metatable for this class
bool RegularExpressionUserdata::registerRegularExpression(lua_State* L)
{
    struct CFunctions methods[] = 
    {
        { "__gc", metamethod__gc },
        { "__tostring", metamethod__tostring },
        { "find", find },
        { "match", match },
        { "gmatch", gmatch },
        { "gsub", gsub },
        { NULL, NULL}
    };
    
    setupUserdataMetatable(L, POCO_REGULAREXPRESSION_METATABLE_NAME, methods);
    return true;
}

// constructor function

/// Constructs a new regex userdata
// @string pattern the regular expression pattern
// @string[opt] options regex options for the match.
// @string[opt] study 
// @return userdata or nil. (error)
// @return error message.
// @function new
int RegularExpressionUserdata::RegularExpression(lua_State* L)
{
    int rv = 0;
    int firstArg = lua_istable(L, 1) ? 2 : 1;
    int top = lua_gettop(L);
    
    const char* pattern = luaL_checkstring(L, firstArg);
    const char* optionsStr = "";
    int options = 0;
    bool study = true;
    
    if (top > firstArg)
    {
        optionsStr = luaL_checkstring(L, firstArg + 1);
        options = parseRegexOptions(optionsStr);
    }
    if (top > firstArg + 1 && lua_isboolean(L, firstArg + 2))
        study = lua_toboolean(L, firstArg + 2);
    
    try
    {
        RegularExpressionUserdata *reud = new(lua_newuserdata(L, sizeof *reud)) RegularExpressionUserdata(pattern, options, study);
        setupPocoUserdata(L, reud, POCO_REGULAREXPRESSION_METATABLE_NAME);
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

///
// @type regex

// metamethod infrastructure
int RegularExpressionUserdata::metamethod__tostring(lua_State* L)
{
    RegularExpressionUserdata* reud = checkPrivateUserdata<RegularExpressionUserdata>(L, 1);
    lua_pushfstring(L, "Poco.RegularExpression (%p)", static_cast<void*>(reud));
    return 1;
}

// userdata methods

/// Looks for the first match of pattern in the string s. 
// If it finds a match, then find returns the indices of s where this occurrence starts and ends; otherwise, it returns nil.
// A third, optional numerical argument init specifies where to start the search; 
// its default value is 1 and can be negative. A value of true as a fourth, optional argument plain turns off the pattern matching facilities, 
// so the function does a plain "find substring" operation, with no characters in pattern being considered "magic". Note that if plain is given, then init must be given as well.
// If the pattern has captures, then in a successful match the captured values are also returned, after the two indices.
//
// @string s string to search using the regex.
// @string[opt] options regex options for the match.
// @int[opt] init position in the string to start the find.
// @return indicies of the match including captures or nil.
// @function find
int RegularExpressionUserdata::find(lua_State* L)
{
    int rv = 0;
    RegularExpressionUserdata* reud = checkPrivateUserdata<RegularExpressionUserdata>(L, 1);
    
    size_t subjectSize = 0;
    const char* subject = luaL_checklstring(L, 2, &subjectSize);
    
    int options = 0;
    int startPosition = 0;
    
    int top = lua_gettop(L);
    if (top > 2)
    {
        const char* optionsStr = luaL_checkstring(L, 3);
        options = parseRegexOptions(optionsStr);
    }
    
    if (top > 3)
    {
        startPosition = luaL_checkinteger(L, 4) - 1;
        startPosition = startPosition < 0 ? 0 : startPosition;
    }
    
    try
    {
        Poco::RegularExpression::MatchVec matches;
        int matchCount = reud->mRegularExpression.match(subject, startPosition, matches, options);
        if (matchCount > 0)
        {
            for (size_t i = 0; matchCount > 0 && i < matches.size(); ++i)
            {
                // overwrite values from 1 to matchCount
                lua_pushnumber(L, matches[i].offset + 1);
                lua_pushnumber(L, matches[i].offset + matches[i].length);
            }
            rv = matchCount * 2;
        }
        else
        {
            lua_pushnil(L);
            rv = 1;
        }
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
/// Looks for the first match of the regex in the subject string.
// @string s string to search using the regex.
// @string[opt] options regex options for the match.
// @int[opt] init position in the string to start the find.
// @return captures or nil.
// If there were no captures specified, returns the entire match.
// @function match
int RegularExpressionUserdata::match(lua_State* L)
{
    int rv = 0;
    RegularExpressionUserdata* reud = checkPrivateUserdata<RegularExpressionUserdata>(L, 1);
    
    size_t subjectSize = 0;
    const char* subject = luaL_checklstring(L, 2, &subjectSize);
    
    int options = 0;
    int startPosition = 0;
    
    int top = lua_gettop(L);
    if (top > 2)
    {
        const char* optionsStr = luaL_checkstring(L, 3);
        options = parseRegexOptions(optionsStr);
    }
    if (top > 3)
    {
        startPosition = luaL_checkinteger(L, 4) - 1;
        startPosition = startPosition < 0 ? 0 : startPosition;
    }
    
    try
    {
        Poco::RegularExpression::MatchVec matches;
        int matchCount = reud->mRegularExpression.match(subject, startPosition, matches, options);
        // if there are captures (matchCount > 1), return the captures only, 
        // otherwise return the whole match.
        for (size_t i = matchCount > 1 ? 1 : 0; i < matchCount; ++i)
        {
            lua_pushlstring(L, subject + matches[i].offset, matches[i].length);
            ++rv;
        }
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

// 1. userdata
// 2. string
// 3. options
// 4. start position
int RegularExpressionUserdata::gmatch_iter(lua_State* L)
{
    int rv = 0;
    RegularExpressionUserdata* reud = static_cast<RegularExpressionUserdata*>(lua_touserdata(L, lua_upvalueindex(1)));
    const char* subject = lua_tostring(L, lua_upvalueindex(2));
    int options = lua_tointeger(L, lua_upvalueindex(3));
    int startPosition = lua_tointeger(L, lua_upvalueindex(4));
    
    try
    {
        Poco::RegularExpression::MatchVec matches;
        int matchCount = reud->mRegularExpression.match(subject, startPosition, matches, options);
        if (matchCount > 0)
        {
            lua_pushinteger(L, matches[0].offset + matches[0].length);
            lua_replace(L, lua_upvalueindex(4));
            for (size_t i = matchCount > 1 ? 1 : 0; i < matchCount; ++i)
            {
                lua_pushlstring(L, subject + matches[i].offset, matches[i].length);
                ++rv;
            }
        }
        else
        {
            lua_pushnil(L);
            rv = 1;
        }
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

/// Iterates a string returning pattern matches.
// Returns an iterator function that, each time it is called, returns the next captures from pattern over string s.
// If pattern specifies no captures, then the whole match is produced in each call.
//
// @string s string to search using the regex.
// @string[opt] options regex options for the match.
// @return gmatch_iter function to iterate matches in s.
// @function gmatch
int RegularExpressionUserdata::gmatch(lua_State* L)
{
    int rv = 0;
    RegularExpressionUserdata* reud = checkPrivateUserdata<RegularExpressionUserdata>(L, 1);
    
    size_t subjectSize = 0;
    const char* subject = luaL_checklstring(L, 2, &subjectSize);
    
    int options = 0;
    int startPosition = 0;
    
    int top = lua_gettop(L);
    if (top > 2)
    {
        const char* optionsStr = luaL_checkstring(L, 3);
        options = parseRegexOptions(optionsStr);
    }
    
    // userdata
    lua_pushvalue(L, 1);
    // string
    lua_pushvalue(L, 2);
    // options
    lua_pushinteger(L, options);
    // start position
    lua_pushinteger(L, startPosition);
    
    lua_pushcclosure(L, gmatch_iter, 4);
    
    return 1;
}

/// Replace all matches in string with a substition.
//
// The replacement parameter may be of the following types:
//
// 1. string - The string value will be used to replace the match. The string may contain $0 for the full match, 
// $1 for the first capture, $2 second capture, etc.
//
// 2. table - The first capture in the match will be used as the key in the table to look up the replacement string.
// If there are no captures, the entire match will be used as the key.
//
// function - The function will be called with the captures in the match as parameters in order.
// If there are no captures, the function will be called with the entire match as a single parameter.
//
// @string s subject string to match against.
// @param replace string, table, or function.
// @string[opt] options regex options for the match.
// @int[opt] n number of matches to replace.
// @return the newly replaced string or nil.
// @function gsub
int RegularExpressionUserdata::gsub(lua_State* L)
{
    int rv = 0;
    int top = lua_gettop(L);    
    int options = 0;
    int replaceCount = -1;
    size_t startPosition = 0;
    int replaced = 0;
    
    // regex:gsub
    RegularExpressionUserdata* reud = checkPrivateUserdata<RegularExpressionUserdata>(L, 1);
    
    // @string s
    size_t subjectSize = 0;
    const char* subject = luaL_checklstring(L, 2, &subjectSize);
    std::string subjectMutable(subject);
    
    // @param replace
    int replaceType = lua_type(L, 3);
    if (replaceType != LUA_TSTRING && replaceType != LUA_TTABLE && replaceType != LUA_TFUNCTION)
        return luaL_error(L, "parameter #3 must be a 'string', 'table', or 'function', got: %s", lua_typename(L, replaceType));
    
    // @string[opt] options
    if (top > 3)
    {
        const char* optionsStr = luaL_checkstring(L, 4);
        options = parseRegexOptions(optionsStr);
    }
    
    // @int[opt] n
    if (top > 4)
    {
        replaceCount = luaL_checkinteger(L, 5);
        if (replaceCount < 0) replaceCount = 0;
    }
    
    try
    {
        if (replaceType == LUA_TSTRING)
        {
            std::string replacement(lua_tostring(L, 3));
            
            // optimize for the most common case where the user wants to replace all matches.
            // ie: don't do the excess iteration.
            if (replaceCount < 0)
            {
                // force RE_GLOBAL to replace all.
                replaced = reud->mRegularExpression.subst(subjectMutable, replacement, options | Poco::RegularExpression::RE_GLOBAL);
                lua_pushlstring(L, subjectMutable.c_str(), subjectMutable.size());
                lua_pushnumber(L, replaced);
                rv = 2;
            }
            else
            {
                Poco::RegularExpression::Match match;
                while(0 < replaceCount && reud->mRegularExpression.match(subjectMutable, startPosition, match, options) > 0)
                {
                    int subjectSizeBefore = static_cast<int>(subjectMutable.size());
                    // perform the replacement
                    if (reud->mRegularExpression.subst(subjectMutable, startPosition, replacement, options) > 0)
                    {
                        ++replaced;
                        --replaceCount;
                    }
                    else
                        break;
                    
                    // The match will have been replaced by string of possibly different size.
                    // Calculate the previous end of match, and apply the delta to it for the next start position.
                    int replaceDelta = static_cast<int>(subjectMutable.size()) - subjectSizeBefore;
                    startPosition = match.offset + match.length + replaceDelta;
                }
    
                lua_pushlstring(L, subjectMutable.c_str(), subjectMutable.size());
                lua_pushnumber(L, replaced);
                rv = 2;
            }
        }
        else if (replaceType == LUA_TTABLE)
        {
            Poco::RegularExpression::MatchVec matches;
            int matchCount = 0;
            while ((matchCount = reud->mRegularExpression.match(subjectMutable, startPosition, matches, options)) > 0)
            {
                // if we were supplied a replaceCount (-1 == replace all), 
                // break if desired number of replacements have been made.
                if (replaceCount > -1 && 0 >= replaceCount)
                    break;

                int subjectSizeBefore = static_cast<int>(subjectMutable.size());
                
                // use the first capture (1) if captures are present, otherwise use the whole match (0).
                int matchIdx = matchCount > 1 ? 1 : 0;
                
                // extract match to look up in table.
                lua_pushlstring(L, subjectMutable.c_str() + matches[matchIdx].offset, matches[matchIdx].length);
                lua_gettable(L, 3);
                if (!lua_isnil(L, -1) && lua_isstring(L, -1))
                {
                    std::string replacement(lua_tostring(L, -1));
                    // perform the replacement
                    if (reud->mRegularExpression.subst(subjectMutable, startPosition, replacement, options) > 0)
                    {
                        ++replaced;
                        --replaceCount;
                    }
                    else
                        break;
                }
                lua_pop(L, 1);
                
                // The match will have been replaced by string of possibly different size.
                // Calculate the previous end of match, and apply the delta to it for the next start position.
                int replaceDelta = static_cast<int>(subjectMutable.size()) - subjectSizeBefore;
                startPosition = matches[0].offset + matches[0].length + replaceDelta;
            }

            lua_pushlstring(L, subjectMutable.c_str(), subjectMutable.size());
            lua_pushnumber(L, replaced);
            rv = 2;
        }
        else if (replaceType == LUA_TFUNCTION)
        {
            Poco::RegularExpression::MatchVec matches;
            int matchCount = 0;
            while ((matchCount = reud->mRegularExpression.match(subjectMutable, startPosition, matches, options)) > 0)
            {
                // if we were supplied a replaceCount (-1 == replace all), 
                // break if desired number of replacements have been made.
                if (replaceCount > -1 && 0 >= replaceCount)
                    break;

                int subjectSizeBefore = static_cast<int>(subjectMutable.size());
                
                // use the first capture (1) if captures are present, otherwise use the whole match (0).
                int matchIdx = matchCount > 1 ? 1 : 0;
                
                // push captures or entire match and pcall function.
                lua_pushvalue(L, 3);
                for (size_t i = matchIdx; i < matchCount; ++i)
                {
                    lua_pushlstring(L, subjectMutable.c_str() + matches[i].offset, matches[i].length);
                }
                int rv = lua_pcall(L, matchCount - matchIdx, 1, 0);
                
                // use function's string return value as the replacement.
                if (!lua_isnil(L, -1) && lua_isstring(L, -1))
                {
                    std::string replacement(lua_tostring(L, -1));
                    // perform the replacement
                    if (reud->mRegularExpression.subst(subjectMutable, startPosition, replacement, options) > 0)
                    {
                        ++replaced;
                        --replaceCount;
                    }
                    else
                        break;
                }
                else
                {
                    lua_pushnil(L);
                    lua_pushvalue(L, -2);
                    return 2;
                }
                lua_pop(L, 1);
                
                // The match will have been replaced by string of possibly different size.
                // Calculate the previous end of match, and apply the delta to it for the next start position.
                int replaceDelta = static_cast<int>(subjectMutable.size()) - subjectSizeBefore;
                startPosition = matches[matchIdx].offset + matches[matchIdx].length + replaceDelta;
            }

            lua_pushlstring(L, subjectMutable.c_str(), subjectMutable.size());
            lua_pushnumber(L, replaced);
            rv = 2;
        }
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
