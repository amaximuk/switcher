CMAKE_MINIMUM_REQUIRED(VERSION 3.13)

project(switcher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

file(GLOB PROJ_HDRS *.h)
file(GLOB PROJ_SRCS *.cpp)
file(GLOB PROJ_RES *.qrc)

add_executable(switcher ${PROJ_HDRS} ${PROJ_SRCS} ${PROJ_RES})

target_link_libraries(switcher Qt5::Core Qt5::Gui Qt5::Widgets)