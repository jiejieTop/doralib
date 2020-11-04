cmake_minimum_required(VERSION 3.0.2)
project(example)

# set example out path
get_filename_component(PARENT_DIR ${PROJECT_SOURCE_DIR} DIRECTORY)
set(OUTDIRS "build_example")
set(LIBRARY_OUTPUT_PATH "${PARENT_DIR}/${OUTDIRS}/lib/")
set(EXECUTABLE_OUTPUT_PATH "${PARENT_DIR}/${OUTDIRS}/bin/")

# example depent lib
set(DEPENT_DORA_LIBNAME "net" "thpool" "common")

set(CMAKE_BUILD_TYPE "Debug")   # Debug Release

# lib prefix
# set(CMAKE_INSTALL_PREFIX "/home/$ENV{USER}")

foreach(TMP_LIB ${DEPENT_DORA_LIBNAME})
  set(DORALIB_LINK_DIR "${CMAKE_INSTALL_PREFIX}/doralib/${TMP_LIB}/lib/;${DORALIB_LINK_DIR}")
  set(DORALIB_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/doralib/${TMP_LIB}/include/;${DORALIB_INCLUDE_DIR}")
endforeach()


file(GLOB FILES LIST_DIRECTORIES false *)

foreach(FILE ${FILES})
    message(STATUS ${FILE})
endforeach()

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=c++11")
    set(CMAKE_CXX_FLAGS "-lpthreads")
    set(CMAKE_CXX_FLAGS "-Wall")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -O0 -g -ggdb")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3") 
endif(CMAKE_COMPILER_IS_GNUCXX)

###########
## build ##
###########

find_package("Threads")

include_directories(${DORALIB_INCLUDE_DIR})
link_directories(${DORALIB_LINK_DIR})

aux_source_directory(. TMP_SRCS)

foreach(EXMAPLE_SRC_NAME ${TMP_SRCS})
  get_filename_component(EXMAPLE_NAME ${EXMAPLE_SRC_NAME} NAME_WE)
  add_executable(${EXMAPLE_NAME} ${EXMAPLE_SRC_NAME})
  target_link_libraries(${EXMAPLE_NAME} ${DEPENT_DORA_LIBNAME} ${CMAKE_THREAD_LIBS_INIT})
endforeach()

