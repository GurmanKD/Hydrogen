cmake_minimum_required(VERSION 3.20)
project(hydrogen CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(hydro src/main.cpp)

# On Linux we will call nasm/ld from the program; ensure they're present at runtime.
# (CI installs them; locally add them with apt/brew as noted in README.)
