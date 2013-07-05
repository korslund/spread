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

add_executable(manager_test manager_test.cpp ${MANGLE})
target_link_libraries(manager_test ${LIBS})

add_executable(file_test file_test.cpp ${MANGLE})
target_link_libraries(file_test ${LIBS})

add_executable(unpack_test unpack_test.cpp ${MANGLE})
target_link_libraries(unpack_test ${LIBS})

add_executable(diff_test diff_test.cpp ${MANGLE})
target_link_libraries(diff_test ${LIBS})

add_executable(blind_test blind_test.cpp ${MANGLE})
target_link_libraries(blind_test ${LIBS})
