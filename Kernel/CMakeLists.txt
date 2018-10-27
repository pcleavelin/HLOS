cmake_minimum_required(VERSION 3.8)
project(Kernel)

#set(CMAKE_C_STANDARD 99)

#set(SOURCE_FILES main.c)
#add_executable(Kernel ${SOURCE_FILES})

add_custom_target(Compile ALL ./compile.sh WORKING_DIRECTORY ~/CLionProjects/16OS)