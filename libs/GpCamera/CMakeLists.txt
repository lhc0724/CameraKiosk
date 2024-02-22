
cmake_minimum_required(VERSION 3.0.0)

set(PROJECT_NAME
    LibGpCamera
)
set(LIBRARY_NAME
    GpCamera
)

set(THIRD_PARTY_CBUE_EYE
    ${CMAKE_SOURCE_DIR}/third-party/CubeEye
)

set(LIBRARY_HEADERS_DIR
    include/${LIBRARY_NAME}
)

file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS
    ${LIBRARY_HEADERS_DIR}/*.h
)
set(LIBRARY_HEADERS
    #${HEADER_FILES}
    ${LIBRARY_HEADERS_DIR}/GpCamera.h
    ${LIBRARY_HEADERS_DIR}/GpCommon.h
    ${LIBRARY_HEADERS_DIR}/CubeEyeWrapper.h
    ${LIBRARY_HEADERS_DIR}/CubeEyeDriver.h
    ${LIBRARY_HEADERS_DIR}/EcCubeEyeProps.h
    ${LIBRARY_HEADERS_DIR}/FrameParser.h
)
set(LIBRARY_SOURCE_DIR
    src
)
set(LIBRARY_SOURCE
    ${LIBRARY_SOURCE_DIR}/GpCamera.cpp
    ${LIBRARY_SOURCE_DIR}/CubeEyeDriver.cpp
    ${LIBRARY_SOURCE_DIR}/EcCubeEyeProps.cpp
    ${LIBRARY_SOURCE_DIR}/FrameParser.cpp
)

project(${PROJECT_NAME})

add_library(${LIBRARY_NAME} STATIC
    ${LIBRARY_HEADERS}
    ${LIBRARY_SOURCE}
)

find_library(LibGphoto gphoto2)
find_library(LibGphotoPort gphoto2_port)

include_directories(${THIRD_PARTY_CBUE_EYE}/include)

#target_link_libraries(${LIBRARY_NAME} ${LibGphoto})
target_link_libraries(${LIBRARY_NAME} ${LibGphoto} ${LibGphotoPort} Util)
target_link_libraries(${LIBRARY_NAME} ${THIRD_PARTY_CBUE_EYE}/lib/libCubeEye.so)

target_compile_options(${LIBRARY_NAME} PRIVATE -Wall -O2 -DNDEBUG -lgphoto2 -lgphoto2_port -lCubeEye)
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
