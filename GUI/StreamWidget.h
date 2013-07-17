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

class MainWindow;

class StreamWidget : public QGroupBox
{
    Q_OBJECT

    friend class XmlSerializer;

public:
    StreamWidget(int number, MainWindow* mw,QWidget* parent = 0);
    StreamWidget(int number, MainWindow* mw, bool tcpUsed, bool appProto, bool ordered, bool nagle, int serverGameTick, int clientGameTick, QList<MessageTemplate*> messages,
                 QWidget* parent = 0);
    ~StreamWidget();
    bool isAppProtoEnabled() const{return appProto->isChecked();}
    bool tcpUsed() const {return tcp->isChecked();}
    void enableStreamWidgets(bool enabled);
    void configureStream();

    static const QList<QString>& getMessageNames();
    static void addMessageName(const QString&);


private:

    QList<MessageTemplate*> messages;
    MessageTemplate* messageInEditor;
    QString previousMessageName;
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
    QPushButton* editMessage;
    QLabel* protocolLabel;
    QLabel* clientGameTickLabel;
    QLabel* serverGameTickLabel;
    bool editButtonEnabled;
    bool removeButtonEnabled;
    MainWindow* mw;

    void deleteMessageFromList(const QString& messageName);

    static QList<QString> messageNames; //messages must have unique names, even in different streams

public slots:
    void addNewMessage();
    void editExistingMessage();
    void removeMessageFromList();
    void newMessageAdded();
    void editorClosed();
    void rowFocusChanged(int);

signals:
    void setupMessageEditor(const MessageTemplate* msg, StreamWidget*);

};

#endif // LISTITEM_H
