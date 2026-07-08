cmake_minimum_required(VERSION 3.28)
project(SigmaMessenger LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Qml Quick)

qt_standard_project_setup(REQUIRES 6.8)

set(ASSETS_FILES
    "assets/logo.png"
    "assets/mic.png"
    "assets/video-camera.png"
    "assets/arrow down.png"
)

qt_add_executable(SigmaMessenger
    main.cpp
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
    RESOURCES ${ASSETS_FILES}

)

target_link_libraries(SigmaMessenger PRIVATE Qt6::Gui Qt6::Qml Qt6::Quick)

install(TARGETS SigmaMessenger
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
