cmake_minimum_required(VERSION 2.6)

find_package(Boost COMPONENTS filesystem system thread REQUIRED)
find_package(CURL REQUIRED)
set(LIBS ${Boost_LIBRARIES} zzip ${CURL_LIBRARIES})

include_directories("../")
include_directories("../../")
include_directories("../../libs/")
include_directories("../../libs/jsoncpp/include/")

set(SPDIR ../..)
set(LIBDIR ../../libs)

set(SLDIR ${SPDIR}/spreadlib)
set(MDIR ${LIBDIR}/mangle)
set(JS ${LIBDIR}/jsoncpp/src)
set(MIDIR ${SPDIR}/misc)
set(UDIR ${SPDIR}/unpack)
set(TDIR ${SPDIR}/tasks)
set(JDIR ${SPDIR}/job)
set(HDIR ${SPDIR}/hash)
set(HTDIR ${SPDIR}/htasks)
set(IDIR ${SPDIR}/install)
set(DDIR ${SPDIR}/dir)
set(RDIR ${SPDIR}/rules)
set(CDIR ${SPDIR}/cache)
set(S0DIR ${SPDIR}/sr0)

set(UNPACK ${UDIR}/dirwriter.cpp ${UDIR}/base.cpp ${UDIR}/unpack_zip.cpp ${UDIR}/auto.cpp)
set(JOB ${JDIR}/thread.cpp ${JDIR}/job.cpp)
set(TASKS ${TDIR}/unpack.cpp ${TDIR}/curl.cpp ${TDIR}/download.cpp ${TDIR}/install.cpp ${TDIR}/multi.cpp ${TDIR}/fileop.cpp ${TDIR}/dircopy.cpp ${TDIR}/notify.cpp ${TDIR} ${JOB} ${UNPACK})

set(C85 ${MIDIR}/comp85.cpp)
set(HASH ${HDIR}/hash.cpp ${LIBDIR}/sha2/sha2.c)

set(HTASKS ${HASH} ${TASKS} ${HTDIR}/hashtask.cpp ${HTDIR}/unpackhash.cpp ${HTDIR}/downloadhash.cpp ${HTDIR}/copyhash.cpp)

set(MANGLE ${MDIR}/stream/clients/io_stream.cpp)
set(JSON ${JS}/json_reader.cpp ${JS}/json_writer.cpp ${JS}/json_value.cpp)
set(READJSON ${JSON} ${MANGLE} ${MIDIR}/readjson.cpp)

set(DIR ${DDIR}/directory.cpp)
set(CONF ${READJSON} ${MIDIR}/jconfig.cpp ${C85})
set(CACHE ${CONF} ${HASH} ${CDIR}/index.cpp)

set(RULES ${DIR} ${CACHE} ${RDIR}/install_finder.cpp ${RDIR}/ruleset.cpp ${RDIR}/arcruleset.cpp ${RDIR}/rule_loader.cpp)

set(ACTINST ${HTASKS} ${IDIR}/action_installer.cpp)
set(ACTBUILD ${RULES} ${IDIR}/build_actions.cpp)

set(INSTALL ${ACTINST} ${ACTBUILD} ${IDIR}/installer.cpp)

set(SR0 ${INSTALL} ${S0DIR}/sr0.cpp)

add_executable(file_test file_test.cpp ${SR0})
target_link_libraries(file_test ${LIBS})
