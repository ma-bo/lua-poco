-- load the environment module from the poco library.
local environment = require("poco.environment")

-- print out environment attributes
print("\npoco.environment functions:\n")
print("nodeId:        \t", environment.nodeId())
print("nodeName:      \t", environment.nodeName())
print("osArchitecture:\t", environment.osArchitecture())
print("osName:        \t", environment.osName())
print("osVerison:     \t", environment.osVersion())

print("\nchecking environment variables...\n")
print("has TEMP:      \t", environment.has("TEMP"))
if (environment.has("TEMP")) then
    print(environment.get("TEMP"))
end

print("has HOME:      \t", environment.has("HOME"))
if (environment.has("HOME")) then
    print(environment.get("HOME"))
end

print("has USERPROFILE:", environment.has("USERPROFILE"))
if (environment.has("USERPROFILE")) then
    print(environment.get("USERPROFILE"))
end
