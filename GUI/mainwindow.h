#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGroupBox>
#include <QFrame>
#include <QStack>
#include <QScrollArea>
#include "MessageTemplate.h"
#include "DistributionWidget.h"
#include "XmlSerializer.h"
#include "XML_parser.h"

class ClientWidget;
class StreamWidget;
class MessageTemplate;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    bool configMessageFromEditor(MessageTemplate* msg);
    void setMsgConfigErrorMessage(const QString& error);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    int numberOfClients;
    int numberOfStreams;
    QStack<ClientWidget*> previousClients;
    QStack<QFrame*> previousClientsLines;
    QStack<StreamWidget*> previousStreams;
    QStack<QFrame*> previousStreamsLines;
    QPalette* palette;
    DistributionWidget* messageSize;
    DistributionWidget* timeInterval;
    XmlSerializer serializer;
    XMLParser parser;
    QString fileName;
    QDialog* configuration;

    bool executeFileDialog();
    void enableMessageEditor(bool enabled);
    bool loadConfigurationFile(QString fileName);
    void configureSimulationParams(const std::string& element);
    void configureClients(const std::string& element);
    void configureClient(const std::string& element);
    void configureAppProto(const std::string& element);
    void configureStreams(const std::string& element);
    void configureStream(const std::string& element);
    void configureMessages(const std::string& element, bool appProtoEnabled, QList<MessageTemplate*>& messages);
    MessageTemplate *configureMessage(const std::string& element, bool appProtoEnabled);


public slots:
    void finishEditor();

private slots:
    void chooseConfigurationFile();
    void usePreviousConfiguration();
    void cancelConfigurationDialog();
    void addClientWidgetToScrollArea();
    void removeClient();
    void addStream();
    void removeStream();
    void setMessage(const MessageTemplate* msg, StreamWidget* caller);
    void configurationFinished();
};


#endif // MAINWINDOW_H
