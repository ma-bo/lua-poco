--[[ compress.lua
    This example shows how a the compress module can be used to create zip files and write them to
    a compatible ostream.  Files and directories can be added via istreams and filesystem paths.
--]]

local decompress = assert(require("poco.zip.decompress"))
local fileistream = assert(require("poco.fileistream"))
local path = assert(require("poco.path"))

-- re-use the zip file from the compress.lua example.
-- note: the istream does not have to be a file, it could be pipeistream, socketistream, memoryistream, etc.
local input_zip_path = path(path.temp())
input_zip_path:setFileName("compress_example.zip")
local zfs = assert(fileistream(input_zip_path:toString()))

-- Construct decompress instance.  There are two additional optional default parameters available: (by default false)
--- flattenDirs = true, which does not recreate the embedded directory structure.
--- keepIncompleteFiles = true, which leaves incomplete files on the filesystem.
local output_path = path(path.temp())
output_path:pushDirectory("decompress-example")
local dc = assert(decompress(zfs, output_path))

-- decompress all entries in the zip file.
-- the function returns (true, "success", good count, fail count) even when there are problems extractinig some files.
-- if a critical error occurs other than failure to write an entry to the filesystem (nil, "msg") will be returned.
print(string.format("Decompressing %s to %s\n", input_zip_path:toString(), output_path:toString()))
local rv, msg, good_count, fail_count = dc:decompressAll()
if rv then
    print("entries decompressed: ", good_count)
    print("entries failed: ", fail_count)
else
    print("error when decompressing files: ", msg)    
end

-- iterate ZipLocalFileHeader's and print the absolute path to their location along with some other attributes.
for path, zflh in dc:decompressed() do
    path:makeAbsolute()
    print("file name:        ", zflh.fileName)
    print("full path:        ", path:toString())
    print("uncompressedSize: ", zflh.uncompressedSize)
    print("method:           ", zflh.method)
    print("level:            ", zflh.level)
    print("headerstartPos:   ", zflh.startPos)
    print("dataStartPos:     ", zflh.dataStartPos)
end

