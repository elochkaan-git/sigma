######## CLIENT CONFIG ########
#                             #
set(CLIENT_NAME client)

file(GLOB SERVER_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)
file(GLOB SERVER_HEADERS CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h
)

add_executable(${CLIENT_NAME} ${SERVER_SOURCES} ${SERVER_HEADERS} client.cpp)
target_include_directories(${CLIENT_NAME}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

find_package(Qt6 REQUIRED COMPONENTS Core Network WebSockets)
qt_standard_project_setup(REQUIRES 6.8)

target_link_libraries(${CLIENT_NAME}
  PRIVATE Qt6::Core Qt6::Network Qt6::WebSockets
)

target_compile_options(${CLIENT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
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