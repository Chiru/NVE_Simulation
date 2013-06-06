#ifndef MESSAGETEMPLATE_H
#define MESSAGETEMPLATE_H

#include <QGroupBox>

typedef enum{Uniform = 0, Constant, Sequential, Exponential, Pareto, Weibull, Normal, Lognormal, Gamma, Erlang, Zipf, Zeta, Triangular, Empirical, Extreme, Split, None} Distribution;

struct DistributionElement{
    Distribution dist;
    QList<double> params;
};

class MessageTemplate : public QGroupBox
{
public:
    typedef enum {ClientToServer, ServerToClient} TYPE;
    MessageTemplate( QWidget* parent = 0, bool appProtoEnabled = false);
    MessageTemplate(const MessageTemplate& c);
    QString getMessageName() const {return messageName;}
    TYPE getType() const {return type;}
    bool isAppProtoEnabled() const {return appProtoEnabled;}
    bool isReliable() const {return reliable;}
    bool isReturnedToSender() const {return returnToSender;}
    DistributionElement getMessageSize() const {return messageSize;}
    DistributionElement getTimeInterval() const {return timeInterval;}
    DistributionElement getForwardMessageSize() const {return forwardMessageSize;}
    DistributionElement getClientsOfInterest() const {return clientsOfInterest;}
    void setMessageName(const QString&);
    void setMessageType(int);
    void setAppProtoEnabled(bool enabled);
    void setReliable(bool);
    void setReturnToSender(bool);
    void setMessageSize(Distribution, QList<double>);
    void setTimeInterval(Distribution, QList<double>);
    void setForwardMessageSize(Distribution, QList<double>);
    void setClientsOfInterest(Distribution, QList<double>);

private:

    QString messageName;
    TYPE type;
    bool appProtoEnabled;
    bool reliable;
    bool returnToSender;
    DistributionElement messageSize;
    DistributionElement timeInterval;
    DistributionElement forwardMessageSize;
    DistributionElement clientsOfInterest;

    static int messageTypeCount;



};

#endif // MESSAGETEMPLATE_H
