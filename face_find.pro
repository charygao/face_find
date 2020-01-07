#-------------------------------------------------
#
# Project created by QtCreator 2019-12-27T11:07:49
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = face_find
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += BOOST_NO_AUTO_PTR

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        main.cpp \
        MainWindow.cpp \
    media_decoder.cpp \
    utility_tool.cpp \
    log.cpp \
    face_detection.cpp

HEADERS += \
        MainWindow.h \
    media_decoder.h \
    utility_tool.h \
    log.h \
    face_detection.h

FORMS += \
        MainWindow.ui


INCLUDEPATH += D:/qt_files/include
LIBS += -LD:/qt_files/lib -lavformat -lavdevice -lavcodec  -lavutil -lswresample -lavfilter -lpostproc -lswscale
LIBS += -LD:/qt_files/lib -lopencv_world420d

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
