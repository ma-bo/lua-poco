set(FOUNDATION_SRC
	Loader.cpp
	File.cpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
add_library(foundation SHARED ${FOUNDATION_SRC})
target_link_libraries(foundation luapoco-core ${LIBLUA51} ${LIBPocoFoundation})
