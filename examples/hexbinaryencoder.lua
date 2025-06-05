--[[ hexbinaryencoder.lua
    This example shows how the hexbinaryencoder module is used as an ostream to output hexbinary data.
--]]

-- prerequisite modules
local path = require("poco.path")
local hexbinaryencoder = require("poco.hexbinaryencoder")
local fileistream = require("poco.fileistream")
local fileostream = require("poco.fileostream")
local streamcopier = require("poco.streamcopier")

-- filesystem locations
local example_file_name = "example_data.txt.gz"
local output_file_name = "example_data.hexbinary.txt"
local example_file_path = string.format("%s%s", path.current(), example_file_name)
local output_file_path = string.format("%s%s", path.temp(), output_file_name)

-- instances
local input_data_is = assert(fileistream(example_file_path))
local output_data_os = assert(fileostream(output_file_path))
local hexbinary_filter_os = assert(hexbinaryencoder(output_data_os))

-- use streamcopier to send all of example_data.txt.gz through the base32 encoder.
local bytes_copied = assert(streamcopier.copyStream(input_data_is, hexbinary_filter_os))
output_data_os:close()

print("input file: ", example_file_path)
print("output file: ", output_file_path)
print(string.format("wrote %d bytes from source to output stream.", bytes_copied))



