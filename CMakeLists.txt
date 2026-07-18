cmake_minimum_required(VERSION 3.25)
project(sigma LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)

add_subdirectory(server)
if (NOT NO_CLIENT)
  add_subdirectory(client)
endif()

if (PACKAGING)
  message(STATUS "Setting version")
  # Generating package with version
  execute_process(
    COMMAND git describe --abbrev=0 --tags
    OUTPUT_VARIABLE tag
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )

  # Tags like "v1.2.3" or "1.2.3" are both supported
  string(REGEX REPLACE "^v" "" clear_tag "${tag}")
  string(REPLACE "." ";" list_tag "${clear_tag}")
  list(GET list_tag 0 MAJOR_VERSION)
  list(GET list_tag 1 MINOR_VERSION)
  list(GET list_tag 2 PATCH_VERSION)

  set(CPACK_PACKAGE_VERSION_MAJOR "${MAJOR_VERSION}")
  set(CPACK_PACKAGE_VERSION_MINOR "${MINOR_VERSION}")
  set(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")
  set(CPACK_PACKAGE_VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}")
  set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
  set(CPACK_PACKAGE_CONTACT elochkaan@gmail.com)
  set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}/package")

  set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}")

  if (WIN32)
    set(CPACK_GENERATOR ZIP)
  else ()
    set(CPACK_GENERATOR TGZ)
  endif()

  include(CPack)
endif()