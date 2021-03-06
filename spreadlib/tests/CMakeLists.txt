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

set(LIBDIR ../../libs)
set(MDIR ${LIBDIR}/mangle)
set(MANGLE ${MDIR}/stream/clients/io_stream.cpp)

add_executable(packlist_test packlist_test.cpp ${MANGLE})
target_link_libraries(packlist_test ${LIBS})

add_executable(statuslist_test statuslist_test.cpp ${MANGLE})
target_link_libraries(statuslist_test ${LIBS})

add_executable(chanlist_test chanlist_test.cpp ${MANGLE})
target_link_libraries(chanlist_test ${LIBS})

#add_executable(bug1_test bug1_test.cpp ${MANGLE})
#target_link_libraries(bug1_test ${LIBS})

add_executable(copy_test copy_test.cpp ${MANGLE})
target_link_libraries(copy_test ${LIBS})

add_executable(copy_many_test copy_many_test.cpp ${MANGLE})
target_link_libraries(copy_many_test ${LIBS})

add_executable(spread1_test spread1_test.cpp ${MANGLE})
target_link_libraries(spread1_test ${LIBS})

add_executable(spread2_test spread2_test.cpp ${MANGLE})
target_link_libraries(spread2_test ${LIBS})

add_executable(spread3_test spread3_test.cpp ${MANGLE})
target_link_libraries(spread3_test ${LIBS})

add_executable(legacy_test legacy_test.cpp ${MANGLE})
target_link_libraries(legacy_test ${LIBS})

add_executable(abort_test abort_test.cpp ${MANGLE})
target_link_libraries(abort_test ${LIBS})
