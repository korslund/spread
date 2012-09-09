cmake_minimum_required(VERSION 2.6)

include_directories("../")
include_directories("../../")
include_directories("../../libs/")
include_directories("../../libs/jsoncpp/include/")
include_directories(${Boost_INCLUDE_DIRS})

find_package(Boost COMPONENTS filesystem system REQUIRED)

add_subdirectory("../../" "${CMAKE_CURRENT_BINARY_DIR}/_spread")

set(LIBS Spread ${Boost_LIBRARIES} zzip ${CURL_LIBRARIES})

set(MDIR ../../libs/mangle)
set(MANGLE ${MDIR}/stream/clients/io_stream.cpp)

add_executable(dummy1_test dummy1_test.cpp ${MANGLE})
target_link_libraries(dummy1_test ${LIBS})

add_executable(zip_test zip_test.cpp ${MANGLE})
target_link_libraries(zip_test ${LIBS})

add_executable(auto_test auto_test.cpp ${MANGLE})
target_link_libraries(auto_test ${LIBS})
