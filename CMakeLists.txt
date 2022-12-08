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
set(LING_OPTIONS -no-obsolete)
qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} mainwindow_ru.ts OPTIONS ${LING_OPTIONS})
configure_file(translations.qrc ${CMAKE_BINARY_DIR} COPYONLY)
set(RES_FILES ${CMAKE_BINARY_DIR}/translations.qrc images.qrc)

add_executable(${PROJECT_NAME}
        main.cpp
        mainwindow.h
        mainwindow.cpp
        mainwindow.ui QAddonListModel.cpp QAddonListModel.h ItemData.cpp ItemData.h
        QvObjectDelegate.cpp QvObjectDelegate.h configdialog.cpp configdialog.h configdialog.ui
        DialogItemDelegate.cpp DialogItemDelegate.h
        ${RES_FILES}
        ${QM_FILES} preferences.h QAddonProxyModel.cpp QAddonProxyModel.h)

target_link_libraries(${PROJECT_NAME} Qt5::Core)
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)
target_link_libraries(${PROJECT_NAME} Qt5::Network)
