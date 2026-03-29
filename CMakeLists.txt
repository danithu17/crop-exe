cmake_minimum_required(VERSION 3.16)
project(FlashPrint-Enhancer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt 6
find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Core)
# Find OpenCV
find_package(OpenCV REQUIRED)

add_executable(FlashPrint-Enhancer
    main.cpp
    MainWindow.cpp
    MainWindow.h
)

target_link_libraries(FlashPrint-Enhancer PRIVATE 
    Qt6::Widgets 
    Qt6::Gui 
    Qt6::Core 
    ${OpenCV_LIBS}
)

set_target_properties(FlashPrint-Enhancer PROPERTIES
    WIN32_EXECUTABLE ON
)