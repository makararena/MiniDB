# CMakeLists.txt

cmake_minimum_required(VERSION 3.29)
project(DatabaseProject)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(
        fmt
        GIT_REPOSITORY  https://github.com/fmtlib/fmt
        GIT_TAG         11.0.2
)
FetchContent_MakeAvailable(fmt)

add_executable(SimpleDatabase src/main.cpp src/database.cpp
        src/utils.h
        src/condition.h
        src/file_io.h
        src/utils.cpp
        src/condition.cpp
        src/file_io.cpp)
target_link_libraries(SimpleDatabase fmt)