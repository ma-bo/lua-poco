cmake_minimum_required (VERSION 2.6)
project(luapoco)

if (LUA_INCLUDE AND LUA_LIB_PATH AND LUA_LIB_NAME)
    find_library(LUA_LIB ${LUA_LIB_NAME} PATHS ${LUA_LIB_PATH} NO_CMAKE_PATH NO_CMAKE_ENVIRONMENT_PATH NO_CMAKE_SYSTEM_PATH)
    include_directories(${LUA_INCLUDE})
    set(LUA_INCLUDE_DIR ${LUA_INCLUDE})
else()
    if(LUA51)
        set(LUA_INCLUDE_PATH lua5.1)
        set(LUA_LIB_NAME lua5.1)
    elseif(LUAJIT)
        set(LUA_INCLUDE_PATH luajit-2.0)
        set(LUA_LIB_NAME luajit-5.1)
    else() # default to whatever is installed as lua.h/lua.so
        set(LUA_INCLUDE_PATH "")
        set(LUA_LIB_NAME lua)
    endif()
    
    find_path(LUA_INCLUDE_DIR ${LUA_INCLUDE_PATH}/lua.h)
    include_directories("${LUA_INCLUDE_DIR}/${LUA_INCLUDE_PATH}")
    find_library(LUA_LIB "${LUA_LIB_NAME}")
endif()

message(STATUS "LUA-POCO using Lua headers: ${LUA_INCLUDE_DIR}/${LUA_INCLUDE_PATH}")
message(STATUS "LUA-POCO using Lua library: ${LUA_LIB}")

# include POCO library
find_path(POCO_INCLUDE_DIR Poco/File.h)
include_directories(${POCO_INCLUDE_DIR})

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
