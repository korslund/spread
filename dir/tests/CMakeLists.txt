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
set(HDIR ${SPDIR}/hash)
set(DDIR ${SPDIR}/dir)
set(RDIR ${SPDIR}/rules)
set(CDIR ${SPDIR}/cache)

set(C85 ${MIDIR}/comp85.cpp)
set(HASH ${HDIR}/hash.cpp ${LIBDIR}/sha2/sha2.c ${C85})

set(MANGLE ${MDIR}/stream/clients/io_stream.cpp)
set(JSON ${JS}/json_reader.cpp ${JS}/json_writer.cpp ${JS}/json_value.cpp)
set(READJSON ${JSON} ${MANGLE} ${MIDIR}/readjson.cpp)

set(DIR ${DDIR}/binary.cpp ${DDIR}/tools.cpp)

set(DIRTEST ${DIR} ${MANGLE} ${HASH})

add_executable(binary_test binary_test.cpp ${DIRTEST})
target_link_libraries(binary_test ${LIBS})

add_executable(tools_test tools_test.cpp ${DIRTEST})
target_link_libraries(tools_test ${LIBS})
