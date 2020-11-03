cmake_minimum_required(VERSION 2.8)

project(doralib)

set(TARGETS "doralib")
set(SUBDIRS "net" "thpool")
set(INCDIRS "")

set(OUTDIRS "build")
set(LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/${OUTDIRS}/lib/")
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/${OUTDIRS}/bin/")

set(LIB_TYPE "SHARED")
set(CMAKE_BUILD_TYPE "Release")   # Debug Release
# set(CMAKE_INSTALL_PREFIX "/home/$ENV{USER}")

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=c++11")
    set(CMAKE_CXX_FLAGS "-lpthreads")
    set(CMAKE_CXX_FLAGS "-Wall")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -g -ggdb")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3") 
endif(CMAKE_COMPILER_IS_GNUCXX)

foreach(incdir ${INCDIRS})
    include_directories(${incdir})
endforeach()

foreach(subdir ${SUBDIRS})
    add_subdirectory(${PROJECT_SOURCE_DIR}/${subdir})
endforeach()

link_directories(${LIBRARY_OUTPUT_PATH})

