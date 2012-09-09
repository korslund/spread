cmake_minimum_required(VERSION 2.6)

include_directories("../")
include_directories("../../")
include_directories("../../libs/")
include_directories("../../libs/jsoncpp/include/")
include_directories(${Boost_INCLUDE_DIRS})

find_package(Boost COMPONENTS filesystem system thread REQUIRED)
find_package(CURL REQUIRED)

add_subdirectory("../../" "${CMAKE_CURRENT_BINARY_DIR}/_spread")

set(LIBS Spread ${Boost_LIBRARIES} zzip ${CURL_LIBRARIES})

set(MDIR ../../libs/mangle)
set(CPP ${MDIR}/stream/clients/io_stream.cpp)

add_executable(inst1_test inst1_test.cpp ${CPP})
target_link_libraries(inst1_test ${LIBS})

add_executable(build_test build_test.cpp ${CPP})
target_link_libraries(build_test ${LIBS})

add_executable(full_test full_test.cpp ${CPP})
target_link_libraries(full_test ${LIBS})
