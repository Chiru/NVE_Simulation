#ifndef LISTITEM_H
#define LISTITEM_H

#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QListWidget>
#include <QDialogButtonBox>
#include <QPushButton>
#include "MessageTemplate.h"
#include "mainwindow.h"

class StreamWidget : public QGroupBox
{
    Q_OBJECT

public:
    StreamWidget(int number, QWidget* parent = 0);

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
    QGroupBox* messageEditor;

public slots:
    void openMessageEditor();
    void removeMessageFromList();
    void newMessageAdded();
    void editorClosed();

signals:
    void setupMessageEditor(const MessageTemplate* const msg, const StreamWidget*);

};

#endif // LISTITEM_H
