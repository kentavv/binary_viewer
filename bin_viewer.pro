QT += xml opengl
TEMPLATE = app
TARGET = binary_viewer
DEPENDPATH += .
INCLUDEPATH += .

QMAKE_CXXFLAGS +=

QMAKE_LFLAGS += -lGLU -lglut

# Input
HEADERS += \
main_app.h version.h histogram.h image_view.h graph_view.h view_3d.h image_view2.h image_view3.h dot_plot.h bayer.h

SOURCES += \
main.cpp  main_app.cpp histogram.cpp image_view.cpp graph_view.cpp view_3d.cpp image_view2.cpp image_view3.cpp dot_plot.cpp bayer.cpp

