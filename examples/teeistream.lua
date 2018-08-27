--[[ teeistream.lua
    This example shows how a teeistream can be used to split an all data from an input stream
     into one or more ostream channels.
--]]

-- require modules needed for this task.
local buffer = require("poco.buffer")
local memoryistream = require("poco.memoryistream")
local teeistream = require("poco.teeistream")
local fileostream = require("poco.fileostream")

-- note that this data could have come from any type of istream, memory, socket, file, or some other API.
local mis = assert(memoryistream(buffer("Some data coming from an input stream.\nSome other data.")))

-- adding several ostreams to a single teeistream, such that all read data gets copied to all ostreams.
local tis = assert(teeistream(mis))
-- these output streams could be a deflating stream, encrypting stream, pipe, socket, etc.
tis:addStream(fileostream("file1.txt"), fileostream("file2.txt"))

print(tis:read("*a"))
print("\ndone writing from teeistream, file1.txt and file2.txt written.")
