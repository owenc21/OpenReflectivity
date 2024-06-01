cmake_minimum_required(VERSION 3.10)
project(OpenReflectivity VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find OpenGL policy (chose new verses OLD)
set(OpenGL_GL_PREFERENCE GLVND)

add_library(open_reflectivity_decoder
  src/decoder.cpp
)

target_include_directories(open_reflectivity_decoder PUBLIC
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/include/zstr/src
  ${PROJECT_SOURCE_DIR}/src
)

# Find packages
find_package(PkgConfig REQUIRED)
find_package(OpenGL REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
find_package(ZLIB REQUIRED)

add_executable(OpenReflectivity src/main.cpp)
target_link_libraries(OpenReflectivity PRIVATE
  open_reflectivity_decoder
  ${GLFW_LIBRARIES}
  ${OPENGL_gl_LIBRARY}
  ZLIB::ZLIB
)

# Add Google Test from local extern directory
add_subdirectory(extern/googletest)

enable_testing()

# Automatically find all test files
file(GLOB TEST_SOURCES "test/*.cpp")

# Create a test executable for each test file and link with the project library
foreach(TEST_SOURCE ${TEST_SOURCES})
  get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
  add_executable(${TEST_NAME} ${TEST_SOURCE})
  target_link_libraries(${TEST_NAME} gtest_main open_reflectivity_decoder)
  include(GoogleTest)
  gtest_discover_tests(${TEST_NAME})
endforeach()
