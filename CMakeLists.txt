cmake_minimum_required (VERSION 2.6)
project(luapoco)
include(ExternalProject)

### Pick Lua implementation to use.
if (LUA_INCLUDE AND LUA_LIB_PATH AND LUA_LIB_NAME)
    find_library(LUA_LIB ${LUA_LIB_NAME} PATHS ${LUA_LIB_PATH} NO_DEFAULT_PATH)
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
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

### Setup internal Poco C++ library.
#### POCO_URL can be defined at cmake invocation via a file:// URI to point at a local version of
#### the poco-x.x.x-all.tar.gz file.  ie: file://c:/users/username/src/poco-1.9.0-all.tar.gz
set(POCO_DIR ${CMAKE_CURRENT_BINARY_DIR}/poco_internal)
set(POCO_INSTALL_DIR ${POCO_DIR}/install)
if(NOT POCO_URL)
    set(POCO_URL http://pocoproject.org/releases/poco-1.9.0/poco-1.9.0-all.tar.gz)
    
endif()
ExternalProject_Add(poco_static 
    PREFIX ${POCO_DIR}
    URL ${POCO_URL}
    CMAKE_COMMAND "${CMAKE_COMMAND}" -E env "CXXFLAGS=${CMAKE_CXX_FLAGS}" "CFLAGS=${CMAKE_C_FLAGS}" "${CMAKE_COMMAND}"
    INSTALL_DIR ${POCO_INSTALL_DIR}
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} -DPOCO_STATIC=ON -DPOCO_MT=ON -DCMAKE_INSTALL_PREFIX=${POCO_INSTALL_DIR} -DENABLE_MONGODB=OFF -DENABLE_PAGECOMPILER=OFF -DENABLE_PAGECOMPILER_FILE2PAGE=OFF
)

include_directories(${POCO_INSTALL_DIR}/include)

add_subdirectory(src)
