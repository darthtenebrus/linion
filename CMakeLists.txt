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
set(RES_FILES images.qrc QvObjectDelegate.cpp QvObjectDelegate.h)

add_executable(${PROJECT_NAME}
        main.cpp
        mainwindow.h
        mainwindow.cpp
        mainwindow.ui QAddonListModel.cpp QAddonListModel.h ItemData.cpp ItemData.h
        ${RES_FILES})

target_link_libraries(${PROJECT_NAME} Qt5::Core)
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)
