cmake_minimum_required(VERSION 4.4.3)

# UID found in Help/dev/experimental.rst in CMake github repo
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "f35a9ac6-8463-4d38-8eec-5d6008153e7d")

project(reflection-module-tests)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_MODULE_STD ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include_directories(include/)

add_executable(reflection-module-tests)
target_compile_features(reflection-module-tests PUBLIC cxx_std_26)
target_compile_options(reflection-module-tests PRIVATE -freflection)
target_sources(reflection-module-tests PRIVATE
  src/main.cpp
)
target_sources(reflection-module-tests PUBLIC FILE_SET CXX_MODULES FILES
  src/utils.cppm
  src/derive.cppm
  src/soa.cppm)