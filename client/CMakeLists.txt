file(GLOB CLIENT_SOURCES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
  ${CMAKE_SOURCE_DIR}/common/src/*.cpp
)
file(GLOB CLIENT_HEADERS CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
  ${CMAKE_SOURCE_DIR}/common/inc/*.h
)

# add_library(libdatachannel SHARED IMPORTED)
# set_target_properties(libdatachannel PROPERTIES
#     IMPORTED_LOCATION "/home/savely/Downloads/libdatachannel/libdatachannel.so"
# )
# target_include_directories(libdatachannel INTERFACE
#     "/home/savely/Downloads/libdatachannel/include"
# )

find_package(Qt6 REQUIRED COMPONENTS Gui Qml Quick Core Network WebSockets Multimedia Sql)
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
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(libdatachannel)

find_package(PkgConfig REQUIRED)
pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET
  libavcodec
  libavutil
  libswscale
  libswresample
)

set(ASSETS_FILES
    "assets/logo.png"
    "assets/mic.png"
    "assets/video-camera.png"
    "assets/arrow down.png"
    "assets/servers.csv"
    "assets/person.png"
)

qt_add_executable(SigmaMessenger
    main.cpp
    ${CLIENT_SOURCES}
    ${CLIENT_HEADERS}
)

qt_add_qml_module(SigmaMessenger
    URI Main
    VERSION 1.0
    RESOURCE_PREFIX "/"
    QML_FILES 
        qml/Server_choice.qml
        qml/Login_server.qml
        qml/Register_server.qml
        qml/Style.qml
        qml/ServerButton.qml
        qml/Main.qml
        qml/General_window.qml
        qml/Toast.qml
        qml/ChatScreen.qml
        qml/FriendsScreen.qml
        qml/CallWindow.qml
        qml/IconButton.qml
    RESOURCES ${ASSETS_FILES}

)
target_include_directories(SigmaMessenger PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/common/inc
  ${FFMPEG_INCLUDE_DIRS}
)

target_link_libraries(SigmaMessenger PRIVATE 
    Qt6::Gui 
    Qt6::Qml 
    Qt6::Quick
    Qt6::Core
    Qt6::Network
    Qt6::WebSockets
    Qt6::Multimedia
    LibDataChannel::LibDataChannel
    Qt6::Sql

    # libdatachannel
    PkgConfig::FFMPEG
)
target_compile_options(SigmaMessenger PRIVATE
  $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra>
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
  ${GST_CFLAGS_OTHER}
)
if (UNIX AND NOT APPLE)
    set_target_properties(SigmaMessenger PROPERTIES
        INSTALL_RPATH "$ORIGIN"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()

install(TARGETS SigmaMessenger
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET SigmaMessenger
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(SCRIPT ${deploy_script})
