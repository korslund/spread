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
set(JOB ${JDIR}/job.cpp ${JDIR}/jobinfo.cpp ${JDIR}/thread.cpp)

add_executable(test1 test1.cpp ${JOB})
target_link_libraries(test1 ${LIBS})
