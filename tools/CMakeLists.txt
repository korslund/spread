cmake_minimum_required(VERSION 2.6)

find_package(Boost COMPONENTS filesystem system REQUIRED)
set(LIBS ${Boost_LIBRARIES})

set(SPDIR ../)
set(LIBDIR ../libs)
set(JSDIR ${LIBDIR}/jsoncpp/src/)
set(JSINC ${LIBDIR}/jsoncpp/include/)

include_directories(${SPDIR})
include_directories(${LIBDIR})
include_directories(${JSINC})

set(MDIR ${LIBDIR}/mangle)
set(CDIR ${SPDIR}/cache)
set(MIDIR ${SPDIR}/misc)
set(HDIR ${SPDIR}/hash)

set(MANGLE ${MDIR}/stream/clients/io_stream.cpp)
set(JSON ${JSDIR}/json_reader.cpp ${JSDIR}/json_writer.cpp ${JSDIR}/json_value.cpp)
set(READJSON ${JSON} ${MANGLE} ${MIDIR}/readjson.cpp)

set(C85 ${MIDIR}/comp85.cpp)
set(HASH ${HDIR}/hash.cpp ${LIBDIR}/sha2/sha2.c ${C85})
set(CONF ${READJSON} ${MIDIR}/jconfig.cpp)
set(CACHE ${CONF} ${HASH} ${CDIR}/index.cpp ${CDIR}/files.cpp)

add_executable(spreadsum spreadsum.cpp ${HASH})

add_executable(clean_cache clean_cache.cpp ${CACHE})
target_link_libraries(clean_cache ${LIBS})
