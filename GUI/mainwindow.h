#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGroupBox>
#include <QFrame>
#include <QStack>

class ClientWidget;
class StreamWidget;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    int numberOfClients;
    int numberOfStreams;
    QStack<ClientWidget*> previousClients;
    QStack<QFrame*> previousClientsLines;
    QStack<StreamWidget*> previousStreams;
    QStack<QFrame*> previousStreamsLines;

    void addClientWidgetToScrollArea();


public slots:
    void addClient();
    void removeClient();
    void addStream();
    void removeStream();
};


#endif // MAINWINDOW_H
