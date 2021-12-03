cmake_minimum_required(VERSION 3.1.0)
project(guzum)

set(GUZUM_VERSION 2.0)

configure_file (
  "${PROJECT_SOURCE_DIR}/src/settings.h.in"
  "${PROJECT_BINARY_DIR}/src/settings.h"
)

# tell where qt is
if(APPLE)
    set(CMAKE_PREFIX_PATH $ENV{HOME}/Qt/5.15.2/clang_64/lib/cmake)
endif()

#if(WIN32)
#    set(CMAKE_PREFIX_PATH C:/Qt/Qt5.7.1/5.7/msvc2015/lib/cmake)
#endif()

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

add_subdirectory(src)
