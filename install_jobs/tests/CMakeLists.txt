cmake_minimum_required(VERSION 2.6)

find_package(Boost COMPONENTS filesystem system thread REQUIRED)
find_package(CURL REQUIRED)
find_package(ZLIB REQUIRED)

include_directories("../")
include_directories("../../")
include_directories("../../libs/")
include_directories("../../libs/jsoncpp/include/")
include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory("../../" "${CMAKE_CURRENT_BINARY_DIR}/_spread")
add_subdirectory("../../libs/unpackcpp/" "${CMAKE_CURRENT_BINARY_DIR}/_unpackcpp")

set(LIBS Spread UnpackCpp ${Boost_LIBRARIES} ${ZLIB_LIBRARIES} ${CURL_LIBRARIES})

set(MANGLE ../../libs/mangle/stream/clients/io_stream.cpp)

add_executable(base1_test base1_test.cpp ${MANGLE})
target_link_libraries(base1_test ${LIBS})

add_executable(leaf1_test leaf1_test.cpp ${LEAF})
target_link_libraries(leaf1_test ${LIBS})

add_executable(deadlock_test deadlock_test.cpp ${MANGLE})
target_link_libraries(deadlock_test ${LIBS})

add_executable(hashfind_test hashfind_test.cpp ${MANGLE})
target_link_libraries(hashfind_test ${LIBS})
