cmake_minimum_required(VERSION 3.22)

include(cmake/UniversalAppleBuild.cmake)

project(
  Pulsar
  DESCRIPTION "madhav's eecs498 game engine"
  VERSION 1.0.0
  LANGUAGES CXX)

set(PROJECT_COMPANY_NAME "Madhav Shekhar Sharma")
set(PROJECT_COMPANY_NAMESPACE "com.madhavshekhar")  # Reverse domain name notation

include(cmake/StandardProjectSettings.cmake)
include(GNUInstallDirs)

# Link this "library" to use the warnings specified in CompilerWarnings.cmake.
add_library(project_warnings INTERFACE)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

enable_testing()

add_subdirectory(packaging)
add_subdirectory(vendor)
add_subdirectory(src)
