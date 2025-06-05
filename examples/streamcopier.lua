--[[ streamcopier.lua
    This example shows how to use the streamcopier module to copy all data from an istream to an
    ostream.
--]]

-- require modules needed for this task.
local fileostream = require("poco.fileostream")
local fileistream = require("poco.fileistream")
local deflatingostream = require("poco.deflatingostream")
local streamcopier = require("poco.streamcopier")

-- construct a source istream, destination ostream, and deflating ostream filter to be used in
-- front of the destination.
local filename = "example_data.txt"
local fis = assert(fileistream(filename))
local dos = assert(deflatingostream(assert(fileostream(filename .. ".gz")),"STREAM_GZIP"))

-- copy all data from input stream, through output stream filter, and to the destination.
local bytes_copied = assert(streamcopier.copyStream(fis, dos))
-- close the filter to make sure all data is flushed to the destination.
dos:close()

print(string.format("%d bytes sent to %s", bytes_copied, filename .. ".gz"))


