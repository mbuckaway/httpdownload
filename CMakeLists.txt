cmake_minimum_required(VERSION 3.0)

set(CMAKE_CONFIGURATION_TYPES debug release CACHE TYPE INTERNAL FORCE)

project(httpdownload)

set(POCO_DIR "${CMAKE_SOURCE_DIR}/poco")
set(POCO_LIB "${POCO_DIR}/lib")
set(POCO_INCLUDE "${POCO_DIR}/include")

message("POCO source directory: ${POCO_DIR}")

# Setup the compiler to support C++11/14/etc
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    set(LIB_PREFIX "lib")
    set(LIB_SUFFIX "so")
    message("Building for Linux")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(CMAKE_OSX_ARCHITECTURES, x86_64)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXXFLAGS} -std=c++0x -stdlib=libc++ -mmacosx-version-min=10.7")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CFLAGS} -mmacosx-version-min=10.7")
    set(LIB_PREFIX "lib")
    set(LIB_SUFFIX "dylib")
    message("Building for MACOSX")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:wchar_t,auto,rvalueCast /EHsc /GR")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Zc:wchar_t,auto,rvalueCast /EHsc /GR")
    set(LIB_PREFIX "")
    set(LIB_SUFFIX "dll")
    message("Building for Windows")
endif()

include_directories(
        ${POCO_INCLUDE}
)

link_directories(
        ${POCO_LIB}
)

add_executable(httpdownload
        httpapplicaton.h
        httpapplicaton.cpp
        main.cpp version.h httpdownloader.cpp httpdownloader.h httpdownloader.h httpdownloader.cpp httpdownloadexceptions.h httpdownloadexceptions.cpp)

set_target_properties(httpdownload PROPERTIES OUTPUT_NAME httpdownload)
set_target_properties(httpdownload PROPERTIES DEBUG_POSTFIX "d")

target_link_libraries(httpdownload
        debug ${LIB_PREFIX}PocoFoundationd.${LIB_SUFFIX}
        optimized ${LIB_PREFIX}PocoFoundation.${LIB_SUFFIX}
        debug ${LIB_PREFIX}PocoUtild.${LIB_SUFFIX}
        optimized ${LIB_PREFIX}PocoUtil.${LIB_SUFFIX}
        debug ${LIB_PREFIX}PocoNetd.${LIB_SUFFIX}
        optimized ${LIB_PREFIX}PocoNet.${LIB_SUFFIX}
        )