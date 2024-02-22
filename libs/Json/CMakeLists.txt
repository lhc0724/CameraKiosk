
cmake_minimum_required(VERSION 3.0.0)

set(PROJECT_NAME
    LibJson
)
set(LIBRARY_NAME
    Json
)
set(LIBRARY_HEADERS_DIR
    include/${LIBRARY_NAME}
)

set(LIBRARY_HEADERS
    ${LIBRARY_HEADERS_DIR}/builder.h
)

set(LIBRARY_SOURCE_DIR
    src
)

set(LIBRARY_SOURCE 
    ${LIBRARY_SOURCE_DIR}/builder.cpp
)
project(${PROJECT_NAME})

add_library(${LIBRARY_NAME} STATIC
    ${LIBRARY_HEADERS}
    ${LIBRARY_SOURCE}
)

find_library(LibJsonCpp jsoncpp)
target_link_libraries(${LIBRARY_NAME} ${LibJsonCpp})

target_compile_options(${LIBRARY_NAME} PRIVATE -Wall -O2 -DNDEBUG -ljsoncpp)
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
