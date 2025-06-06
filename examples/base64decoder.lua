--[[ base64decoder.lua
    This example shows how base64decoder module is used as an ostream to output decoded data.
--]]

-- prerequisite modules
local path = require("poco.path")
local base64decoder = require("poco.base64decoder")
local fileistream = require("poco.fileistream")
local fileostream = require("poco.fileostream")
local streamcopier = require("poco.streamcopier")
local temporaryfile = require("poco.temporaryfile")

-- filesystem locations
-- re-use output file from base64encoder.lua
local example_file_name = "example_data.base64.txt"
local example_file_path = string.format("%s%s", path.temp(), example_file_name)

-- instances
local output_tempfile = assert(temporaryfile())
local output_file_path = output_tempfile:path()
local input_data_is = assert(fileistream(example_file_path))
local output_data_os = assert(fileostream(output_file_path))
local base64_filter_is = assert(base64decoder(input_data_is))
-- use streamcopier to send read all base64 data through the filter, and send it to the output stream.
local bytes_copied = assert(streamcopier.copyStream(base64_filter_is, output_data_os))
output_data_os:close()

print("input file: ", example_file_path)
print("output file: ", output_file_path)
print(string.format("wrote %d bytes from source to output stream.", bytes_copied))

