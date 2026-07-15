######## SERVER CONFIG ########
#                             #
set(SERVER_NAME server)

file(GLOB SERVER_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)
file(GLOB SERVER_HEADERS CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h
)

add_executable(${SERVER_NAME} ${SERVER_SOURCES} ${SERVER_HEADERS} server.cpp)
target_include_directories(${SERVER_NAME}
  PRIVATE ${CMAKE_SOURCE_DIR}/third-party/libsodium/include ${CMAKE_CURRENT_SOURCE_DIR}/inc
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

target_compile_definitions(${SERVER_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:SODIUM_STATIC SODIUM_EXPORT>
)

target_compile_options(${SERVER_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

if (UNIX AND NOT APPLE)
  set_target_properties(${SERVER_NAME} PROPERTIES
    INSTALL_RPATH "$ORIGIN"
    BUILD_WITH_INSTALL_RPATH TRUE
  )
endif()
#                             #
###############################

########## PACKAGING ##########
#                             #
install(TARGETS ${SERVER_NAME}
  RUNTIME DESTINATION .
  LIBRARY DESTINATION .
)

qt_generate_deploy_app_script(
  TARGET ${SERVER_NAME}
  OUTPUT_SCRIPT deploy_script
  NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
#                             #
###############################