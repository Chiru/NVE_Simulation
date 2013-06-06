#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGroupBox>
#include <QFrame>
#include <QStack>
#include <QScrollArea>

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
    bool configMessageFromEditor(MessageTemplate* const msg);
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

    void addClientWidgetToScrollArea();
    void enableMessageEditor(bool enabled);


public slots:
    void addClient();
    void removeClient();
    void addStream();
    void removeStream();
    void setMessage(const MessageTemplate* const msg, const StreamWidget* caller);
    void finishEditor();
};


#endif // MAINWINDOW_H
