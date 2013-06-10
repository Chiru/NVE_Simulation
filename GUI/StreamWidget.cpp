#include "StreamWidget.h"
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <iostream>
#include <typeinfo>

QList<QString> StreamWidget::messageNames;


StreamWidget::StreamWidget(int number, MainWindow* mw, QWidget *parent)
    : QGroupBox(parent),
      messages(QList<MessageTemplate*>()),
      messageInEditor(0),
      previousMessageName(""),
      streamNumber(number),
      editButtonEnabled(false),
      removeButtonEnabled(false),
      mw(mw)

{
    QGridLayout* layout = new QGridLayout(this);

    this->setTitle("Stream number " + QString::number(streamNumber));

    this->setLayout(layout);

    protocolLabel = new QLabel("Transport layer protocol: ", this);

    layout->addWidget(protocolLabel,1,1);

    tcp = new QRadioButton("TCP", this);
    udp = new QRadioButton("UDP", this);
    nagle = new QCheckBox("Nagle's algorithm", this);
    appProto = new QCheckBox("Enable application protocol", this);
    ordered = new QCheckBox("Ordered transfer");
    clientGameTick = new QSpinBox(this);
    serverGameTick = new QSpinBox(this);
    messageList = new QListWidget(this);
    addMessage = new QPushButton("Add message", this);
    removeMessage = new QPushButton("Remove", this);
    editMessage = new QPushButton("Edit", this);

    QVBoxLayout* labelLayout = new QVBoxLayout(layout->widget());
    QVBoxLayout* gameTickLayout = new QVBoxLayout(layout->widget());
    QVBoxLayout* listLayout = new QVBoxLayout(layout->widget());
    QHBoxLayout* listButtons = new QHBoxLayout(layout->widget());

    listButtons->addWidget(addMessage);
    listButtons->addWidget(removeMessage);
    listButtons->addWidget(editMessage);

    editMessage->setEnabled(false);
    removeMessage->setEnabled(false);

    listLayout->addWidget(messageList);
    listLayout->addItem(listButtons);

    gameTickLayout->addWidget(clientGameTick);
    gameTickLayout->addWidget(serverGameTick);

    clientGameTickLabel = new QLabel("Client game tick (ms):", this);
    serverGameTickLabel = new QLabel("Server game tick (ms):", this);

    labelLayout->addWidget(clientGameTickLabel);
    labelLayout->addWidget(serverGameTickLabel);

    layout->addWidget(udp, 1, 2);
    layout->addWidget(tcp, 1, 3);
    layout->addWidget(appProto, 2, 2);
    layout->addWidget(nagle, 2, 3);
    layout->addWidget(ordered, 3, 2);
    layout->addItem(labelLayout, 4, 1);
    layout->addItem(gameTickLayout, 4, 2);
    layout->addItem(listLayout, 4, 3);

    QObject::connect(tcp, SIGNAL(toggled(bool)), appProto, SLOT(setDisabled(bool)));
    QObject::connect(tcp, SIGNAL(toggled(bool)), ordered, SLOT(setDisabled(bool)));
    QObject::connect(tcp, SIGNAL(toggled(bool)), nagle, SLOT(setEnabled(bool)));
    QObject::connect(tcp, SIGNAL(toggled(bool)), nagle, SLOT(setChecked(bool)));
    QObject::connect(udp, SIGNAL(toggled(bool)), appProto, SLOT(setChecked(bool)));
    QObject::connect(udp, SIGNAL(toggled(bool)), ordered, SLOT(setChecked(bool)));
    QObject::connect(appProto, SIGNAL(toggled(bool)), ordered, SLOT(setEnabled(bool)));
    QObject::connect(appProto, SIGNAL(toggled(bool)), ordered, SLOT(setChecked(bool)));
    QObject::connect(messageList, SIGNAL(currentRowChanged(int)), this, SLOT(rowFocusChanged(int)));

    udp->setChecked(true);
    nagle->setDisabled(true);
    nagle->setChecked(false);

    clientGameTick->setMaximum(10000);
    serverGameTick->setMaximum(10000);

    clientGameTick->setSingleStep(10);
    serverGameTick->setSingleStep(10);

    tcp->setToolTip("Use TCP in this stream");
    udp->setToolTip("use UDP in this stream");
    nagle->setToolTip("Enable nagle's algorithm");
    appProto->setToolTip("Enables the application protocol in UDP stream");
    ordered->setToolTip("Defines all messages in this stream to be received in order");
    clientGameTick->setToolTip("Defines the amount of time clients buffer data before sending it");
    serverGameTick->setToolTip("Defines the amount of time the server buffers data before sending it");
    messageList->setToolTip("Message types in this stream can be defined here");
    addMessage->setToolTip("Adds new message type to this stream");
    removeMessage->setToolTip("Removes the selected message from this stream");

    QObject::connect(addMessage, SIGNAL(clicked()), this, SLOT(addNewMessage()));
    QObject::connect(editMessage, SIGNAL(clicked()), this, SLOT(editExistingMessage()));
    QObject::connect(removeMessage, SIGNAL(clicked()), this, SLOT(removeMessageFromList()));

}

