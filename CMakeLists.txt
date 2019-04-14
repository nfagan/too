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

file(GLOB_RECURSE TOO_DEBUG_SRC
  debug/too/*.cpp
  debug/too/*.h
  debug/too/*.hpp
)

add_library(too STATIC ${TOO_SRC})
add_library(too-debug STATIC ${TOO_DEBUG_SRC})

add_executable(too-run src/main.cpp)
add_executable(too-test ${TOO_TEST_SRC})
add_executable(too-debug-run debug/main.cpp)

set(TOO_TARGETS too too-run too-test too-debug too-debug-run)

foreach(TARG ${TOO_TARGETS})
  set_property(TARGET ${TARG} PROPERTY CXX_STANDARD ${TOO_CXX_STANDARD})
  set_property(TARGET ${TARG} PROPERTY LINKER_LANGUAGE CXX)
endforeach()

target_link_libraries(too-run too)
target_link_libraries(too-test too)
target_link_libraries(too-debug too)
target_link_libraries(too-debug-run too-debug)

target_include_directories(too-debug PUBLIC src/)