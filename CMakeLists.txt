cmake_minimum_required(VERSION 3.2)
project(too VERSION 0.1 LANGUAGES CXX)

set(TOO_CXX_STANDARD 17)

file(GLOB_RECURSE TOO_SRC 
  src/too/*.cpp
  src/too/*.h
  src/too/*.hpp
  src/too/*.hh
  include/*.hpp
)

file(GLOB_RECURSE TOO_TEST_SRC
  test/*.cpp
  test/*.h
  test/*.hpp
)

add_library(too STATIC ${TOO_SRC})
add_executable(too-run src/main.cpp)
add_executable(too-test ${TOO_TEST_SRC})

set_property(TARGET too PROPERTY CXX_STANDARD ${TOO_CXX_STANDARD})
set_property(TARGET too PROPERTY LINKER_LANGUAGE CXX)

set_property(TARGET too-run PROPERTY CXX_STANDARD ${TOO_CXX_STANDARD})
set_property(TARGET too-test PROPERTY CXX_STANDARD ${TOO_CXX_STANDARD})

target_link_libraries(too-run too)
target_link_libraries(too-test too)