-- load the poco.timestamp module.
local timestamp = assert(require("poco.timestamp"))
-- load the poco.dynamic any module, as dynamicany userdata are used to store
-- 64-bit timestamp values.
local dynamicany = assert(require("poco.dynamicany"))

local ts1 = timestamp()
print("Microseconds since the Unix epoch, midnight, January 1, 1970:")
print("\t\t", ts1:epochMicroseconds():toString())
print("Local Date and Time:")
print("\t\t", os.date("%c", ts1:epochTime()))

local elapsed
local zero_delta = dynamicany(0)
-- count how many loop iterations pass before a change is noticed.
-- note: on Windows, the elapsed value will be around 15000 due to OS scheduling.
-- on linux and else where, there should only be a single loop pass with some
-- small number of microseconds elapsed.
local counter = 0
local ts3 = timestamp()
repeat
    elapsed = ts3:elapsed()
    counter = counter + 1
until elapsed > zero_delta

print("\nMicroseconds taken to notice a timestamp delta:")
print("\t\t", elapsed:toString())
print("Loop iterations to notice a timestamp delta:")
print("\t\t", counter)
