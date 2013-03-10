cmake_minimum_required(VERSION 2.6)

find_package(Boost COMPONENTS filesystem system thread REQUIRED)
find_package(CURL REQUIRED)

include_directories("../")
include_directories("../../")
include_directories("../../libs/")
include_directories("../../libs/jsoncpp/include/")
include_directories(${Boost_INCLUDE_DIRS})

set(LIBS ${Boost_LIBRARIES} zzip ${CURL_LIBRARIES})

set(SPDIR ../..)
set(LIBDIR ../../libs)

set(MDIR ${LIBDIR}/mangle)
set(JS ${LIBDIR}/jsoncpp/src)
set(MIDIR ${SPDIR}/misc)
set(DDIR ${SPDIR}/dir)
set(UDIR ${SPDIR}/unpack)
set(TDIR ${SPDIR}/tasks)
set(JDIR ${SPDIR}/job)
set(PJDIR ${SPDIR}/parent_job)
set(HDIR ${SPDIR}/hash)
set(HTDIR ${SPDIR}/htasks)
set(RDIR ${SPDIR}/rules)
set(CDIR ${SPDIR}/cache)
set(IJDIR ${SPDIR}/install_jobs)
set(IDDIR ${SPDIR}/install_dir)
set(ISDIR ${SPDIR}/install_system)

set(MANGLE ${MDIR}/stream/clients/io_stream.cpp)
set(JSON ${JS}/json_reader.cpp ${JS}/json_writer.cpp ${JS}/json_value.cpp)
set(READJSON ${JSON} ${MANGLE} ${MIDIR}/readjson.cpp)
set(CONF ${READJSON} ${MIDIR}/jconfig.cpp ${MIDIR}/comp85.cpp)
set(LOG ${MIDIR}/logger.cpp)

set(HASH ${HDIR}/hash.cpp ${SPDIR}/libs/sha2/sha2.c)
set(UNPACK ${UDIR}/dirwriter.cpp ${UDIR}/base.cpp ${UDIR}/unpack_zip.cpp ${UDIR}/auto.cpp)
set(HTASKS ${HTDIR}/hashtask.cpp ${HTDIR}/unpackhash.cpp ${HTDIR}/downloadhash.cpp ${HTDIR}/copyhash.cpp)
set(DIR ${DDIR}/binary.cpp ${DDIR}/tools.cpp ${DDIR}/from_fs.cpp)
set(TASKS ${TDIR}/unpack.cpp ${TDIR}/curl.cpp ${TDIR}/download.cpp ${JOB} ${UNPACK})
set(RULES ${RDIR}/arcruleset.cpp ${RDIR}/ruleset.cpp)
set(JOB ${JDIR}/job.cpp ${JDIR}/jobinfo.cpp ${JDIR}/thread.cpp)
set(PJOB ${PJDIR}/parentjob.cpp ${PJDIR}/listjob.cpp ${PJDIR}/jobholder.cpp ${PJDIR}/execjob.cpp ${PJDIR}/andjob.cpp ${PJDIR}/askqueue.cpp)
set(IJOBS ${IJDIR}/treebase.cpp ${IJDIR}/hashfinder.cpp ${IJDIR}/leaffactory.cpp)
set(IDIR ${IDDIR}/dir_install.cpp)
set(CHE ${CDIR}/index.cpp ${CDIR}/files.cpp)
set(ISYS ${ISDIR}/jobmanager.cpp)

set(ALL ${ISYS} ${CONF} ${IDIR} ${IJOBS} ${CHE} ${JOB} ${PJOB} ${HASH} ${UNPACK} ${HTASKS} ${TASKS} ${DIR} ${RULES} ${LOG})

add_executable(manager_test manager_test.cpp ${ALL})
target_link_libraries(manager_test ${LIBS})

add_executable(file_test file_test.cpp ${ALL})
target_link_libraries(file_test ${LIBS})

add_executable(unpack_test unpack_test.cpp ${ALL})
target_link_libraries(unpack_test ${LIBS})

add_executable(diff_test diff_test.cpp ${ALL})
target_link_libraries(diff_test ${LIBS})
