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
set(IJDIR ${SPDIR}/install_jobs)
set(HDIR ${SPDIR}/hash)
set(HTDIR ${SPDIR}/htasks)
set(ISDIR ${SPDIR}/install_system)

set(HASH ${HDIR}/hash.cpp ${SPDIR}/libs/sha2/sha2.c)
set(UNPACK ${UDIR}/dirwriter.cpp ${UDIR}/base.cpp ${UDIR}/unpack_zip.cpp ${UDIR}/auto.cpp)
set(HTASKS ${HTDIR}/hashtask.cpp ${HTDIR}/unpackhash.cpp ${HTDIR}/downloadhash.cpp ${HTDIR}/copyhash.cpp)
set(DIR ${DDIR}/directory.cpp ${DDIR}/from_fs.cpp)
set(TASKS ${TDIR}/unpack.cpp ${TDIR}/curl.cpp ${TDIR}/download.cpp ${JOB} ${UNPACK})
set(ISYS ${ISDIR}/hashfinder.cpp)

set(JOB ${JDIR}/job.cpp ${JDIR}/jobinfo.cpp ${JDIR}/thread.cpp)
set(PJOB ${PJDIR}/parentjob.cpp ${PJDIR}/listjob.cpp ${PJDIR}/jobholder.cpp ${PJDIR}/execjob.cpp ${PJDIR}/andjob.cpp)

set(IJOBS ${IJDIR}/target.cpp ${IJDIR}/filejob.cpp ${IJDIR}/jobmaker.cpp)

set(ALL ${IJOBS} ${JOB} ${PJOB} ${IJOBS} ${HASH} ${UNPACK} ${HTASKS} ${TASKS} ${ISYS})

add_executable(target1_test target1_test.cpp ${ALL})
target_link_libraries(target1_test ${LIBS})

add_executable(target2_test target2_test.cpp ${ALL})
target_link_libraries(target2_test ${LIBS})

add_executable(target3_test target3_test.cpp ${ALL})
target_link_libraries(target3_test ${LIBS})

add_executable(file1_test file1_test.cpp ${ALL})
target_link_libraries(file1_test ${LIBS})
