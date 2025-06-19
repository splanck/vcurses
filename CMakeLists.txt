cmake_minimum_required(VERSION 3.5)
project(vcurses C)

add_library(vcurses src/vcurses.c)

target_include_directories(vcurses PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

install(TARGETS vcurses
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)
install(DIRECTORY include/ DESTINATION include)
