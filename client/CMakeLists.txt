
cmake_minimum_required(VERSION 3.12)

project(WorkQueue_client)

file(GLOB_RECURSE HEADER "inc/*.hpp" "inc/*.h")
file(GLOB_RECURSE SOURCE "src/*.cpp" "src/*.c")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

add_executable(${PROJECT_NAME} ${SOURCE} ${HEADER})

target_include_directories(${PROJECT_NAME} PRIVATE "../lib/inc" )
target_link_libraries(${PROJECT_NAME} WorkQueue)