cmake_minimum_required(VERSION 3.11)
project(binary_viewer CXX)

set(CMAKE_CXX_STANDARD 14)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

include_directories(.)
add_compile_options(-Wall -Wextra -Wno-sign-compare)

add_executable(binary_viewer
        bayer.cpp
        bayer.h
        dot_plot.cpp
        dot_plot.h
        graph_view.cpp
        graph_view.h
        histogram.cpp
        histogram.h
        image_view.cpp
        image_view.h
        image_view2.cpp
        image_view2.h
        image_view3.cpp
        image_view3.h
        main.cpp
        main_app.cpp
        main_app.h
        version.cpp
        version.h
        view_3d.cpp
        view_3d.h)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui OpenGL)
target_link_libraries(binary_viewer Qt5::Core Qt5::Widgets Qt5::Gui Qt5::OpenGL)

target_link_libraries(binary_viewer GL GLU)
