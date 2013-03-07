cmake_minimum_required(VERSION 2.6)

include_directories("../")
include_directories("../../")
include_directories("../../libs/")
include_directories("../../libs/jsoncpp/include/")
include_directories(${Boost_INCLUDE_DIRS})

find_package(Boost COMPONENTS filesystem system thread REQUIRED)
find_package(Boost COMPONENTS thread REQUIRED)
find_package(CURL REQUIRED)

set(LIBS ${Boost_LIBRARIES} zzip ${CURL_LIBRARIES})

set(SPDIR ../..)

set(DDIR ${SPDIR}/dir)
set(UDIR ${SPDIR}/unpack)
set(TDIR ${SPDIR}/tasks)
set(JDIR ${SPDIR}/job)
set(PJDIR ${SPDIR}/parent_job)
set(HDIR ${SPDIR}/hash)
set(HTDIR ${SPDIR}/htasks)
set(RDIR ${SPDIR}/rules)
set(IJDIR ${SPDIR}/install_jobs)
set(IDDIR ${SPDIR}/install_dir)

set(HASH ${HDIR}/hash.cpp ${SPDIR}/libs/sha2/sha2.c)
set(UNPACK ${UDIR}/dirwriter.cpp ${UDIR}/base.cpp ${UDIR}/unpack_zip.cpp ${UDIR}/auto.cpp)
set(HTASKS ${HTDIR}/hashtask.cpp ${HTDIR}/unpackhash.cpp ${HTDIR}/downloadhash.cpp ${HTDIR}/copyhash.cpp)
set(DIR ${DDIR}/binary.cpp ${DDIR}/tools.cpp)
set(TASKS ${TDIR}/unpack.cpp ${TDIR}/curl.cpp ${TDIR}/download.cpp ${JOB} ${UNPACK})
set(RULES ${RDIR}/arcruleset.cpp ${RDIR}/ruleset.cpp)
set(JOB ${JDIR}/job.cpp ${JDIR}/jobinfo.cpp ${JDIR}/thread.cpp)
set(PJOB ${PJDIR}/parentjob.cpp ${PJDIR}/listjob.cpp ${PJDIR}/jobholder.cpp ${PJDIR}/execjob.cpp ${PJDIR}/andjob.cpp)
set(IJOBS ${IJDIR}/treebase.cpp ${IJDIR}/hashfinder.cpp ${IJDIR}/leaffactory.cpp)
set(IDIR ${IDDIR}/dir_install.cpp)

set(ALL ${IDIR} ${IJOBS} ${JOB} ${PJOB} ${HASH} ${UNPACK} ${HTASKS} ${TASKS} ${DIR} ${RULES})

add_executable(hints1_test hints1_test.cpp ${ALL})
target_link_libraries(hints1_test ${LIBS})

add_executable(add_test add_test.cpp ${ALL})
target_link_libraries(add_test ${LIBS})

add_executable(dirfile_test dirfile_test.cpp ${ALL})
target_link_libraries(dirfile_test ${LIBS})

add_executable(handle_test handle_test.cpp ${ALL})
target_link_libraries(handle_test ${LIBS})

add_executable(blinds_test blinds_test.cpp ${ALL})
target_link_libraries(blinds_test ${LIBS})
