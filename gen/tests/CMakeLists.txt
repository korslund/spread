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

set(MDIR ../../libs/mangle)
set(MANGLE ${MDIR}/stream/clients/io_stream.cpp)

add_executable(lister_test lister_test.cpp ${MANGLE})
target_link_libraries(lister_test SpreadGen ${LIBS})

add_executable(writer_test writer_test.cpp ${MANGLE})
target_link_libraries(writer_test SpreadGen ${LIBS})

add_executable(sr0_test sr0_test.cpp ${MANGLE})
target_link_libraries(sr0_test SpreadGen ${LIBS})
