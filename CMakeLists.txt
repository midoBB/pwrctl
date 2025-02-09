cmake_minimum_required(VERSION 3.16)
project(pwrctl VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets DBus)

qt_add_executable(pwrctl
    main.hpp main.cpp powerprofile.hpp powerprofile.cpp
)

qt_add_resources(pwrctl "resources"
    PREFIX "/"
    BASE "assets"
    FILES
        assets/icon.png
        assets/cable.png
        assets/battery.png
)

target_link_libraries(pwrctl PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::DBus
)

install(TARGETS pwrctl DESTINATION bin)
