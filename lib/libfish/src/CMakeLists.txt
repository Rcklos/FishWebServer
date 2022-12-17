cmake_minimum_required(VERSION 3.2)
project(libfish)
set(LIBFISH_VERSION 0.1)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../build/bin)
set(LIBRARY_OUTPUT_PATH /tmp/libfish_lib)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAAGS} -O3 -g -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")
enable_language(C ASM)

set(libs dl)

include_directories(${PROJECT_SOURCE_DIR})
# MESSAGE(source dir: ${PROJECT_SOURCE_DIR})
# aux_source_directory(${PROJECT_SOURCE_DIR} source_files)
file(GLOB source_files ${PROJECT_SOURCE_DIR}/*.cpp ${PROJECT_SOURCE_DIR}/*.S ${PROJECT_SOURCE_DIR}/http/*.cpp)
# MESSAGE(${source_files})
file(GLOB test_files ${PROJECT_SOURCE_DIR}/../test/*.cpp)

add_library(cofish_static STATIC ${source_files})
add_library(cofish_shared STATIC ${source_files})
target_link_libraries(cofish_static ${libs})
target_link_libraries(cofish_shared ${libs})
set_target_properties(cofish_static PROPERTIES OUTPUT_NAME cofish)
set_target_properties(cofish_shared PROPERTIES OUTPUT_NAME cofish VERSION ${LIBFISH_VERSION} SOVERSION ${LIBFISH_VERSION})

install(TARGETS cofish_shared LIBRARY DESTINATION "lib" ARCHIVE DESTINATION "lib")
install(DIRECTORY . ./http DESTINATION "include/cofish" FILES_MATCHING PATTERN "*.h")
install(DIRECTORY ./util DESTINATION "include/cofish" FILES_MATCHING PATTERN "*.hpp")

set(libs dl cofish_static)


foreach(test_file ${test_files})
  STRING(REGEX REPLACE ".*test_(.+)\\..+$" "\\1" target ${test_file})
  add_executable(${target} ${test_file})
  target_link_libraries(${target} ${libs})
endforeach(test_file)
