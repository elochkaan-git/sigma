<<<<<<< HEAD
cmake_minimum_required(VERSION 3.25)
project(sigma LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)

add_subdirectory(server)
add_subdirectory(client)

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
=======
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
    "assets/servers.csv"
    "assets/person.png"
)

qt_add_executable(SigmaMessenger
    main.cpp
    include/client_controller.hpp
    src/client_controller.cpp
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
    RESOURCES ${ASSETS_FILES}

)
target_include_directories(SigmaMessenger PUBLIC include)

target_link_libraries(SigmaMessenger PRIVATE Qt6::Gui Qt6::Qml Qt6::Quick)

install(TARGETS SigmaMessenger
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
>>>>>>> origin/client
