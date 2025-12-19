cmake_minimum_required(VERSION 3.10)
project(Lachut CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#
# Serwer
#

file(GLOB SERVER_SOURCES
    src/server/*.cpp
    src/server/*.h
)

add_executable(server ${SERVER_SOURCES})

target_compile_options(server PRIVATE 
    -Wall
    -Wextra
)

# skopiuj katalog quizSource do folderu build przy każdym buildzie
file(COPY ${CMAKE_SOURCE_DIR}/quizSource DESTINATION ${CMAKE_BINARY_DIR})

#
# Klient
#
#file(GLOB CLIENT_SOURCES
#    src/client/*.cpp
#    src/client/*.h
#)

#add_executable(client ${CLIENT_SOURCES})