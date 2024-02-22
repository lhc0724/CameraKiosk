
cmake_minimum_required(VERSION 3.0.0)

set(PROJECT_NAME
    LibUtil
)
set(LIBRARY_NAME
    Util
)
set(LIBRARY_HEADERS_DIR
    include/${LIBRARY_NAME}
)

set(LIBRARY_HEADERS
    ${LIBRARY_HEADERS_DIR}/Socket.h
    ${LIBRARY_HEADERS_DIR}/SocketException.h
    ${LIBRARY_HEADERS_DIR}/ThreadPool.h
)

set(LIBRARY_SOURCE_DIR
    src
)

set(LIBRARY_SOURCE 
    ${LIBRARY_SOURCE_DIR}/ThreadPool.cpp
    ${LIBRARY_SOURCE_DIR}/SocketAcceptor.cpp
    ${LIBRARY_SOURCE_DIR}/SocketCommunicator.cpp
    ${LIBRARY_SOURCE_DIR}/SocketHandler.cpp
    ${LIBRARY_SOURCE_DIR}/SocketException.cpp
)

project(${PROJECT_NAME})

add_library(${LIBRARY_NAME} STATIC
    ${LIBRARY_HEADERS}
    ${LIBRARY_SOURCE}
)

target_compile_options(${LIBRARY_NAME} PRIVATE -Wall -O2 -DNDEBUG -pthread)
target_compile_features(${LIBRARY_NAME} PRIVATE cxx_std_17)

target_include_directories(${LIBRARY_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_include_directories(${LIBRARY_NAME} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/${LIBRARY_NAME}>
    $<INSTALL_INTERFACE:include/${LIBRARY_NAME}>
)

target_include_directories(${LIBRARY_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
