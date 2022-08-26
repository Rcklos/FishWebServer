cmake_minimum_required(VERSION 3.10)

SET(CMAKE_CXX_COMPILER "g++")
SET(CMAKE_BUILD_TYPE Debug)
SET(CMAKE_EXPORT_COMPILE_COMMANDS ON)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -lpthread")



SET(MY_LIB_DIR lib)
SET(TEST_SOURCE_BASE_DIR test)

project(io_multiplex)

include_directories(inc ${MY_LIB_DIR}/inc)

aux_source_directory(${MY_LIB_DIR}/src CXX_LIB_SRC)
aux_source_directory(${MY_LIB_DIR}/src/kit CXX_KIT_SRC)
aux_source_directory(${MY_LIB_DIR}/src/kit/event CXX_EVENT_SRC)
aux_source_directory(${MY_LIB_DIR}/src/kit/http CXX_HTTP_SRC)
aux_source_directory(src CXX_SRC)

aux_source_directory(${TEST_SOURCE_BASE_DIR}/simple_select CXX_SIMPLE_SELECT_SRC)
aux_source_directory(${TEST_SOURCE_BASE_DIR}/thread_select CXX_THREAD_SELECT_SRC)
aux_source_directory(${TEST_SOURCE_BASE_DIR}/simple_client CXX_SIMPLE_CLIENT_SRC)
aux_source_directory(${TEST_SOURCE_BASE_DIR}/base_server CXX_BASE_SERVER_SRC)

add_executable(simple_select 
  ${CXX_LIB_SRC} 
  ${CXX_KIT_SRC} 
  ${CXX_SIMPLE_SELECT_SRC})

add_executable(thread_select
  ${CXX_LIB_SRC}
  ${CXX_KIT_SRC}
  ${CXX_SRC}
  ${CXX_THREAD_SELECT_SRC})

add_executable(simple_client
  ${CXX_LIB_SRC} 
  ${CXX_KIT_SRC} 
  ${CXX_SIMPLE_CLIENT_SRC})

add_executable(base_server
  ${CXX_LIB_SRC}
  ${CXX_KIT_SRC}
  ${CXX_EVENT_SRC}
  ${CXX_HTTP_SRC}
  ${CXX_BASE_SERVER_SRC})
