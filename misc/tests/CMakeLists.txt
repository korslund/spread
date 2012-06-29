cmake_minimum_required(VERSION 2.6)

include_directories("../")

set(MIDIR ../../misc)
set(C85 ${MIDIR}/comp85.cpp)

add_executable(comp85_test comp85_test.cpp ${C85})
