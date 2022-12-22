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
file(GLOB PROJ_UI *.ui)

if(UNIX)
	set(CMAKE_SKIP_BUILD_RPATH FALSE)
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	set(CMAKE_INSTALL_RPATH "$ORIGIN/:$ORIGIN/lib/:./:./lib/")
	set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
endif()

add_executable(${PROJECT_NAME} ${PROJ_HDRS} ${PROJ_SRCS} ${PROJ_RES} ${PROJ_UI})

target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Gui Qt5::Widgets)

if(WIN32)
	target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:/SUBSYSTEM:CONSOLE>)
	target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>)
	target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:/ENTRY:mainCRTStartup>)
endif()

configure_file(${PROJECT_SOURCE_DIR}/deploy_linux.sh.in ${PROJECT_SOURCE_DIR}/deploy_linux.sh @ONLY)
