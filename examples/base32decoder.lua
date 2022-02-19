--[[ base32decoder.lua
    This example shows how base32encoder module is used as an ostream to output decoded base32 data.
--]]

-- prerequisite modules
local path = assert(require("poco.path"))
local base32decoder = assert(require("poco.base32decoder"))
local fileistream = assert(require("poco.fileistream"))
local fileostream = assert(require("poco.fileostream"))
local streamcopier = assert(require("poco.streamcopier"))
local temporaryfile = assert(require("poco.temporaryfile"))

-- filesystem locations
-- re-use output file from base32encoder.lua
local example_file_name = "example_data.base32.txt"
local example_file_path = string.format("%s%s", path.temp(), example_file_name)

-- instances
local output_tempfile = assert(temporaryfile())
local output_file_path = output_tempfile:path()
local input_data_is = assert(fileistream(example_file_path))
local output_data_os = assert(fileostream(output_file_path))
local base32_filter_is = assert(base32decoder(input_data_is))
-- use streamcopier to send read all base32 data through the filter, and send it to the output stream.
local bytes_copied = assert(streamcopier.copyStream(base32_filter_is, output_data_os))
output_data_os:close()

print("input file: ", example_file_path)
print("output file: ", output_file_path)
print(string.format("wrote %d bytes from source to output stream.", bytes_copied))

