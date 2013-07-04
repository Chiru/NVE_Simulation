#-------------------------------------------------
#
# Project created by QtCreator 2013-05-28T14:26:19
#
#-------------------------------------------------

QT       += core gui

TARGET = NVE_simulation
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    ClientWidget.cpp \
    StreamWidget.cpp \
    MessageTemplate.cpp \
    DistributionWidget.cpp \
    XmlSerializer.cpp

HEADERS  += mainwindow.h \
    ClientWidget.h \
    StreamWidget.h \
    MessageTemplate.h \
    DistributionWidget.h \
    XmlSerializer.h

FORMS    += mainwindow.ui
