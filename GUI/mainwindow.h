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

    void enableMessageEditor(bool enabled);
    bool loadConfigurationFile(QString fileName);

public slots:
    void finishEditor();

private slots:
    void addClientWidgetToScrollArea();
    void removeClient();
    void addStream();
    void removeStream();
    void setMessage(const MessageTemplate* msg, StreamWidget* caller);
    void configurationFinished();
};


#endif // MAINWINDOW_H
