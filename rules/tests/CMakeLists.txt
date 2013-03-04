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

set(DIR ${DDIR}/directory.cpp)
set(CONF ${READJSON} ${MIDIR}/jconfig.cpp)
set(CACHE ${CONF} ${HASH} ${CDIR}/index.cpp)

set(RULES ${DIR} ${CACHE} ${RDIR}/ruleset.cpp ${RDIR}/arcruleset.cpp ${RDIR}/rule_loader.cpp)

add_executable(ifinder1_test ifinder1_test.cpp ${RULES})
target_link_libraries(ifinder1_test ${LIBS})

add_executable(urlrule_test urlrule_test.cpp ${RULES})
target_link_libraries(urlrule_test ${LIBS})

add_executable(ifinder2_test ifinder2_test.cpp ${RULES})
target_link_libraries(ifinder2_test ${LIBS})

add_executable(ifinder3_test ifinder3_test.cpp ${RULES})
target_link_libraries(ifinder3_test ${LIBS})

add_executable(arcrule1_test arcrule1_test.cpp ${RULES})
target_link_libraries(arcrule1_test ${LIBS})

add_executable(arcset1_test arcset1_test.cpp ${RULES})
target_link_libraries(arcset1_test ${LIBS})

add_executable(loader_test loader_test.cpp ${RULES})
target_link_libraries(loader_test ${LIBS})
