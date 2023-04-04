cmake_minimum_required(VERSION 3.0)
project(async-http-downloader CXX)
enable_language(CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

set(SOURCES_DIR "${CMAKE_CURRENT_LIST_DIR}/src")
set(INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/include")
set(VENDOR_DIR "${CMAKE_CURRENT_LIST_DIR}/vendor")

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS "${SOURCES_DIR}/*.cpp")

add_definitions(-DBIT7Z_AUTO_FORMAT)

add_subdirectory("${VENDOR_DIR}/yaml-cpp")
add_subdirectory("${VENDOR_DIR}/bit7z")

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE yaml-cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE bit7z64)

target_include_directories(${PROJECT_NAME}
    PRIVATE ${INCLUDE_DIR}
    PRIVATE ${SOURCES_DIR}
    PRIVATE "${VENDOR_DIR}/yaml-cpp/include"
    PRIVATE "${VENDOR_DIR}/bit7z/include"
)

