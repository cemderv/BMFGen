cmake_minimum_required(VERSION 3.20)

include(cmake/CPM.cmake)

if (NOT CMAKE_PREFIX_PATH)
  if (WIN32)
    # Specify Qt installation path
    set(CMAKE_PREFIX_PATH "C:/Qt/6.7.2/msvc2019_64")
  endif ()
endif ()

project(
  BMFGen
  VERSION 1.2.0
  LANGUAGES CXX
  DESCRIPTION "Bitmap Font Generator"
  HOMEPAGE_URL "https://bmfgen.com"
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

message(STATUS "[BMFGen] Looking for Qt")
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# GSL
CPMAddPackage(
  NAME GSL
  GITHUB_REPOSITORY microsoft/GSL
  VERSION 4.0.0
  OPTIONS
  "GSL_CXX_STANDARD 20"
  "GSL_INSTALL OFF"
  "GSL_TEST OFF"
  "GSL_VS_ADD_NATIVE_VISUALIZERS ON"
  GIT_SHALLOW
  SYSTEM
)

qt_standard_project_setup()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_subdirectory("src")

