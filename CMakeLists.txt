cmake_minimum_required (VERSION 2.6)
project(luapoco)

# lua5.1 // hack for now
find_path(LUA_INCLUDE_DIR lua5.1/lua.h)
include_directories(${LUA_INCLUDE_DIR}/lua5.1)
find_library(LIBLUA51 lua5.1)

# include POCO library
find_path(POCO_INCLUDE_DIR Poco/File.h)
include_directories(${POCO_INCLUDE_DIR}/Poco)

find_library(LIBPocoFoundation PocoFoundation)
find_library(LIBPocoCrypto PocoCrypto)
find_library(LIBPocoData PocoData)
find_library(LIBPocoDataMySQL PocoDataMySQL)
find_library(LIBPocoDataSQLite PocoDataSQLite)
find_library(LIBPocoNet PocoNet)
find_library(LIBPocoNetSSL PocoNetSSL)
find_library(LIBPocoUtil PocoUtil)
find_library(LIBPocoXML PocoXML)
find_library(LIBPocoZip PocoZip)

add_subdirectory(src)
