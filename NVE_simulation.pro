#-------------------------------------------------
#
# Project created by QtCreator 2013-05-28T14:26:19
#
#-------------------------------------------------

QT       += core gui

TARGET = NVE_simulation
TEMPLATE = app


SOURCES += GUI/main.cpp\
        GUI/mainwindow.cpp \
    GUI/ClientWidget.cpp \
    GUI/StreamWidget.cpp \
    GUI/MessageTemplate.cpp \
    GUI/DistributionWidget.cpp \
    GUI/XmlSerializer.cpp \
    nve_simulator.cc \
    Messages.cpp \
    ApplicationProtocol.cpp \
    DataGenerator.cpp \
    DataSender.cpp \
    RScriptGenerator.cpp \
    Server.cpp \
    StatisticCollector.cpp \
    XML_Parser.cpp

HEADERS  += GUI/mainwindow.h \
    GUI/ClientWidget.h \
    GUI/StreamWidget.h \
    GUI/MessageTemplate.h \
    GUI/DistributionWidget.h \
    GUI/XmlSerializer.h \
    ApplicationProtocol.h \
    XML_parser.h \
    utilities.h \
    StatisticsCollector.h \
    Server.h \
    RScriptGenerator.h \
    Messages.h \
    DataSender.h \
    DataGenerator.h \
    Client.h

FORMS    += GUI/mainwindow.ui

OTHER_FILES += \
    .gitignore

INCLUDEPATH += ../build
