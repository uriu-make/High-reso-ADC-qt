cmake_minimum_required(VERSION 3.2)

project(sample-app)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5Widgets)

add_executable(sample-app
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

# find_package(PkgConfig)
if(UNIX)
    include_directories(/usr/include/qwt)
    link_directories(/usr/lib/)
endif()

target_link_libraries(sample-app
    Qt5::Widgets
    qwt-qt5
)

set(EXECUTABLE_OUTPUT_PATH "../")
