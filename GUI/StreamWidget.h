#ifndef LISTITEM_H
#define LISTITEM_H

#include <QGroupBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QSpinBox>

class StreamWidget : public QGroupBox
{
public:
    StreamWidget(int number, QWidget* parent = 0);

private:

    int streamNumber;
    QRadioButton* tcp;
    QRadioButton* udp;
    QCheckBox* nagle;
    QCheckBox* appProto;
    QCheckBox* ordered;
    QSpinBox* clientGameTick;
    QSpinBox* serverGameTick;

};

#endif // LISTITEM_H
