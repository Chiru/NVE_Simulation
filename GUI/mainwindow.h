#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGroupBox>
#include <QFrame>
#include <QStack>

class ClientWidget;

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
    QStack<ClientWidget*> previousClients;
    QStack<QFrame*> previousLines;

    void addClientWidgetToScrollArea();



public slots:
    void addClient();
    void removeClient();
};


#endif // MAINWINDOW_H
