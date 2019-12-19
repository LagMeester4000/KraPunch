cmake_minimum_required(VERSION 3.13.0)
cmake_policy(SET CMP0079 NEW)
cmake_policy(SET CMP0077 NEW)
project(KraPunch)


# User options
set(APP_NAME LagMeester4000\ KraPunch)
set(APP_PACKAGE KraPunch)
set(IS_GAME true)
set(APP_VERSION 1.0) 


# Default options
add_definitions(-D__PROJECT_NAME__="${APP_NAME}")

set(HOME_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_SUPPRESS_REGENERATION true)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)


# dependencies
add_subdirectory(deps)

# app projects
add_subdirectory(KraPunch)
set_property(TARGET KraPunch PROPERTY FOLDER application)