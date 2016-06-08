--[[ teeostream.lua
    This example shows how a teeostream can be used to split an output into one or more 
    ostream channels.
--]]

-- require modules needed for this task.
local teeostream = require("poco.teeostream")
local fileostream = require("poco.fileostream")

-- read all example data from the example file.
-- note that this data could have come from a socket, file, or some other API,
-- provided that the data is returned as a string, it can be used with memoryistream.
local data = assert(io.open("example_data.txt", "rb")):read("*a")

-- adding several ostreams to a single teeostream, such that all writes go to all ostreams.
local tos = teeostream(assert(fileostream("file1.txt")), assert(fileostream("file2.txt")))
assert(tos:write("Hello World from teeostream.lua."))
assert(tos:flush())
print("done writing to teeostream, file1.txt and file2.txt written.")
