CMAKE_MINIMUM_REQUIRED(VERSION 3.0.2 FATAL_ERROR)
 
PROJECT(picojson-download NONE)

INCLUDE(ExternalProject)
EXTERNALPROJECT_ADD(picojson
  GIT_REPOSITORY    https://github.com/kazuho/picojson.git
  GIT_TAG           rel/v1.3.0 
  SOURCE_DIR        "${CMAKE_BINARY_DIR}/picojson-src"
  BINARY_DIR        "${CMAKE_BINARY_DIR}/picojson-build"
  INSTALL_DIR       "${CMAKE_BINARY_DIR}"
  CONFIGURE_COMMAND cp ${CMAKE_SOURCE_DIR}/picojson.cmake ${CMAKE_BINARY_DIR}/picojson-src/CMakeLists.txt
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
  TEST_COMMAND      ""
)

