#ifndef MESSAGEWIDGET_H
#define MESSAGEWIDGET_H

#include <QGroupBox>

class MessageWidget : public QGroupBox
{
public:
    MessageWidget(QWidget* parent = 0);


private:
    static int messageTypeCount;



};

#endif // MESSAGEWIDGET_H
