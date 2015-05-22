/// Regular expressions
// Regular expressions implemented by the PCRE library.
// @module regex

#include "RegularExpression.h"
#include "Poco/Exception.h"

int luaopen_poco_regex(lua_State* L)
{
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
    struct UserdataMethod methods[] = 
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
// @string[opt] options 
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
// @string[opt] regex options for the match.
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
        startPosition = luaL_checkint(L, 4) - 1;
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

// @string s string to search using the regex.
// @string[opt] regex options for the match.
// @int[opt] init position in the string to start the find.
// @return match strings including captures or nil.
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
        startPosition = luaL_checkint(L, 4) - 1;
        startPosition = startPosition < 0 ? 0 : startPosition;
    }
    
    try
    {
        Poco::RegularExpression::MatchVec matches;
        int matchCount = reud->mRegularExpression.match(subject, startPosition, matches, options);
        lua_pushnumber(L, matchCount);
        for (size_t i = 0; matchCount > 0 && i < matches.size(); ++i)
        {
            lua_pushlstring(L, subject + matches[i].offset, matches[i].length);
            // overwrite values from 1 to matchCount
            lua_rawseti(L, 3, i + 1);
        }
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
            for (size_t i = 0; matchCount > 0 && i < matches.size(); ++i)
            {
                lua_pushlstring(L, subject + matches[i].offset, matches[i].length);
            }
            rv = matchCount;
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
// @string[opt] regex options for the match.
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
// @string s subject string to match against.
// @string string that will be used to replace matches in subject.
// Captures may be referenced in the replacement by using:
//  $0 (entire match), $1 (first capture), $2 (second capture), etc.
// @string[opt] options regex options for the match.
// Note: The RE_GLOBAL flag is always set for gsub.
// @int[opt] startPos starting position for the match.
// @return the newly replaced string or nil.
// @function gsub
int RegularExpressionUserdata::gsub(lua_State* L)
{
    int rv = 0;
    RegularExpressionUserdata* reud = checkPrivateUserdata<RegularExpressionUserdata>(L, 1);
    
    size_t subjectSize = 0;
    const char* subject = luaL_checklstring(L, 2, &subjectSize);
    const char* replacement = luaL_checkstring(L, 3);
    
    int options = Poco::RegularExpression::RE_GLOBAL;
    int startPosition = 0;
    
    int top = lua_gettop(L);
    if (top > 3)
    {
        const char* optionsStr = luaL_checkstring(L, 4);
        options = parseRegexOptions(optionsStr);
    }
    
    if (top > 4)
    {
        startPosition = luaL_checkint(L, 5) - 1;
        startPosition = startPosition < 0 ? 0 : startPosition;
    }
    
    try
    {
        std::string subjectMutable(subject);
        int replacedCount = reud->mRegularExpression.subst(subjectMutable, startPosition, replacement, options);
        lua_pushnumber(L, replacedCount);
        lua_pushlstring(L, subjectMutable.c_str(), subjectMutable.size());
        rv = 2;
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
