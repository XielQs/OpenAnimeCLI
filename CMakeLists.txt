cmake_minimum_required(VERSION 3.16)
project(OpenAnimeCLI LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(inc)

file(GLOB SRC_FILES CONFIGURE_DEPENDS src/*.cpp)

add_compile_options(-Wall -Wextra -pedantic)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-O0 -g3 -ggdb3)
else()
  add_compile_options(-O2)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s")
endif()

# include cpr
include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG dd967cb48ea6bcbad9f1da5ada0db8ac0d532c06)
FetchContent_MakeAvailable(cpr)

add_executable(openanime-cli ${SRC_FILES})

target_link_libraries(openanime-cli PRIVATE cpr::cpr)
