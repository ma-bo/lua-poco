cmake_minimum_required (VERSION 2.6)
project(luapoco)
include(ExternalProject)

### Pick Lua implementation to use.
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

message(STATUS "lua-poco using Lua headers: ${LUA_INCLUDE_DIR}/${LUA_INCLUDE_PATH}")
message(STATUS "lua-poco using Lua library: ${LUA_LIB}")

### lua-poco on Windows will use a static/interal copy of the Poco C++ library.
### It will also use a statically linked C/C++ runtime to avoid the redistributable mess.
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(PocoInternalSuffix "d")
else()
    set(PocoInternalSuffix )
endif()

if (MSVC)
    set(CompilerFlags
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        )
    foreach(CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()

    set(PocoInternalSuffix "mt${PocoInternalSuffix}")
elseif(UNIX)
    set(CMAKE_C_FLAGS "$CMAKE_C_FLAGS -fPIC")
    set(CMAKE_CXX_FLAGS "$CMAKE_CXX_FLAGS -fPIC")
endif()

### Setup internal Poco C++ library.
set(POCO_DIR ${CMAKE_CURRENT_BINARY_DIR}/poco_internal)
set(POCO_INSTALL_DIR ${POCO_DIR}/install)

ExternalProject_Add(poco_static 
    PREFIX ${POCO_DIR}
    URL http://pocoproject.org/releases/poco-1.7.3/poco-1.7.3-all.tar.gz
#    URL file://c:/users/matt/desktop/poco-1.7.3-all.tar.gz
    INSTALL_DIR ${POCO_INSTALL_DIR}
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DPOCO_STATIC=ON -DPOCO_MT=ON -DCMAKE_INSTALL_PREFIX=${POCO_INSTALL_DIR} -DENABLE_MONGODB=OFF -DENABLE_PAGECOMPILER=OFF -DENABLE_PAGECOMPILER_FILE2PAGE=OFF
)

include_directories(${POCO_INSTALL_DIR}/include)

add_subdirectory(src)
