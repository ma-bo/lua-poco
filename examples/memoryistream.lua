--[[ memoryistream.lua
    This example shows how a memoryistream can be used with data from an arbitrary
    source like a file, socket, http request, or other mechanism to create an 
    istream from a buffer.
    
    The example uses the memoryistream as a data source for the inflatingistream
    filter to read compressed data and inflate it.
    
    The compressed file contains lines in the format:
    1 Hello World!\n
    2 Hello World!\n
--]]

-- require modules needed for this task.
local buffer = require("poco.buffer")
local memoryistream = require("poco.memoryistream")
local inflatingistream = require("poco.inflatingistream")

-- read all compressed data from the example file.
-- note that this data could have come from a socket, file, or some other API,
-- provided that the data is returned as a string, it can be used with memoryistream.
local data = assert(io.open("example_data.txt.gz", "rb")):read("*a")

-- store the data from the file in a buffer, and use the memoryistream as the source
-- istream for the inflatingistream filter in order to decompress the data that is read through it.

-- note that the memoryistream holds a reference to the buffer, and the 
-- inflatingistream holds a reference to the memoryistream, so the programmer
-- does not need to keep the userdata in variables to prevent garbage collection.
local is = assert(inflatingistream(memoryistream(buffer(data)),"STREAM_GZIP"))

local hello_world_count = 0
for line in is:lines() do
    if line:find("%d+ Hello World!") then
        hello_world_count = hello_world_count + 1
    end
end

io.write("read ", tostring(hello_world_count), " Hello World!'s from example_data.txt.gz.\n")
