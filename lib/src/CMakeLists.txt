cmake_minimum_required(VERSION 3.2)
project(libfish)
set(LIBFISH_VERSION 0.1)


include_directories(${PROJECT_SOURCE_DIR})
MESSAGE(source dir: ${PROJECT_SOURCE_DIR})
file(GLOB test_files ${PROJECT_SOURCE_DIR}/../test/*.cpp)
foreach(test_file ${test_files})
  STRING(REGEX REPLACE ".*test_(.+)\\.*" "\\1" target ${test_file})
  add_executable(${target} ${test_file})
endforeach(test_file)
