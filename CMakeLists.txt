cmake_minimum_required(VERSION 3.28)
project(sigma LANGUAGES CXX)

# Add files and deps

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