cmake_minimum_required(VERSION 2.6)

include_directories("../")
include_directories("../../libs/")

set(HASH ../hash/hash.cpp ../libs/sha2/sha2.c ../misc/comp85.cpp)

add_executable(spreadsum spreadsum.cpp ${HASH})
