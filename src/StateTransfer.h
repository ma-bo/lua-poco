#ifndef LUA_POCO_STATETRANSFER_H
#define LUA_POCO_STATETRANSFER_H

#include "LuaPoco.h"
#include "Poco/Buffer.h"

namespace LuaPoco
{

class TransferBuffer : public Poco::Buffer<char>
{
public:
	TransferBuffer();
	TransferBuffer(size_t startSize);
	~TransferBuffer();
	void insert(const char* data, size_t amount);
	size_t contentSize();
private:
	TransferBuffer(const TransferBuffer&);
	TransferBuffer& operator= (const TransferBuffer&);
	size_t mWriteIdx;
};

int functionWriter(lua_State* L, const void* p, size_t sz, void* ud);
const char* functionReader(lua_State* L, void* data, size_t* size);
bool transferFunction(lua_State* toL, lua_State* fromL);
bool transferValue(lua_State* toL, lua_State* fromL);

} // LuaPoco


#endif
