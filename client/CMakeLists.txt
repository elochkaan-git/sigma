######## CLIENT CONFIG ########
#                             #
set(CLIENT_NAME client)

file(GLOB SERVER_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
  ${CMAKE_SOURCE_DIR}/common/src/*.cpp
)
file(GLOB SERVER_HEADERS CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h
  ${CMAKE_SOURCE_DIR}/common/inc/*.h
)

add_executable(${CLIENT_NAME} ${SERVER_SOURCES} ${SERVER_HEADERS} client.cpp)

find_package(Qt6 REQUIRED COMPONENTS Core Network WebSockets Test)
qt_standard_project_setup(REQUIRES 6.8)

include(FetchContent)
FetchContent_Declare(
  libdatachannel
  GIT_REPOSITORY https://github.com/paullouisageneau/libdatachannel.git
  GIT_TAG        v0.24.5
)
set(NO_WEBSOCKET ON CACHE BOOL "" FORCE)
set(NO_TESTS ON CACHE BOOL "" FORCE)
set(NO_EXAMPLES ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(libdatachannel)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED
  gstreamer-1.0
  gstreamer-app-1.0
  gstreamer-video-1.0
)

target_link_libraries(${CLIENT_NAME} PRIVATE
  Qt6::Core
  Qt6::Network
  Qt6::WebSockets
  datachannel
  ${GST_LIBRARIES}
)

target_include_directories(${CLIENT_NAME} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/inc
  ${CMAKE_SOURCE_DIR}/common/inc
  ${GST_INCLUDE_DIRS}
)

target_compile_options(${CLIENT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
  ${GST_CFLAGS_OTHER}
)

# if (UNIX AND NOT APPLE)
#   set_target_properties(${CLIENT_NAME} PROPERTIES
#     INSTALL_RPATH "$ORIGIN"
#     BUILD_WITH_INSTALL_RPATH TRUE
#   )
# endif()
#                             #
###############################

########## PACKAGING ##########
#                             #
# install(TARGETS ${CLIENT_NAME}
#   RUNTIME DESTINATION .
#   LIBRARY DESTINATION .
# )

# qt_generate_deploy_app_script(
#   TARGET ${CLIENT_NAME}
#   OUTPUT_SCRIPT deploy_script
#   NO_UNSUPPORTED_PLATFORM_ERROR
# )
# install(SCRIPT ${deploy_script})
#                             #
###############################