StreamWidget::~StreamWidget()
{
    for(QList<MessageTemplate*>::iterator it = messages.begin(); it != messages.end(); it++)
    {
        delete *it;
    }
}


void StreamWidget::addNewMessage()
{
    messageInEditor = new MessageTemplate(this, appProto->isChecked());

    previousMessageName = "";

    emit setupMessageEditor(messageInEditor, this);
}


void StreamWidget::editExistingMessage()
{
    QString text = messageList->currentItem()->text();

    for(QList<MessageTemplate*>::iterator it = messages.begin(); it != messages.end(); it++)
    {
        if((*it)->getMessageName() == text)
        {
            messageInEditor = new MessageTemplate(**it);
            previousMessageName = (*it)->getMessageName();
            break;
        }
    }

    emit setupMessageEditor(messageInEditor, this);
}

void StreamWidget::removeMessageFromList()
{
    if(messageList->currentRow() != -1)
        deleteMessageFromList(messageList->item(messageList->currentRow())->text());
}

void StreamWidget::newMessageAdded()
{
    if(mw->configMessageFromEditor(messageInEditor))
    {
        if((previousMessageName == "" || previousMessageName != messageInEditor->getMessageName()) && messageNames.contains(messageInEditor->getMessageName()))
        {
            mw->setMsgConfigErrorMessage("Message name already in use!");
        }
        else
        {
            deleteMessageFromList(previousMessageName);

            messageNames.append(messageInEditor->getMessageName());

            messages.append(new MessageTemplate(*messageInEditor));
            messageList->insertItem(messageList->count(), messageInEditor->getMessageName());
            editorClosed();
        }
    }
    else
    {
        mw->setMsgConfigErrorMessage("Missing parameters in message configuration!");
    }
}

void StreamWidget::deleteMessageFromList(const QString &messageName)
{
    for(QList<MessageTemplate*>::iterator it = messages.begin(); it != messages.end(); it++)
    {
        if((*it)->getMessageName() == messageName)
        {
            delete(*it);
            messages.erase(it);
            messageNames.removeAll(messageName);
            messageList->takeItem(messageList->currentRow());
            break;
        }
    }
}

void StreamWidget::editorClosed()
{
    delete messageInEditor;
    messageInEditor = 0;
    mw->finishEditor();
    enableStreamWidgets(true);
}

void StreamWidget::rowFocusChanged(int rowNumber)
{
    if(rowNumber != -1)
    {
        editMessage->setEnabled(true);
        editButtonEnabled = true;
        removeMessage->setEnabled(true);
        removeButtonEnabled = true;
    }
    else
    {
        editMessage->setEnabled(false);
        editButtonEnabled = false;
        removeMessage->setEnabled(false);
        removeButtonEnabled = false;
    }
}

void StreamWidget::enableStreamWidgets(bool enabled)
{
    tcp->setEnabled(enabled);
    udp->setEnabled(enabled);

    if(tcp->isChecked())
    {
        nagle->setEnabled(enabled);
    }
    else
    {
        appProto->setEnabled(enabled);
        ordered->setEnabled(enabled);
    }

    clientGameTick->setEnabled(enabled);
    serverGameTick->setEnabled(enabled);
    messageList->setEnabled(enabled);
    addMessage->setEnabled(enabled);
    clientGameTickLabel->setEnabled(enabled);
    serverGameTickLabel->setEnabled(enabled);
    protocolLabel->setEnabled(enabled);

    if(removeButtonEnabled)
        removeMessage->setEnabled(enabled);
    else
        removeMessage->setEnabled(false);

    if(editButtonEnabled)
        editMessage->setEnabled(enabled);
    else
        editMessage->setEnabled(false);

}

