cmake_minimum_required(VERSION 2.6)

find_package(Boost COMPONENTS filesystem system REQUIRED)
set(LIBS ${Boost_LIBRARIES})

include_directories("../")
include_directories("../../")
include_directories("../../libs/")
include_directories("../../libs/jsoncpp/include/")

set(SPDIR ../..)
set(LIBDIR ../../libs)

set(MDIR ${LIBDIR}/mangle)
set(JS ${LIBDIR}/jsoncpp/src)
set(MIDIR ${SPDIR}/misc)
set(JDIR ${SPDIR}/job)
set(HDIR ${SPDIR}/hash)
set(CDIR ${SPDIR}/cache)

set(JOB ${JDIR}/thread.cpp ${JDIR}/job.cpp)
set(C85 ${MIDIR}/comp85.cpp)
set(HASH ${HDIR}/hash.cpp ${LIBDIR}/sha2/sha2.c ${C85})

set(MANGLE ${MDIR}/stream/clients/io_stream.cpp)
set(JSON ${JS}/json_reader.cpp ${JS}/json_writer.cpp ${JS}/json_value.cpp)
set(READJSON ${JSON} ${MANGLE} ${MIDIR}/readjson.cpp)

set(CONF ${READJSON} ${MIDIR}/jconfig.cpp)

set(CACHE ${CONF} ${HASH} ${CDIR}/index.cpp ${CDIR}/files.cpp)

add_executable(cache1_test cache1_test.cpp ${CACHE})
target_link_libraries(cache1_test ${LIBS})

add_executable(cache2_test cache2_test.cpp ${CACHE})
target_link_libraries(cache2_test ${LIBS})

add_executable(cache3_test cache3_test.cpp ${CACHE})
target_link_libraries(cache3_test ${LIBS})

add_executable(cache4_test cache4_test.cpp ${CACHE})
target_link_libraries(cache4_test ${LIBS})

add_executable(cache5_test cache5_test.cpp ${CACHE})
target_link_libraries(cache5_test ${LIBS})

add_executable(cache6_test cache6_test.cpp ${CACHE})
target_link_libraries(cache6_test ${LIBS})

add_executable(cache_speed1 cache_speed1.cpp ${CACHE})
target_link_libraries(cache_speed1 ${LIBS})

add_executable(virtual_test virtual_test.cpp ${CACHE})
target_link_libraries(virtual_test ${LIBS})

add_executable(files_test files_test.cpp ${CACHE})
target_link_libraries(files_test ${LIBS})
