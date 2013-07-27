set(FOUNDATION_SRC
	Loader.cpp
	File.cpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
add_library(foundation SHARED ${FOUNDATION_SRC})
set_target_properties(foundation PROPERTIES PREFIX "")
target_link_libraries(foundation poco-common ${LIBLUA51} ${LIBPocoFoundation})

install(TARGETS foundation LIBRARY DESTINATION ${CMAKE_BINARY_DIR}/poco)
