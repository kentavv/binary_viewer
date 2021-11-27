cmake_minimum_required(VERSION 3.11)
project(binary_viewer CXX)

set(CMAKE_CXX_STANDARD 14)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Instruct CMake to include project Qt resources as needed
set(CMAKE_AUTORCC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

include_directories(.)
if(MSVC)
        # Similar to -Wall -Wextra
        add_compile_options(/W3 /WX)

        # Type conversion warnings
        add_compile_options("/wd4244") # type conversion
        add_compile_options("/wd4305") # type truncation
        add_compile_options("/wd4267") # type conversion, loss of data

elseif(CMAKE_COMPILER_IS_GNUCXX)
        add_compile_options(-Wall -Wextra -Wno-sign-compare)
endif(MSVC)

add_executable(binary_viewer
        bayer.cpp
        bayer.h
        binary_viewer.cpp
        binary_viewer.h
        dot_plot.cpp
        dot_plot.h
        plot_view.cpp
        plot_view.h
        hilbert.cpp
        hilbert.h
        histogram_calc.cpp
        histogram_calc.h
        overall_view.cpp
        overall_view.h
        histogram_2d_view.cpp
        histogram_2d_view.h
        image_view.cpp
        image_view.h
        main.cpp
        main_app.cpp
        main_app.h
        version.cpp
        version.h
        histogram_3d_view.cpp
        histogram_3d_view.h
        bin_viewer.qrc)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui OpenGL)
target_link_libraries(binary_viewer Qt5::Core Qt5::Widgets Qt5::Gui Qt5::OpenGL)

if(MSVC)
        # On Windows the dependency is provided by the glui package
        find_package(glui CONFIG REQUIRED)
        target_link_libraries(binary_viewer glui::glui glui::glui_static)
else()
        target_link_libraries(binary_viewer GL GLU)
endif(MSVC)
