CMAKE_MINIMUM_REQUIRED(VERSION 3.13)

project(switcher LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)

# include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)  # Includes the contents of the conanbuildinfo.cmake file.
# conan_basic_setup()  # Prepares the CMakeList.txt for Conan.

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

file(GLOB PROJ_HDRS *.h)
file(GLOB PROJ_SRCS *.cpp)
file(GLOB PROJ_RES *.qrc)
file(GLOB PROJ_UI *.ui)

add_executable(switcher ${PROJ_HDRS} ${PROJ_SRCS} ${PROJ_RES} ${PROJ_UI})

# target_link_libraries(switcher Qt5::Core Qt5::Gui Qt5::Widgets ${CONAN_LIBS})
target_link_libraries(switcher Qt5::Core Qt5::Gui Qt5::Widgets)
target_link_options(switcher PRIVATE $<$<CONFIG:Debug>:/SUBSYSTEM:CONSOLE>)
target_link_options(switcher PRIVATE $<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>)
target_link_options(switcher PRIVATE $<$<CONFIG:Release>:/ENTRY:mainCRTStartup>)
