cmake_minimum_required(VERSION 2.6)
project(SFEMU)

set(VERSION "1.0")

set(sources md5.cpp http_message.cpp sf.cpp sfemu.cpp ctoken.cpp)

configure_file(version.h.in ${PROJECT_BINARY_DIR}/version.h)

include_directories("${PROJECT_BINARY_DIR}")

add_executable(sfemu ${sources})

target_link_libraries(sfemu z)

