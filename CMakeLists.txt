cmake_minimum_required(VERSION 3.0)
project(async-http-downloader CXX)

set(SOURCES_DIR "${CMAKE_CURRENT_LIST_DIR}/src")
set(INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/include")

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${SOURCES_DIR}/*.cpp")

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME}
    PRIVATE ${INCLUDE_DIR}
    PRIVATE ${SOURCES_DIR}
)
