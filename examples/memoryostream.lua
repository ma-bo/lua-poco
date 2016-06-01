--[[ memoryostream.lua
    This example shows how a memoryostream can be used as an endpoint/sink for 
    other filtering ostreams.  In this instance, it will be used to hold the 
    output of the deflatingostream in memory. The buffer could then later be used
    to send the compressed data elsewhere.
--]]

-- require modules needed for this task.
local buffer = require("poco.buffer")
local memoryostream = require("poco.memoryostream")
local deflatingostream = require("poco.deflatingostream")

-- read all example data from the example file.
-- note that this data could have come from a socket, file, or some other API,
-- provided that the data is returned as a string, it can be used with memoryistream.
local data = assert(io.open("example_data.txt", "rb")):read("*a")

-- allocate a buffer a bit larger than the input data in the case of 0% compression + gzip header.
local deflate_buffer = buffer(#data * 2)

-- note that the memoryostream holds a reference to the buffer, and the 
-- inflatingistream holds a reference to the memoryistream, so the programmer
-- does not need to keep the linked userdata in variables to prevent garbage collection.
local mos = memoryostream(deflate_buffer)
local os = assert(deflatingostream(mos,"STREAM_GZIP"))

-- writing all the data to the ostream will send it through the links resulting in compression
-- and outputting it to the poco buffer.  
-- note that the data could be sent directly to a file instead, however, using the buffer highlights 
-- the ability to then send the data to miscellaneous output mechanisms afterwards.
assert(os:write(data))
-- flush the data to the output stream by calling close().
os:close()

local edd_file = assert(io.open("example_data_deflated.txt.gz", "wb"))
assert(edd_file:write(deflate_buffer:data():sub(1, mos:bytesWritten())))
edd_file:close()
io.write("wrote ", mos:bytesWritten(), " bytes to example_data_deflated.txt.gz.\n")


