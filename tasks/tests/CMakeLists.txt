cmake_minimum_required(VERSION 2.6)

include_directories("../")
include_directories("../../")
include_directories("../../libs/")
include_directories("../../libs/jsoncpp/include/")
include_directories(${Boost_INCLUDE_DIRS})

find_package(Boost COMPONENTS filesystem system thread REQUIRED)
find_package(CURL REQUIRED)
find_package(ZLIB REQUIRED)

add_subdirectory("../../" "${CMAKE_CURRENT_BINARY_DIR}/_spread")
add_subdirectory("../../libs/unpackcpp/" "${CMAKE_CURRENT_BINARY_DIR}/_unpackcpp")

set(LIBS Spread UnpackCpp ${Boost_LIBRARIES} ${ZLIB_LIBRARIES} ${CURL_LIBRARIES})

set(MDIR ../../libs/mangle)
set(CPP ${MDIR}/stream/clients/io_stream.cpp)

add_executable(dl_test dl_test.cpp ${CPP})
target_link_libraries(dl_test ${LIBS})

add_executable(zip_test zip_test.cpp ${CPP})
target_link_libraries(zip_test ${LIBS})
