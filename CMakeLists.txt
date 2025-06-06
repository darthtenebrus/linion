cmake_minimum_required(VERSION 3.17)
project(linion)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)


if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-D_DEBUG)
endif()

find_package(Qt5 COMPONENTS Core Widgets Network LinguistTools)
find_package(X11)
set(LING_OPTIONS -no-obsolete)
qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} mainwindow_ru.ts OPTIONS ${LING_OPTIONS})
qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} mainwindow_de.ts OPTIONS ${LING_OPTIONS})
configure_file(translations.qrc ${CMAKE_BINARY_DIR} COPYONLY)
set(RES_FILES ${CMAKE_BINARY_DIR}/translations.qrc images.qrc)

add_executable(${PROJECT_NAME}
        main.cpp
        mainwindow.h
        mainwindow.cpp
        mainwindow.ui QAddonListModel.cpp QAddonListModel.h ItemData.cpp ItemData.h
        QvObjectDelegate.cpp QvObjectDelegate.h configdialog.cpp configdialog.h configdialog.ui
        DialogItemDelegate.cpp DialogItemDelegate.h preferences.h QAddonProxyModel.cpp QAddonProxyModel.h
        BinaryDownloader.cpp BinaryDownloader.h aboutdialog.cpp aboutdialog.h aboutdialog.ui version.h
        terminalwindow.cpp terminalwindow.h terminalwindow.ui
        ${RES_FILES}
        ${QM_FILES})

target_link_libraries(${PROJECT_NAME}
        Qt5::Core
        Qt5::Widgets
        Qt5::Network
        X11)

install(TARGETS ${PROJECT_NAME})
