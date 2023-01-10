cmake_minimum_required(VERSION 3.2)

project(High-reso-ADC)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_FLAGS "-Wall")

find_package(Qt5Widgets)
find_package(Qt5 REQUIRED COMPONENTS Network)

add_executable(High-reso-ADC
    ADS1256.h
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

target_link_libraries(High-reso-ADC
    Qt5::Widgets
    Qt5::Network
    qwt-qt5
)

set(EXECUTABLE_OUTPUT_PATH "../")
