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
    NVE_model/nve_simulator.cc \
    NVE_model/Messages.cpp \
    NVE_model/ApplicationProtocol.cpp \
    NVE_model/DataGenerator.cpp \
    NVE_model/DataSender.cpp \
    NVE_model/RScriptGenerator.cpp \
    NVE_model/Server.cpp \
    NVE_model/StatisticCollector.cpp \
    NVE_model/XML_Parser.cpp \
    NVE_model/Client.cpp \


HEADERS += GUI/mainwindow.h \
    GUI/ClientWidget.h \
    GUI/StreamWidget.h \
    GUI/MessageTemplate.h \
    GUI/DistributionWidget.h \
    GUI/XmlSerializer.h \
    NVE_model/ApplicationProtocol.h \
    NVE_model/XML_parser.h \
    NVE_model/utilities.h \
    NVE_model/StatisticsCollector.h \
    NVE_model/Server.h \
    NVE_model/RScriptGenerator.h \
    NVE_model/Messages.h \
    NVE_model/DataSender.h \
    NVE_model/DataGenerator.h \
    NVE_model/Client.h \
    NVE_model/simulation_interface.h

FORMS    += GUI/mainwindow.ui

OTHER_FILES += \
    .gitignore \
    licence.txt \
    README.txt \
    build.sh

INCLUDEPATH += NVE_model \
    NS3/build/ \

LIBS +=  -L./NS3/build/ \
    -lns3.17-core-optimized \
    -lns3.17-internet-optimized \
    -lns3.17-network-optimized \
    -lns3.17-flow-monitor-optimized \
    -lns3.17-point-to-point-optimized \
    -lns3.17-mpi-optimized \
    -lns3.17-bridge-optimized \



QMAKE_RPATHDIR += ./NS3/build/

