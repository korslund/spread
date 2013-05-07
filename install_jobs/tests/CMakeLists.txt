cmake_minimum_required(VERSION 2.6)

include_directories("../")
include_directories("../../")
include_directories("../../libs/")
include_directories("../../libs/jsoncpp/include/")
include_directories(${Boost_INCLUDE_DIRS})

find_package(Boost COMPONENTS filesystem system thread REQUIRED)
find_package(CURL REQUIRED)
find_package(ZLIB REQUIRED)

add_subdirectory("../../libs/unpackcpp/" "${CMAKE_CURRENT_BINARY_DIR}/_unpackcpp")

set(LIBS UnpackCpp ${Boost_LIBRARIES} ${ZLIB_LIBRARIES} ${CURL_LIBRARIES})

set(SPDIR ../..)

set(DDIR ${SPDIR}/dir)
set(TDIR ${SPDIR}/tasks)
set(JDIR ${SPDIR}/job)
set(PJDIR ${SPDIR}/parent_job)
set(IJDIR ${SPDIR}/install_jobs)
set(HDIR ${SPDIR}/hash)
set(HTDIR ${SPDIR}/htasks)
set(RDIR ${SPDIR}/rules)

set(HASH ${HDIR}/hash.cpp ${SPDIR}/libs/sha2/sha2.c)
set(HTASKS ${HTDIR}/hashtask.cpp ${HTDIR}/unpackhash.cpp ${HTDIR}/downloadhash.cpp ${HTDIR}/copyhash.cpp)
set(DIR ${DDIR}/binary.cpp ${DDIR}/tools.cpp)
set(TASKS ${TDIR}/unpack.cpp ${TDIR}/curl.cpp ${TDIR}/download.cpp ${JOB})
set(RULES ${RDIR}/arcruleset.cpp ${RDIR}/ruleset.cpp)
set(JOB ${JDIR}/job.cpp ${JDIR}/jobinfo.cpp ${JDIR}/thread.cpp)
set(PJOB ${PJDIR}/parentjob.cpp ${PJDIR}/listjob.cpp ${PJDIR}/jobholder.cpp ${PJDIR}/execjob.cpp ${PJDIR}/andjob.cpp)

set(IJOBS ${IJDIR}/treebase.cpp ${IJDIR}/hashfinder.cpp)

set(ALL ${IJOBS} ${JOB} ${PJOB} ${HASH})
set(LEAF ${ALL} ${DIR} ${HTASKS} ${TASKS} ${UNPACK} ${IJDIR}/leaffactory.cpp)

add_executable(base1_test base1_test.cpp ${ALL})
target_link_libraries(base1_test ${LIBS})

add_executable(leaf1_test leaf1_test.cpp ${LEAF})
target_link_libraries(leaf1_test ${LIBS})

add_executable(deadlock_test deadlock_test.cpp ${ALL})
target_link_libraries(deadlock_test ${LIBS})

add_executable(hashfind_test hashfind_test.cpp ${ALL})
target_link_libraries(hashfind_test ${LIBS})
