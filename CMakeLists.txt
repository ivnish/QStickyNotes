cmake_minimum_required(VERSION 3.5)

project(QStickyNotes VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Test)

set(PROJECT_SOURCES
    src/main.cpp
    src/traymanager.h
    src/traymanager.cpp
    src/settingsdialog.h
    src/settingsdialog.cpp
    src/settings.h
    src/settings.cpp
    src/stickynotewidget.h
    src/stickynotewidget.cpp
)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(QStickyNotes
        ${PROJECT_SOURCES}
    )
else()
    add_executable(QStickyNotes
        ${PROJECT_SOURCES}
    )
endif()

target_link_libraries(QStickyNotes PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

include(GNUInstallDirs)
install(TARGETS QStickyNotes
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Custom uninstall command

configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY
)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
)

# CMake config to program

set(GENERATED_INCLUDE_DIR
    ${CMAKE_CURRENT_BINARY_DIR}/generated
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/config.h
    @ONLY
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/QStickyNotes.desktop.in
    ${CMAKE_CURRENT_BINARY_DIR}/QStickyNotes.desktop
    @ONLY
)

target_include_directories(QStickyNotes PRIVATE
    ${GENERATED_INCLUDE_DIR}
)

# DEB package

install(DIRECTORY Icons/
        DESTINATION share/QStickyNotes/Icons)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/QStickyNotes.desktop
        DESTINATION share/applications)

install(DIRECTORY translations/
        DESTINATION share/QStickyNotes/translations
        FILES_MATCHING PATTERN "*.qm")

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_CONTACT "ivnish.dev@gmail.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Lightweight sticky notes application")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION
"QStickyNotes is a lightweight sticky notes application inspired by indicator-stickynotes.
It provides a simple and fast way to create and manage notes directly from the system tray.
Notes are automatically saved and restored on startup.")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/ivnish/QStickyNotes")
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Andrei 'ivnish' Ivnitskii")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt${QT_VERSION_MAJOR}gui${QT_VERSION_MAJOR}t64")
include(CPack)

# Testing

if(DEFINED CMAKE_BUILD_TYPE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    enable_testing()

    add_executable(tests
        tests/main.cpp
        tests/test_traymanager.h
        tests/test_traymanager.cpp
        src/traymanager.cpp
        src/stickynotewidget.cpp
        src/settingsdialog.cpp
        src/settings.cpp
    )

    target_link_libraries(tests
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Test
    )

    target_include_directories(tests PRIVATE
        ${GENERATED_INCLUDE_DIR}
    )

    add_test(NAME QStickyNotesTests COMMAND tests)
endif()
