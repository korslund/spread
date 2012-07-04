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

set(CACHE ${CONF} ${HASH} ${CDIR}/index.cpp)

add_executable(cache1_test cache1_test.cpp ${CACHE})
target_link_libraries(cache1_test ${LIBS})

add_executable(cache2_test cache2_test.cpp ${CACHE})
target_link_libraries(cache2_test ${LIBS})
