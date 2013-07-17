#ifndef MESSAGETEMPLATE_H
#define MESSAGETEMPLATE_H

#include <QGroupBox>
#include <QDoubleSpinBox>

typedef enum{Uniform = 0, Constant, Sequential, Exponential, Pareto, Weibull, Normal, Lognormal, Gamma, Erlang, Zipf, Zeta, Triangular, Extreme, Empirical, Split, None} Distribution;

class DistributionElement
{

public:
    DistributionElement();
    DistributionElement(const DistributionElement&);
    ~DistributionElement();

    void setDist(Distribution);
    void setParams(const QList<double>&);
    void setFileName(const QString&);
    void copyDistributions(const QList<DistributionElement*>&);
    Distribution getDist() const {return dist;}
    QString getFileName() const {return filename;}
    QList<double>& getParams() {return params;}
    const QList<double>& getParams() const {return params;}
    QList<DistributionElement*>& getSplitDistributions() {return splitDistributions;}
    DistributionElement& operator=(const DistributionElement& elem);
    QString getDistributionString() const;
    double getPercentage() const {return percentage;}
    void addSplitDistribution(const DistributionElement* dist);
    void setSplitDistributionPercentages(const QList<QDoubleSpinBox*>& percentages);
    void setPercentage(double);
    void setDistributionString(const QString& dist);

private:
    Distribution dist;
    QList<double> params;
    QString filename;
    QList<DistributionElement*> splitDistributions;
    QString distributionString;
    double percentage; //this is only for split distribution

};

class MessageTemplate : public QGroupBox
{

    friend class XmlSerializer;

public:
    typedef enum {ClientToServer, ServerToClient} TYPE;
    MessageTemplate( QWidget* parent = 0, bool appProtoEnabled = false);
    MessageTemplate(const MessageTemplate& c);
    QString getMessageName() const {return messageName;}
    const DistributionElement& getMessageSize() const {return messageSize;}
    const DistributionElement& getMessageTimeInterval() const {return timeInterval;}
    bool getForwardMessageSize(int& size) const;
    TYPE getType() const {return type;}
    bool isAppProtoEnabled() const {return appProtoEnabled;}
    bool isReliable() const {return reliable;}
    bool isReturnedToSender() const {return returnToSender;}
    double getClientsOfInterest() const {return clientsOfInterest;}
    int getTimeReqClient() const {return timeRequirementClient;}
    int getTimeReqServer() const {return timeRequirementServer;}

    void setMessageName(const QString&);
    void setMessageType(int);
    void setAppProtoEnabled(bool enabled);
    void setReliable(bool);
    void setReturnToSender(bool);
    bool setMessageSize(DistributionElement*);
    bool setTimeInterval(DistributionElement*);
    void setMessageSize(const QString&);
    void setTimeInterval(const QString&);
    void setForwardMessageSize(int size, bool useRecvSize);
    void setClientsOfInterest(double);
    void setTimeRequirementClient(int);
    void setTimeRequirementServer(int);

private:

    QString messageName;
    TYPE type;
    bool appProtoEnabled;
    bool reliable;
    bool returnToSender;
    DistributionElement messageSize;
    DistributionElement timeInterval;
    bool useReceivedMessageSize;
    uint forwardMessageSize;
    double clientsOfInterest;
    int timeRequirementClient;
    int timeRequirementServer;

    static int messageTypeCount;



};

#endif // MESSAGETEMPLATE_H
