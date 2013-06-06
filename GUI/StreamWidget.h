#ifndef LISTITEM_H
#define LISTITEM_H

#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QLabel>
#include "MessageTemplate.h"
#include "mainwindow.h"

class StreamWidget : public QGroupBox
{
    Q_OBJECT

public:
    StreamWidget(int number, MainWindow* mw,QWidget* parent = 0);
    bool isAppProtoEnabled() const{return appProto->isChecked();}
    bool tcpUsed() const {return tcp->isChecked();}
    void enableStreamWidgets(bool enabled);

private:

    QList<MessageTemplate*> messages;
    MessageTemplate* messageInEditor;
    int streamNumber;
    QRadioButton* tcp;
    QRadioButton* udp;
    QCheckBox* nagle;
    QCheckBox* appProto;
    QCheckBox* ordered;
    QSpinBox* clientGameTick;
    QSpinBox* serverGameTick;
    QListWidget* messageList;
    QPushButton* addMessage;
    QPushButton* removeMessage;
    QLabel* protocolLabel;
    QLabel* clientGameTickLabel;
    QLabel* serverGameTickLabel;

    MainWindow* mw;

    static QList<QString> messageNames; //messages must have unique names, even in different streams

public slots:
    void openMessageEditor();
    void removeMessageFromList();
    void newMessageAdded();
    void editorClosed();

signals:
    void setupMessageEditor(const MessageTemplate* const msg, StreamWidget*);

};

#endif // LISTITEM_H
