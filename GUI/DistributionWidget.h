#ifndef DISTRIBUTIONWIDGET_H
#define DISTRIBUTIONWIDGET_H

#include <QFrame>
#include <QGridLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QDialog>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include "MessageTemplate.h"


class DistributionWidget : public QFrame
{

    Q_OBJECT

public:
    DistributionWidget(const QString& label, QWidget* parent = 0);
    DistributionWidget(QLabel* label, QPushButton* button, QLineEdit* resultDist, QLayout* layout, QWidget* parent = 0);
    ~DistributionWidget();
    void setValue(DistributionElement*);
    bool getDistributionName(QString&) const;

    void enableWidget(bool enabled);

private:
    DistributionElement* distribution;
    QLayout* layout;
    QLabel* description;
    QLineEdit* distributionName;
    QPushButton* config;

    void connectSignals();

private slots:
    void createDistributionDialog();

};


class DistributionDialog : public QDialog
{

    Q_OBJECT

public:
    DistributionDialog(bool alreadySplit);
    ~DistributionDialog();
    DistributionElement* getDistribution();
    const QList<QDoubleSpinBox*>& getSplitDistributionPercentages() const {return splitDistributionPercentages;}

private:
    DistributionElement* distribution;
    QDialogButtonBox* buttonBox;
    QGridLayout* dialogLayout;
    QComboBox* availableDistributions;
    QList<QDoubleSpinBox*> params;
    QList<QLabel*> paramDescriptions;
    QList<QLineEdit*> splitDistributionNames;
    QList<QDoubleSpinBox*> splitDistributionPercentages;
    QPushButton* splitDistributionButton;
    QFormLayout* paramsLayout;
    QLineEdit* fileName;
    bool alreadySplit;
    int numberOfDistributionsSplit;

    void createDistributionComboBox();
    void cleanUpDialog();
    void addParamWidgetsToDialog();
    void setValues() const;
    void updateSplitDistribution();


private slots:
    void acceptDistributionDialog();
    void cancelDistributionDialog();
    void distributionChanged(int);
    void createDistributionDialog();

};


#endif // DISTRIBUTIONWIDGET_H
