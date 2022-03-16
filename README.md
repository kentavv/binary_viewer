Binary Viewer is a tool for binary file discovery using visualizations that may highlight patterns.

See the project [Wiki](https://github.com/kentavv/binary_viewer/wiki) for some examples.

![Screenshot](https://raw.githubusercontent.com/wiki/kentavv/binary_viewer/images/view_3d_hist.png)

Loosely based on Cantor.Dust, Binary Viewer was developed after seeing a demo of Cantor.Dust but receiving no response regarding availability.

Since Cantor.Dust was demoed, other tools with have similar functionality became available.

https://github.com/devttys0/binwalk/wiki/Quick-Start-Guide

https://sites.google.com/site/xxcantorxdustxx/home

https://github.com/wapiflapi/binglide

https://github.com/codilime/veles

https://github.com/radareorg/radare2


The beginnings of Cantor.Dust was Greg Conti's work
https://github.com/rebelbot/binvis

https://media.blackhat.com/bh-us-10/whitepapers/Bratus_Conti/BlackHat-USA-2010-Bratus-Conti-Taxonomy-wp.pdf


Even earlier are dotplots for RE'ing, here Dan Kaminsky's Blackops talk

https://www.slideshare.net/dakami/dmk-blackops2006

For more information on this and related programs for visualizing binaries see
https://www.youtube.com/watch?v=C8--cXwuuFQ&list=PLUyyOw61zxiJXMihb4PjYbGHEgdGxMuY3

Qt5 is required to compile Binary Viewer.
QDarkStyleSheet (MIT License, https://github.com/ColinDuquesnoy/QDarkStyleSheet/) provides the Qt dark theme.

## Building on Windows

The easiest way to get binary_viewer to build on Windows is with CMake and [vcpkg](https://vcpkg.io/en/getting-started.html). With vcpkg you can install the dependencies in way that CMake can find then automatically. For example, if you're targeting Windows 64-bit this is the command you would use to install your dependencies:

    vcpkg --triplet x64-windows install "@.vcpkg_requirements.txt"

QT5 takes some time to install. Once it is done, you can build using this command (update with your path to vcpkg.cmake):

    mkdir build
    pushd build
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=X:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
    cmake --build .

Kent A. Vander Velden
kent.vandervelden@gmail.com
