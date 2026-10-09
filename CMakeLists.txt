cmake_minimum_required(VERSION 3.20)

project(Porcupine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(porcupine
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
)