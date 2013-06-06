#include "StreamWidget.h"
#include <QGridLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

QList<QString> StreamWidget::messageNames;


StreamWidget::StreamWidget(int number, MainWindow* mw, QWidget *parent)
    : QGroupBox(parent),
      messageInEditor(0),
      streamNumber(number),
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
    removeMessage = new QPushButton("Remove message", this);

    QVBoxLayout* labelLayout = new QVBoxLayout(layout->widget());
    QVBoxLayout* gameTickLayout = new QVBoxLayout(layout->widget());
    QVBoxLayout* listLayout = new QVBoxLayout(layout->widget());
    QHBoxLayout* listButtons = new QHBoxLayout(layout->widget());

    listButtons->addWidget(addMessage);
    listButtons->addWidget(removeMessage);

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

    QObject::connect(addMessage, SIGNAL(clicked()), this, SLOT(openMessageEditor()));
    QObject::connect(removeMessage, SIGNAL(clicked()), this, SLOT(removeMessageFromList()));

}


void StreamWidget::openMessageEditor()
{
    delete messageInEditor;

    messageInEditor = new MessageTemplate(this, appProto->isChecked());
    emit setupMessageEditor(messageInEditor, this);

}

void StreamWidget::removeMessageFromList()
{


}

void StreamWidget::newMessageAdded()
{
    if(mw->configMessageFromEditor(messageInEditor))
    {
        if(messageNames.contains(messageInEditor->getMessageName()))
        {
            mw->setMsgConfigErrorMessage("Message name already in use!");
        }
        else
        {
            messageNames.append(messageInEditor->getMessageName());
            messages.append(new MessageTemplate(messageInEditor));
            messageList->insertItem(messageList->count(), messageInEditor->getMessageName());
            editorClosed();
        }
    }
    else
    {
        mw->setMsgConfigErrorMessage("Missing parameters in message configuration!");
    }
}

void StreamWidget::editorClosed()
{
    delete messageInEditor;
    messageInEditor = 0;
    mw->finishEditor();
    enableStreamWidgets(true);
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
    removeMessage->setEnabled(enabled);
    clientGameTickLabel->setEnabled(enabled);
    serverGameTickLabel->setEnabled(enabled);
    protocolLabel->setEnabled(enabled);

}

