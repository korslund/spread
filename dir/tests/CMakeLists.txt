cmake_minimum_required(VERSION 2.6)

include_directories("../")
include_directories("../../")
include_directories("../../libs/")
include_directories("../../libs/jsoncpp/include/")
include_directories(${Boost_INCLUDE_DIRS})

find_package(Boost COMPONENTS filesystem system REQUIRED)

add_subdirectory("../../" "${CMAKE_CURRENT_BINARY_DIR}/_spread")

set(LIBS ${Boost_LIBRARIES} zzip ${CURL_LIBRARIES})

set(MDIR ../../libs/mangle)
set(MANGLE ${MDIR}/stream/clients/io_stream.cpp)

add_executable(dir_test dir_test.cpp ${MANGLE})
target_link_libraries(dir_test Spread ${LIBS})

add_executable(fs_test fs_test.cpp ${MANGLE})
target_link_libraries(fs_test Spread ${LIBS})
