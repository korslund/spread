cmake_minimum_required(VERSION 2.6)

include_directories("../")
include_directories("../../")
include_directories("../../libs/")
include_directories("../../libs/jsoncpp/include/")
include_directories(${Boost_INCLUDE_DIRS})

#find_package(Boost COMPONENTS filesystem system thread REQUIRED)
find_package(Boost COMPONENTS thread REQUIRED)
#find_package(CURL REQUIRED)

#add_subdirectory("../../" "${CMAKE_CURRENT_BINARY_DIR}/_spread")

#set(LIBS Spread ${Boost_LIBRARIES} zzip ${CURL_LIBRARIES})
set(LIBS ${Boost_LIBRARIES})

set(SPDIR ../..)
set(JDIR ${SPDIR}/job)
set(PJDIR ${SPDIR}/parent_job)

set(JOB ${JDIR}/job.cpp ${JDIR}/jobinfo.cpp ${JDIR}/thread.cpp)
set(PJOB ${PJDIR}/parentjob.cpp ${PJDIR}/listjob.cpp ${PJDIR}/jobholder.cpp ${PJDIR}/execjob.cpp ${PJDIR}/andjob.cpp)

add_executable(parent_test parent_test.cpp ${JOB} ${PJOB})
target_link_libraries(parent_test ${LIBS})

add_executable(list_test list_test.cpp ${JOB} ${PJOB})
target_link_libraries(list_test ${LIBS})

add_executable(holder_test holder_test.cpp ${JOB} ${PJOB})
target_link_libraries(holder_test ${LIBS})

add_executable(exec_test exec_test.cpp ${JOB} ${PJOB})
target_link_libraries(exec_test ${LIBS})

add_executable(and_test and_test.cpp ${JOB} ${PJOB})
target_link_libraries(and_test ${LIBS})
