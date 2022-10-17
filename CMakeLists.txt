cmake_minimum_required(VERSION 3.16)

project(edgeFinder CXX)

set(CXX ON)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

SET(PROJECT_CMAKE_SEARCH_PATH "C:/Qt/6.3.1/msvc2019_64" CACHE PATH "Additional Qt5 search path" )

list(APPEND CMAKE_PREFIX_PATH "${PROJECT_CMAKE_SEARCH_PATH}/lib/cmake/")
list(APPEND CMAKE_PREFIX_PATH "${PROJECT_SOURCE_DIR}")

# Visual Studio compiler with static runtime libraries
if(MSVC AND MT)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)
qt_standard_project_setup()

add_definitions(/std:c++latest)

# Add base folder for better inclusion paths
include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/src")

# Main Sources
file(GLOB PROJECT_HEADERS ${PROJECT_SOURCE_DIR}/src/*.h)
file(GLOB PROJECT_SOURCES_CPP ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_executable(${CMAKE_PROJECT_NAME} ${PROJECT_HEADERS} ${PROJECT_SOURCES_CPP})

set(CMAKE_CXX_STANDARD 20)

target_link_libraries(${CMAKE_PROJECT_NAME} Qt6::Core Qt6::Gui)

