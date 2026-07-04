cmake_minimum_required(VERSION 3.28)
project(sigma LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(SERVER_NAME server)
set(CMAKE_AUTOMOC ON)

######## SERVER CONFIG ########
#                             #
file(GLOB SERVER_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_SOURCE_DIR}/server/src/*.cpp
)
file(GLOB SERVER_HEADERS CONFIGURE_DEPENDS
  ${CMAKE_SOURCE_DIR}/server/inc/*.h
)

add_executable(${SERVER_NAME} ${SERVER_SOURCES} ${SERVER_HEADERS} main.cpp)
target_include_directories(${SERVER_NAME}
  PRIVATE third-party/libsodium/include ${CMAKE_SOURCE_DIR}/server/inc
)
find_library(LIBSODIUM
  NAMES sodium
  HINTS "${CMAKE_SOURCE_DIR}/third-party/libsodium/lib"
)

find_package(Qt6 REQUIRED COMPONENTS Core Network WebSockets Sql)
qt_standard_project_setup(REQUIRES 6.8)

target_link_libraries(${SERVER_NAME}
  PRIVATE Qt6::Core Qt6::Network Qt6::WebSockets Qt6::Sql "${LIBSODIUM}"
)

target_compile_options(${SERVER_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
)
#                             #
###############################

# Generating package with version
# execute_process(
#   COMMAND git describe --abbrev=0 --tags
#   OUTPUT_VARIABLE tag
# )

# string(LENGTH ${tag} tag_len)
# math(EXPR new_tag_len "${tag_len} - 1")
# string(SUBSTRING "${tag}" 0 ${new_tag_len} clear_tag)
# string(REPLACE "." ";" list_tag "${clear_tag}")
# list(GET list_tag 0 MAJOR_VERSION)
# list(GET list_tag 1 MINOR_VERSION)
# list(GET list_tag 2 PATCH_VERSION)

# set(CPACK_PACKAGE_VERSION_MAJOR "${MAJOR_VERSION}")
# set(CPACK_PACKAGE_VERSION_MINOR "${MINOR_VERSION}")
# set(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")
# set(CPACK_PACKAGE_VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}")
# set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
# set(CPACK_PACKAGE_CONTACT elochkaan@gmail.com)
# set(CPACK_PACKAGE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# if (MSVC)
#     set(CPACK_GENERATOR ZIP)
# else ()
#     set(CPACK_GENERATOR DEB)
# endif()

# include(CPack)