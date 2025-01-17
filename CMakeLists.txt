cmake_minimum_required(VERSION 3.22)
project(DatabaseProject)

set(CMAKE_CXX_STANDARD 20)

# Find the fmt library installed on the system
find_package(fmt REQUIRED)

# Add the executable
add_executable(SimpleDatabase src/main.cpp src/database.cpp
        src/utils.h
        src/condition.h
        src/file_io.h
        src/utils.cpp
        src/condition.cpp
        src/file_io.cpp)

# Link the fmt library
target_link_libraries(SimpleDatabase fmt)
