QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    converter.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    converter.h \
    mainwindow.h

FORMS += \
    mainwindow.ui


win32:CONFIG(release, debug|release): LIBS += -L'C:/Users/manbi/CPP Libraries/ziplib/bin/' -llibzip
else:win32:CONFIG(debug, debug|release): LIBS += -L'C:/Users/manbi/CPP Libraries/ziplib/bin/' -llibzip
else:unix: LIBS += -L'C:/Users/manbi/CPP Libraries/ziplib/bin/' -llibzip

INCLUDEPATH += 'C:/Users/manbi/CPP Libraries/ziplib/include'
DEPENDPATH += 'C:/Users/manbi/CPP Libraries/ziplib/include'
