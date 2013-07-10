#ifndef DATASENDER_H
#define DATASENDER_H

#include "ns3/address.h"
#include "ns3/socket.h"
#include "ApplicationProtocol.h"
#include "Messages.h"
#include <map>
//

class DataSender{

public:
    DataSender(ApplicationProtocol* appProto, uint16_t gameTick) : appProto(appProto), gameTick(gameTick), maxDatagramSize(1400), lastSendTimes(){}

    bool send(bool sendNow, uint8_t* buffer, const Message* msg, const Ptr<Socket> sock, bool forward, bool isClient);
    bool sendTo(bool sendNow, uint8_t* buffer, const Message* msg, const Address& addr, bool forward, bool isClient, const Ptr<Socket> sock = 0);
    bool flushUdpBuffer(Ptr<Socket>, bool isClient);
    bool flushTcpBuffer(bool isClient);
    void setGameTick(uint16_t gameTick){this->gameTick = gameTick;}

private:

    struct MessageInfo{
        MessageInfo(uint16_t stream, int msgNumber, std::string msgName): streamNumber(stream), messageNumber(msgNumber), name(msgName){}

        uint16_t streamNumber;
        int messageNumber;
        std::string name;

    };

    std::map<Ptr<Socket>, std::string> tcpBuffer;
    std::vector<MessageInfo> clientMessagesInTcpBuffer;
    std::vector<MessageInfo> serverMessagesInTcpBuffer;
    std::map<Address, std::pair<std::string, bool> > udpBuffer;
    std::vector<MessageInfo> clientMessagesInUdpBuffer;
    std::vector<MessageInfo> serverMessagesInUdpBuffer;
    ApplicationProtocol* appProto;
    bool isClient;
    uint16_t gameTick;
    uint16_t maxDatagramSize;
    std::map<std::string, Time, Message::StringComparator> lastSendTimes;   //collect time inter-departure times for each message type in this stream

    int sendAndFragment(Ptr<Socket> socket, uint8_t *buffer, uint16_t size, bool reliable, const Address *const addr = 0);
    Time getMessageSendInterval(int messageNumber, bool isClient, const std::string& name);

};


#endif // DATASENDER_H
