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

add_executable(hashtask_test hashtask_test.cpp ${CPP})
target_link_libraries(hashtask_test ${LIBS})

add_executable(unpack_test unpack_test.cpp ${CPP})
target_link_libraries(unpack_test ${LIBS})

add_executable(blindunpack_test blindunpack_test.cpp ${CPP})
target_link_libraries(blindunpack_test ${LIBS})

add_executable(dl_test dl_test.cpp ${CPP})
target_link_libraries(dl_test ${LIBS})

add_executable(copy_test copy_test.cpp ${CPP})
target_link_libraries(copy_test ${LIBS})
