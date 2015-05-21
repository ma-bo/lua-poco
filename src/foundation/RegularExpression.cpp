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
        { "extract", extract },
        { "oldmatch", oldmatch },
        { "extractPositions", extractPositions },
        { "extractCaptures", extractCaptures },
        { "substitute", substitute },
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
    
    const char* pattern = luaL_checkstring(L, 1);
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

/// Extracts the primary regex match.
// Subgroups are returned in the match count, but only the primary match is extracted.
// @string subject the string which will be processed with the regex.
// @string[opt] options regex options for the match.
// @int[opt] startPos the position to start the match.
// @return the count of matches as a number, or nil. (error)
// @return the substring matched by the expression, or error message.
// @function extract
int RegularExpressionUserdata::extract(lua_State* L)
{
    int rv = 0;
    RegularExpressionUserdata* reud = checkPrivateUserdata<RegularExpressionUserdata>(L, 1);
    
    const char* subject = luaL_checkstring(L, 2);
    int options = 0;
    int startPosition = 0;
    
    int top = lua_gettop(L);
    if (top > 3)
    {
        const char* optionsStr = luaL_checkstring(L, 3);
        options = parseRegexOptions(optionsStr);
    }
    if (top > 4)
    {
        startPosition = luaL_checkint(L, 4) - 1;
        startPosition = startPosition < 0 ? 0 : startPosition;
    }
    
    try
    {
        std::string match;
        int matchCount = reud->mRegularExpression.extract(subject, startPosition, match, options);
        lua_pushnumber(L, matchCount);
        lua_pushlstring(L, match.c_str(), match.size());
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

/// Matches pattern against string.
// Returns match position and length.
// @string subject attempt to match the pattern against subject string.
// @string[opt] regex options for the match.
// @int[opt] startPos starting position for the match.
// @return the starting position of the match as a number or nil. (error)
// @return the length of the match or error message.
// @function match
int RegularExpressionUserdata::oldmatch(lua_State* L)
{
    int rv = 0;
    RegularExpressionUserdata* reud = checkPrivateUserdata<RegularExpressionUserdata>(L, 1);
    
    const char* subject = luaL_checkstring(L, 2);
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
        Poco::RegularExpression::Match match;
        int matchCount = reud->mRegularExpression.match(subject, startPosition, match, options);
        lua_pushnumber(L, matchCount);
        if (matchCount > 0)
        {
            // return a 1-based start position and end position, 
            // instead of offset + length
            lua_pushnumber(L, match.offset + 1);
            lua_pushnumber(L, match.offset + match.length);
        }
        else
        {
            lua_pushnil(L);
            lua_pushnil(L);
        }
        rv = 3;
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

/// Replace match in string with a substitute.
// @string subject attempt to match the pattern against subject string.
// @string replacement string that will be used to replace the matched pattern.
// @string[opt] options regex options for the match.
// @int[opt] startPos starting position for the match.
// @return the count of matches replaced or nil. (error)
// @return the newly replaced string or error message.
// @function substitute
int RegularExpressionUserdata::substitute(lua_State* L)
{
    int rv = 0;
    RegularExpressionUserdata* reud = checkPrivateUserdata<RegularExpressionUserdata>(L, 1);
    
    size_t subjectSize = 0;
    const char* subject = luaL_checklstring(L, 2, &subjectSize);
    const char* replacement = luaL_checkstring(L, 3);
    
    int options = 0;
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

/// Extracts all matches into supplied table.
//
// Note: matches are written to the table starting at index 1.
// It is up to the user to clear the table before calling this function, or 
// observe the return value when iterating the table.
// @string subject attempt to match the pattern against subject string.
// @param extractTable a table where matches will be stored starting at index 1.
// @string[opt] regex options for the match.
// @int[opt] startPos starting position for the match.
// @return the count of matches replaced or nil. (error)
// @return error message.
// @function extractCaptures
int RegularExpressionUserdata::extractCaptures(lua_State* L)
{
    int rv = 0;
    RegularExpressionUserdata* reud = checkPrivateUserdata<RegularExpressionUserdata>(L, 1);
    
    size_t subjectSize = 0;
    const char* subject = luaL_checklstring(L, 2, &subjectSize);
    
    int options = 0;
    int startPosition = 0;
    
    luaL_checktype(L, 3, LUA_TTABLE);
    
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

/// Extracts all match positions into the supplied table.
//
// Note: positions are written to the table starting at index 1.
// It is up to the user to clear the table before calling this function, or 
// observe the return value when iterating the table.
// @string subject attempt to match the pattern against subject string.
// @param extractTable a table where matches will be stored starting at index 1.
// @string[opt] regex options for the match.
// @int[opt] startPos starting position for the match.
// @return the count of matches replaced or nil. (error)
// @return error message.
// @function extractPositions
int RegularExpressionUserdata::extractPositions(lua_State* L)
{
    int rv = 0;
    RegularExpressionUserdata* reud = checkPrivateUserdata<RegularExpressionUserdata>(L, 1);
    
    size_t subjectSize = 0;
    const char* subject = luaL_checklstring(L, 2, &subjectSize);
    
    int options = 0;
    int startPosition = 0;
    
    luaL_checktype(L, 3, LUA_TTABLE);
    
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
        Poco::RegularExpression::MatchVec matches;
        int matchCount = reud->mRegularExpression.match(subject, startPosition, matches, options);
        lua_pushnumber(L, matchCount);
        for (size_t i = 0; matchCount > 0 && i < matches.size(); ++i)
        {
            // overwrite values from 1 to matchCount
            lua_pushnumber(L, matches[i].offset + 1);
            lua_rawseti(L, 3, i + 1);
            lua_pushnumber(L, matches[i].offset + matches[i].length);
            lua_rawseti(L, 3, i + 2);
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

int RegularExpressionUserdata::find(lua_State* L)
{
    return 0;
}

int RegularExpressionUserdata::match(lua_State* L)
{
    return 0;
}

int RegularExpressionUserdata::gmatch(lua_State* L)
{
    return 0;
}

int RegularExpressionUserdata::gsub(lua_State* L)
{
    return 0;
}

} // LuaPoco
