-- load the poco.checksum module
local checksum = assert(require("poco.checksum"))

-- these are equivalent to the following:
-- local adler = assert(checksum.new("Adler-32"))
-- local crc = assert(checksum.new("CRC-32"))
local adler = assert(checksum("ADLER32"))
local crc = assert(checksum("CRC32"))

local hash_string = "hello world."
print(string.format("hash_string:\t'%s'", hash_string))
crc:update(hash_string)
adler:update(hash_string)
print("\tCRC-32:  ", string.format("0x%x", crc:checksum()))
print("\tAdler-32:", string.format("0x%x", adler:checksum()))

-- create new hashes with just "hello " and output.
hash_string = "hello "
print(string.format("\nhash_string:\t'%s'", hash_string))
adler = assert(checksum("ADLER32"))
crc = assert(checksum("CRC32"))
crc:update(hash_string)
adler:update(hash_string)
print("\tCRC-32:  ", string.format("0x%x", crc:checksum()))
print("\tAdler-32:", string.format("0x%x", adler:checksum()))

-- update the existing hash with "world." and output.
hash_string = "world."
print(string.format("\nupdating with:\t'%s'", hash_string))
crc:update(hash_string)
adler:update(hash_string)
print("\tCRC-32:  ", string.format("0x%x", crc:checksum()))
print("\tAdler-32:", string.format("0x%x", adler:checksum()))
