
cmake_minimum_required(VERSION 3.0.0)

set(PROJECT_NAME
    LibKioskFSM
)
set(LIBRARY_NAME
    KioskFSM
)
set(LIBRARY_HEADERS_DIR
    include/${LIBRARY_NAME}
)

set(LIBRARY_HEADERS
#    ${HEADER_FILES}
    ${LIBRARY_HEADERS_DIR}/DetectEvent.h
    ${LIBRARY_HEADERS_DIR}/InitEvent.h
    ${LIBRARY_HEADERS_DIR}/KioskInterface.h
    ${LIBRARY_HEADERS_DIR}/MoveEvent.h
    ${LIBRARY_HEADERS_DIR}/ExtractEvent.h
)

set(LIBRARY_SOURCE_DIR
    src
)

set(LIBRARY_SOURCE 
    ${LIBRARY_SOURCE_DIR}/DetectEvent.cpp
    ${LIBRARY_SOURCE_DIR}/InitEvent.cpp
    ${LIBRARY_SOURCE_DIR}/KioskInterface.cpp
    ${LIBRARY_SOURCE_DIR}/MoveEvent.cpp
    ${LIBRARY_SOURCE_DIR}/ExtractEvent.cpp
)
project(${PROJECT_NAME})

add_library(${LIBRARY_NAME} STATIC
    ${LIBRARY_HEADERS}
    ${LIBRARY_SOURCE}
)

set(THIRD_PARTY_CBUE_EYE
    ${CMAKE_SOURCE_DIR}/third-party/CubeEye
)
include_directories(${THIRD_PARTY_CBUE_EYE}/include)

target_link_libraries(${LIBRARY_NAME} GpCamera HwManager Util Json)

target_compile_options(${LIBRARY_NAME} PRIVATE -Wall -O2 -DNDEBUG -lwiringPi -ljsoncpp)
target_compile_features(${LIBRARY_NAME} PRIVATE cxx_std_17)

target_include_directories(${LIBRARY_NAME} PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/${LIBRARY_NAME}>
    $<INSTALL_INTERFACE:include/${LIBRARY_NAME}>
)

target_include_directories(${LIBRARY_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# add_subdirectory(test)
