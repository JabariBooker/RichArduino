#-------------------------------------------------
#
# Project created by QtCreator 2019-04-12T02:54:12
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = test
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11
CONFIG += static

SOURCES += \
        main.cpp \
    Assembler.cpp \
    main.cpp \
    USB.cpp \
    RichArduino.cpp

HEADERS += \
    Assembler.h \
    USB.h \
    RichArduino.h \
    FTDI/ftd2xx.h

FORMS += \
    RichArduino.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target


win32: LIBS += -L$$PWD/FTDI/Static/i386/ -lftd2xx

INCLUDEPATH += $$PWD/FTDI/Static/i386
DEPENDPATH += $$PWD/FTDI/Static/i386

win32:!win32-g++: PRE_TARGETDEPS += $$PWD/FTDI/Static/i386/ftd2xx.lib
else:win32-g++: PRE_TARGETDEPS += $$PWD/FTDI/Static/i386/libftd2xx.a
