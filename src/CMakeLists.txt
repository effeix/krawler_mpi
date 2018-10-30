cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(MAIN_SOURCES main.cpp)
set(MAIN_EXE_NAME "krawler_mpi")
set(CURL_LIB "-lcurl")

find_package(Boost REQUIRED COMPONENTS regex mpi)
# find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(CURL REQUIRED)
find_package(MPI REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Boost_INCLUDE_DIR}
    ${CURL_INCLUDE_DIR}
    ${MPI_INCLUDE_PATH}
)

file(GLOB SOURCES *.cpp)

add_executable(
    ${MAIN_EXE_NAME}
    ${MAIN_SOURCES}
    ${SOURCES}
)

target_link_libraries(
    ${MAIN_EXE_NAME}
    # ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
    ${CURL_LIB}
    ${CURL_LIBRARIES}
    ${MPI_LIBRARIES}
)
