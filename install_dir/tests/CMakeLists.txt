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

set(LIBS Spread UnpackCpp ${Boost_LIBRARIES} zzip ${CURL_LIBRARIES})
set(MANGLE ../../libs/mangle/stream/clients/io_stream.cpp)

add_executable(hints1_test hints1_test.cpp ${MANGLE})
target_link_libraries(hints1_test ${LIBS})

add_executable(add_test add_test.cpp ${MANGLE})
target_link_libraries(add_test ${LIBS})

add_executable(dirfile_test dirfile_test.cpp ${MANGLE})
target_link_libraries(dirfile_test ${LIBS})

add_executable(handle_test handle_test.cpp ${MANGLE})
target_link_libraries(handle_test ${LIBS})

add_executable(blinds_test blinds_test.cpp ${MANGLE})
target_link_libraries(blinds_test ${LIBS})

add_executable(sortadddel_test sortadddel_test.cpp ${MANGLE})
target_link_libraries(sortadddel_test ${LIBS})

add_executable(ask_test ask_test.cpp ${MANGLE})
target_link_libraries(ask_test ${LIBS})

add_executable(conflict_test conflict_test.cpp ${MANGLE})
target_link_libraries(conflict_test ${LIBS})

add_executable(conflict_noask_test conflict_noask_test.cpp ${MANGLE})
target_link_libraries(conflict_noask_test ${LIBS})

add_executable(findmoves_test findmoves_test.cpp ${MANGLE})
target_link_libraries(findmoves_test ${LIBS})
