--[[ base64encoder.lua
    This example shows how base64encoder module is used as an ostream to output base64 encoded data.
--]]

-- prerequisite modules
local path = require("poco.path")
local base64encoder = require("poco.base64encoder")
local fileistream = require("poco.fileistream")
local fileostream = require("poco.fileostream")
local streamcopier = require("poco.streamcopier")

-- filesystem locations
local example_file_name = "example_data.txt.gz"
local output_file_name = "example_data.base64.txt"
local example_file_path = string.format("%s%s", path.current(), example_file_name)
local output_file_path = string.format("%s%s", path.temp(), output_file_name)

-- instances
local input_data_is = assert(fileistream(example_file_path))
local output_data_os = assert(fileostream(output_file_path))
local base64_filter_os = assert(base64encoder(output_data_os))

-- use streamcopier to send all of example_data.txt.gz through the base64 encoder.
local bytes_copied = assert(streamcopier.copyStream(input_data_is, base64_filter_os))

-- flush data to output file
base64_filter_os:close()
print("input file: ", example_file_path)
print("output file: ", output_file_path)
print(string.format("wrote %d bytes from source to output stream.", bytes_copied))



