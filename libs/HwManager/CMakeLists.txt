
cmake_minimum_required(VERSION 3.0.0)

set(PROJECT_NAME
    LibHwManager
)
set(LIBRARY_NAME
    HwManager
)
set(LIBRARY_HEADERS_DIR
    include/${LIBRARY_NAME}
)

# file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS
#     ${LIBRARY_HEADERS_DIR}/*.h
# )
set(LIBRARY_HEADERS
#    ${HEADER_FILES}
    ${LIBRARY_HEADERS_DIR}/Balancer.h
    ${LIBRARY_HEADERS_DIR}/GpioDriver.h
    ${LIBRARY_HEADERS_DIR}/SerialDriver.h
)
set(LIBRARY_SOURCE_DIR
    src
)
set(LIBRARY_SOURCE
    ${LIBRARY_SOURCE_DIR}/GpioDriver.cpp
    ${LIBRARY_SOURCE_DIR}/SerialDriver.cpp
    ${LIBRARY_SOURCE_DIR}/Balancer.cpp
)

project(${PROJECT_NAME})

add_library(${LIBRARY_NAME} STATIC
    ${LIBRARY_HEADERS}
    ${LIBRARY_SOURCE}
)

find_library(LibWiringPi wiringPi)
target_link_libraries(${LIBRARY_NAME} ${LibWiringPi})

target_compile_options(${LIBRARY_NAME} PRIVATE -Wall -O2 -DNDEBUG -lwiringPi)
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
