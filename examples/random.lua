--[[ random.lua
    This example shows how the random pseudo random number generator module is used.
--]]
local random = require("poco.random")

-- explicit construction using state_size, and a seed value
-- local prng = assert(random(256, os.time()))

-- explicit construction using just state_size and default internal Poco RandomInputStream seed.
-- local prng = assert(random(256))

-- construction using default state_size of 256 and internal Poco RandomInputStream seed.
local prng = assert(random())


-- generate some random numbers.
for i = 1, 15 do
    print("\ninteger: ", prng:next())
    print("modulo 100: ", prng:next(100))
    print("number: ", prng:nextNumber())
    print("byte: ", prng:nextByte())
    print("bool: ", prng:nextBool())
end
