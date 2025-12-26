cmake_minimum_required(VERSION 3.10)
project(Lachut CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include_directories(
    src/client
    src/client/gui
)

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

#
# Klient
#
file(GLOB CLIENT_SOURCES
   src/client/*.h
   src/client/*.cpp
   src/client/gui/*.ui
   src/client/gui/*.h
)

add_executable(client ${CLIENT_SOURCES})
target_compile_options(client PRIVATE
    -Wall
    -Wextra
)

# Qt
find_package(Qt6 COMPONENTS Widgets REQUIRED)
target_link_libraries(client PRIVATE Qt6::Widgets)

# skopiuj katalog quizSource do folderu build przy każdym buildzie
add_custom_target(copy_quiz ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/quizSource
        ${CMAKE_BINARY_DIR}/quizSource
)

