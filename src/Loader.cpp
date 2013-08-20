#include "LuaPoco.h"
#include "foundation/File.h"
#include "foundation/RegularExpression.h"
#include "foundation/DynamicAny.h"
#include "foundation/Timestamp.h"
#include "foundation/Checksum.h"

extern "C" 
{

using namespace LuaPoco;

// create all metatables per class
// and load class constructors into the poco table.
int luaopen_poco(lua_State* L)
{
	int rv = 0;
	// table for main poco module
	// all sub modules will load into this table
	// corresponding to the Poco namespaces for each class.
	lua_createtable(L, 0, 5);
	
	// Foundation classes go into the root of the table.
	if (
		FileUserdata::registerFile(L) &&
		RegularExpressionUserdata::registerRegularExpression(L) &&
		DynamicAnyUserdata::registerDynamicAny(L) &&
		TimestampUserdata::registerTimestamp(L) &&
		ChecksumUserdata::registerChecksum(L)
	)
	{
		rv = 1;
	}
	
	
	return rv;
}

}
