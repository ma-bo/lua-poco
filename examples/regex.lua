--[[ regex.lua
    This example shows how the regex module functions in a very similar fashion 
    when compared to Lua patterns.
--]]

local regex = require("poco.regex")

local url_test_port = "ftp://host.example.org:21/example/directory"
local url_test_noport = "ftp://host.example.org/example/directory"

-- using the [[string literal]] format for your regex enables you to avoid
-- escaping backslashes.
local url_pattern = [[(https?|ftp|smtp)://([^:/]+)(:\d+)?(/.*)]]
local url_regex = regex(url_pattern)
print("\nurl_pattern: ", url_pattern)

-- use find to determine if pattern matches
if url_regex:find(url_test_port) then
     print(string.format("\nurl_regex:find(\"%s\"): %s", url_test_port, tostring(true)))
end

if url_regex:find(url_test_noport) then
    print(string.format("\nurl_regex:find(\"%s\"): %s", url_test_noport, tostring(true)))
end

-- use match to extract matches.
local url_prefix, url_host, url_port, url_path = url_regex:match(url_test_port)
if url_prefix then
    print(string.format("\nurl_regex:match(\"%s\"):", url_test_port))
    print("proto: ", url_prefix)
    print("host: ", url_host)
    print("port: ", url_port)
    print("path: ", url_path)
end

-- use gmatch to iterate through matches in a string
local multi_urls = 
[[http://www.example.org:80/some/path
http://www.example.org/some/other/path
https://www.example.org:443/some/ssl/path]]

print(string.format("\nmulti_urls:\n%s\n", multi_urls))
local gmatch_count = 0
print("\nurl_regex:gmatch(multi_urls)")
for url_prefix, url_host, url_port, url_path in url_regex:gmatch(multi_urls) do
    gmatch_count = gmatch_count + 1
    print("match:", gmatch_count)
    print("proto: ", url_prefix)
    print("host: ", url_host)
    print("port: ", url_port)
    print("path: ", url_path)
end

-- use gsub to convert certain types of urls listed in the table to another.
local imap_test = "imap://mail.example.org/something"
local replacements = { ["imap"] = "pop3", ["ftp"] = "http" }
local proto_regex = regex([[^(\w+)(?=://)]])
print(string.format("\nproto_regex:gsub(\"%s\"):\n%s.", url_test_noport, proto_regex:gsub(url_test_noport, replacements)))
print(string.format("\nproto_regex:gsub(\"%s\"):\n%s.", imap_test, proto_regex:gsub(imap_test, replacements)))
