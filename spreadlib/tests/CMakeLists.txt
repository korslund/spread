cmake_minimum_required(VERSION 2.6)

include_directories("../")
include_directories("../../")
include_directories("../../libs/")
include_directories("../../libs/jsoncpp/include/")
include_directories(${Boost_INCLUDE_DIRS})

find_package(Boost COMPONENTS filesystem system thread REQUIRED)
find_package(CURL REQUIRED)

#add_subdirectory("../../" "${CMAKE_CURRENT_BINARY_DIR}/_spread")
#set(LIBS Spread ${Boost_LIBRARIES} zzip ${CURL_LIBRARIES})
set(LIBS ${Boost_LIBRARIES})

set(SPDIR ../..)
set(LIBDIR ../../libs)

set(MDIR ${LIBDIR}/mangle)
set(JS ${LIBDIR}/jsoncpp/src)
set(MIDIR ${SPDIR}/misc)
set(SLDIR ${SPDIR}/spreadlib)

set(MANGLE ${MDIR}/stream/clients/io_stream.cpp)
set(JSON ${JS}/json_reader.cpp ${JS}/json_writer.cpp ${JS}/json_value.cpp)
set(READJSON ${JSON} ${MANGLE} ${MIDIR}/readjson.cpp)

set(SPLIB ${READJSON} ${SLDIR}/packlist.cpp ${SLDIR}/statuslist.cpp ${SLDIR}/infojson.cpp)

add_executable(packlist_test packlist_test.cpp ${SPLIB})
target_link_libraries(packlist_test ${LIBS})

add_executable(statuslist_test statuslist_test.cpp ${SPLIB})
target_link_libraries(statuslist_test ${LIBS})

#add_executable(bug1_test bug1_test.cpp)
#target_link_libraries(bug1_test ${LIBS})

#add_executable(copy_test copy_test.cpp)
#target_link_libraries(copy_test ${LIBS})

#add_executable(copy_many_test copy_many_test.cpp)
#target_link_libraries(copy_many_test ${LIBS})
