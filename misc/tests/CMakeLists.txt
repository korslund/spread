cmake_minimum_required(VERSION 2.6)

include_directories("../")
include_directories("../../libs/")
include_directories("../../libs/jsoncpp/include/")

find_package(Boost COMPONENTS filesystem system thread REQUIRED)
set(BLIBS ${Boost_LIBRARIES})

set(SPDIR ../..)
set(LIBDIR ../../libs)

set(MDIR ${LIBDIR}/mangle)
set(JS ${LIBDIR}/jsoncpp/src)
set(MIDIR ${SPDIR}/misc)

set(MANGLE ${MDIR}/stream/clients/io_stream.cpp)
set(JSON ${JS}/json_reader.cpp ${JS}/json_writer.cpp ${JS}/json_value.cpp)
set(READJSON ${JSON} ${MANGLE} ${MIDIR}/readjson.cpp)

set(C85 ${MIDIR}/comp85.cpp)

set(CONF ${READJSON} ${MIDIR}/jconfig.cpp ${C85})

add_executable(comp85_test comp85_test.cpp ${C85})

add_executable(json_test json_test.cpp ${READJSON})

add_executable(conf_test conf_test.cpp ${CONF})
target_link_libraries(conf_test ${BLIBS})

add_executable(conf2_test conf2_test.cpp ${CONF})
target_link_libraries(conf2_test ${BLIBS})

add_executable(conf_reg1_test conf_reg1_test.cpp ${CONF})
target_link_libraries(conf_reg1_test ${BLIBS})

add_executable(rand_test rand_test.cpp)
