cmake_minimum_required(VERSION 3.12)

project(WorkQueue_test)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

file(GLOB_RECURSE HEADER_TEST "inc/*.hpp" "inc/*.h")
file(GLOB_RECURSE SOURCE_TEST "src/*.cpp" "src/*.c")

file(GLOB_RECURSE HEADER_SCR "../lib/inc/*.hpp" "../lib/inc/*.h")   #recompile
file(GLOB_RECURSE SOURCE_SCR "../lib/src/*.cpp" "../lib/src/*.c")   #recompile

foreach(item ${SOURCE_SCR})
    if(${item} MATCHES "lib/src/main.cpp")
        list(REMOVE_ITEM SOURCE_SCR ${item})
    endif(${item} MATCHES "lib/src/main.cpp")
endforeach(item)


find_package(GTest REQUIRED)

add_executable(${PROJECT_NAME}  ${SOURCE_SCR} ${SOURCE_TEST}  )

target_include_directories(${PROJECT_NAME} PRIVATE "../lib/inc" "inc" )
target_link_libraries(${PROJECT_NAME} PRIVATE GTest::GTest)



