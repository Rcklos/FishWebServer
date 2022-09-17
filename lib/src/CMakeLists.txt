include_directories(${PROJECT_SOURCE_DIR})
file(GLOB test_files ${PROJECT_SOURCE_DIR}/../test/*.cpp)
foreach(test_file ${file_files})
  STRING(REGEX REPLACE ".*/(.+)\\..*" "\\1" target ${test_file})
  add_executable(${target} ${test_file})
endforeach