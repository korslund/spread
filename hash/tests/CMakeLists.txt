cmake_minimum_required(VERSION 2.6)

include_directories("../")
include_directories("../../libs/")

set(C85 ../../misc/comp85.cpp)
set(HASH ../hash.cpp ../../libs/sha2/sha2.c ${C85})

add_executable(hash_test hash_test.cpp ${HASH})
add_executable(stream_test stream_test.cpp ${HASH})
