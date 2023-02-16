QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    converter.cpp \
    main.cpp \
    mainwindow.cpp \
    settings_dialog.cpp

HEADERS += \
    converter.h \
    mainwindow.h \
    settings_dialog.h \
    stb_image_write.h

FORMS += \
    mainwindow.ui \
    settings_dialog.ui

# including zlib
LIBS += -L"C:\Users\manbi\Libraries\zlib\lib" -lzlib

INCLUDEPATH += "C:\Users\manbi\Libraries\zlib\include"
DEPENDPATH += "C:\Users\manbi\Libraries\zlib\include"

# including libzip
LIBS += -L"C:\Users\manbi\Libraries\libzip\lib" -lzip

INCLUDEPATH += "C:\Users\manbi\Libraries\libzip\include"
DEPENDPATH += "C:\Users\manbi\Libraries\libzip\include"

# including opencv
LIBS += "C:\Users\manbi\Libraries\opencv\mingw\lib\libopencv_core460.dll.a"
LIBS += "C:\Users\manbi\Libraries\opencv\mingw\lib\libopencv_highgui460.dll.a"
LIBS += "C:\Users\manbi\Libraries\opencv\mingw\lib\libopencv_imgcodecs460.dll.a"
LIBS += "C:\Users\manbi\Libraries\opencv\mingw\lib\libopencv_imgproc460.dll.a"

INCLUDEPATH += "C:\Users\manbi\Libraries\opencv\include"
DEPENDPATH += "C:\Users\manbi\Libraries\opencv\include"


# including resources
RESOURCES += \
    assets.qrc \
    identity_maps.qrc
