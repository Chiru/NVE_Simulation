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
    XmlSerializer.cpp \
    ../nve_simulator.cc

HEADERS  += mainwindow.h \
    ClientWidget.h \
    StreamWidget.h \
    MessageTemplate.h \
    DistributionWidget.h \
    XmlSerializer.h \
    ../XML_parser.h \
    ../utilities.h \
    ../StatisticsCollector.h \
    ../Server.h \
    ../RScriptGenerator.h \
    ../Messages.h \
    ../DataSender.h \
    ../DataGenerator.h \
    ../Client.h \
    ../ApplicationProtocol.h

FORMS    += mainwindow.ui

OTHER_FILES += \
    ../NVE_simulation.pro \
    ../NVE.includes \
    ../NVE.files \
    ../NVE.creator.user.2.3pre1 \
    ../NVE.creator.user \
    ../NVE.creator \
    ../NVE.config
