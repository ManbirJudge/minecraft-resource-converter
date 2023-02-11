QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    converter.cpp \
    main.cpp \
    mainwindow.cpp \
    settings_dialog.cpp

HEADERS += \
    converter.h \
    mainwindow.h \
    settings_dialog.h

FORMS += \
    mainwindow.ui \
    settings_dialog.ui

LIBS += -L"C:\Users\manbi\Libraries\zlib\lib" -lzlib

INCLUDEPATH += "C:\Users\manbi\Libraries\zlib\include"
DEPENDPATH += "C:\Users\manbi\Libraries\zlib\include"

LIBS += -L"C:\Users\manbi\Libraries\libzip\lib" -lzip

INCLUDEPATH += "C:\Users\manbi\Libraries\libzip\include"
DEPENDPATH += "C:\Users\manbi\Libraries\libzip\include"

RESOURCES += \
    assets.qrc \
    identity_maps.qrc
