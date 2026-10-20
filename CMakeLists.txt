cmake_minimum_required(VERSION 3.21)
project(LiveWallpaper VERSION 0.5.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.5 REQUIRED COMPONENTS Widgets Network Multimedia MultimediaWidgets)
qt_standard_project_setup()

qt_add_executable(LiveWallpaper
    src/main.cpp
    src/PlayerWindow.cpp
    src/PlayerWindow.h
    src/DesktopHost.cpp
    src/DesktopHost.h
    src/FocusClock.h
    src/ProductivityModel.h
    src/ProductivityModel.cpp
    src/ProductivityPanel.h
    src/ProductivityPanel.cpp
    src/CalmTheme.h
    src/CalmTheme.cpp
    src/ImportSupport.h
    src/ImportSupport.cpp
    src/ImportJob.h
    src/ImportJob.cpp
    src/YouTubeSetup.h
    src/YouTubeSetup.cpp
)
target_link_libraries(LiveWallpaper PRIVATE
    Qt6::Widgets Qt6::Network Qt6::Multimedia Qt6::MultimediaWidgets user32
)
set_target_properties(LiveWallpaper PROPERTIES WIN32_EXECUTABLE TRUE)

if(MSVC)
    target_compile_options(LiveWallpaper PRIVATE /W4 /permissive-)
else()
    target_compile_options(LiveWallpaper PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
if(BUILD_TESTING)
    qt_add_executable(ProductivityTests
        tests/ProductivityTests.cpp src/ProductivityModel.cpp src/ProductivityModel.h src/FocusClock.h)
    target_include_directories(ProductivityTests PRIVATE src)
    target_link_libraries(ProductivityTests PRIVATE Qt6::Core)
    add_test(NAME ProductivityTests COMMAND ProductivityTests)
    qt_add_executable(ImportTests tests/ImportTests.cpp src/ImportSupport.cpp src/ImportSupport.h)
    target_include_directories(ImportTests PRIVATE src)
    target_link_libraries(ImportTests PRIVATE Qt6::Core)
    add_test(NAME ImportTests COMMAND ImportTests)
endif()
