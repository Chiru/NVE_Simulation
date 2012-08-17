/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef APPLICATION_PROTOCOL_H
#define APPLICATION_PROTOCOL_H



#include "ns3/applications-module.h"
#include "ns3/nstime.h"
#include "ns3/core-module.h"
#include <vector>
#include <string>
#include <cstring>
#include <deque>
#include "utilities.h"
#include "Messages.h"


class ApplicationProtocol{

    friend class XMLParser;

public:
    ~ApplicationProtocol();
    bool sendFromClient(const Message*, uint8_t*, Ptr<Socket>, bool forward = false);
    bool sendFromClient(const std::string& buffer, Ptr<Socket>, bool reliable);
    bool sendFromServer(uint8_t*, const Message*, const Address&, Ptr<Socket>, bool forward = false);
    bool sendFromServer(const std::string& buffer, const Address& addr, Ptr<Socket> sock, bool reliable);
    void configureForStream(Callback<void, uint8_t*, uint16_t, Address&> memFunc);
    void recv(Ptr<Socket>);
    uint16_t getHeaderSize() const{return headerSize;}

    //recursive function used to send all messages that fit into single datagram
    static int sendFragment(const std::string& buffer, const size_t index, Ptr<Socket> sock, uint16_t maxDatagramSize, const Address* const addr = 0,
                            ApplicationProtocol* appProto = 0, bool reliable = false, uint16_t headerSize = 0);
private:
    ApplicationProtocol(uint16_t ackSize, uint16_t delayedAck, uint16_t retransmit, uint16_t headerSize);


    class ReliablePacket{
    public:
        ReliablePacket(uint32_t msgNumber, uint16_t msgSize, uint8_t* buffer, Address addr = Address()):
            addr(addr), msgNumber(msgNumber), msgSize(msgSize), buffer(buffer){}

        ~ReliablePacket(){
            free(buffer);
        }
        Address addr;
        uint32_t msgNumber;
        uint16_t msgSize;
        uint8_t* buffer;
    };

    enum AppProtoPacketType{UNRELIABLE, RELIABLE, DUPLICATE, ACK, ERROR}; //distinguish between different application proto headers

    std::list<ReliablePacket*> packetsWaitingAcks;
    std::map<const Address, std::deque<uint32_t> > packetsToAck;  //server needs to remember also the client address for each message it has received
    std::map<const Address, std::list<uint32_t> > alreadyAcked;   //ack these again, but do not forward to application
    uint16_t ackSize;
    uint16_t delayedAck;
    uint16_t retransmit;
    uint16_t headerSize;
    uint64_t totalBytesSent;
    uint32_t reliableMsgNumber;
    Ptr<Socket> socket;
    uint16_t maxDatagramSize;
    Callback<void, uint8_t*, uint16_t, Address&> forwardToApplication;

    void resendCheckClient(uint32_t reliableMsgNumber);   //resend data without an ack before the timer runs out
    void resendCheckServer(uint32_t reliableMsgNumber, Address addr);
    void addAppProtoHeader(char* buffer, bool reliable);
    AppProtoPacketType parseAppProtoHeader(uint8_t* buffer, const Address& addr, Ptr<Socket> sock);
    bool sendAck(int* messagesToAck, uint16_t numberOfMessages, const Address& addr, Ptr<Socket> sock);
    uint16_t createAck(char* ack, int* numbers, uint16_t numberOfMessages);
    void ackAllPackets();

    //IP fragmentation over UDP doesn't work like a charm, so send big packets in separate datagrams
    int sendAndFragment(Ptr<Socket> socket, uint8_t* buffer, uint16_t size, bool reliable, const Address* const addr = 0);
};


//Class ApplicationProtocol function definitions

ApplicationProtocol::ApplicationProtocol(uint16_t packetSize, uint16_t delayedAck, uint16_t retransmit, uint16_t headerSize)
    : ackSize(packetSize),
      delayedAck(delayedAck),
      retransmit(retransmit),
      headerSize(headerSize),
      reliableMsgNumber(0),
      socket(0),
      maxDatagramSize(1400){  //TODO: hard coding

}

ApplicationProtocol::~ApplicationProtocol(){

    for(std::list<ReliablePacket*>::iterator it = packetsWaitingAcks.begin(); it != packetsWaitingAcks.end(); it++){
        delete(*it);
    }

}


bool ApplicationProtocol::sendFromClient(const Message *msg, uint8_t *buffer, Ptr<Socket> socket, bool forward){

    if(this->socket == 0){
        this->socket = socket;
    }

    uint16_t bytesSent = 0;

    char msgContents[30 + headerSize];
    msgContents[0] = '\0';

    addAppProtoHeader(msgContents, msg->getReliable());
    memcpy(msgContents + headerSize, (char*)buffer, 30);

    uint16_t messageSize;
    int msgId;

    msg->parseMessageId(std::string((char*)buffer), msgId);

    if(!forward)
        messageSize = msg->getMessageSize(msgId);
    else
        messageSize = msg->getForwardMessageSize(msgId);

    if(socket->GetTxAvailable() < messageSize + headerSize){        //TODO: how to remember messages when buffer overflows
        return false;
    }

    if((bytesSent = sendAndFragment(socket, (uint8_t*)msgContents, messageSize + headerSize, msg->getReliable(), 0)) == -1){
        return false;
    }

    if(msg->getReliable()){
        uint8_t* tempMsg;
        tempMsg = (uint8_t*)malloc(messageSize + headerSize);
        memcpy(tempMsg, msgContents, messageSize + headerSize);
        packetsWaitingAcks.push_back(new ApplicationProtocol::ReliablePacket(reliableMsgNumber, msg->getMessageSize(msgId), tempMsg));
        Simulator::Schedule(Time(MilliSeconds(retransmit)), &ApplicationProtocol::resendCheckClient, this, reliableMsgNumber);
    }

    totalBytesSent += bytesSent;

    return true;
}

bool ApplicationProtocol::sendFromClient(const std::string &buffer, Ptr<Socket> socket, bool reliable){

    if(this->socket == 0)
        this->socket = socket;

    char* msgContents = new char[buffer.length() + headerSize];
    msgContents[0] = 0;

    addAppProtoHeader(msgContents, reliable);
    memcpy(msgContents + headerSize, buffer.c_str(), buffer.length());

    if(socket->GetTxAvailable() < buffer.length() + headerSize){        //TODO: how to remember messages when buffer overflows
        delete [] msgContents;
        return false;
    }

    if(sendAndFragment(socket, (uint8_t*)msgContents, buffer.length() + headerSize, reliable, 0) == -1){
        delete [] msgContents;
        return false;
    }

    if(reliable){
        uint8_t* tempMsg;
        tempMsg = (uint8_t*)malloc(buffer.length() + headerSize);
        memcpy(tempMsg, msgContents, buffer.length() + headerSize);
        packetsWaitingAcks.push_back(new ApplicationProtocol::ReliablePacket(reliableMsgNumber, buffer.length(), tempMsg));
        Simulator::Schedule(Time(MilliSeconds(retransmit)), &ApplicationProtocol::resendCheckClient, this, reliableMsgNumber);
    }

    delete [] msgContents;

    return true;
}

void ApplicationProtocol::recv(Ptr<Socket> socket){

    if(this->socket == 0)
        this->socket = socket;

    Address addr;
    uint8_t* buffer = 0;
    uint16_t bufferSize;
    bufferSize = socket->GetRxAvailable();
    buffer = (uint8_t*)calloc(bufferSize, 1);
    socket->RecvFrom(buffer, bufferSize, 0, addr);

    switch(parseAppProtoHeader(buffer, addr, socket)){

        case UNRELIABLE:
        case RELIABLE:
            forwardToApplication(buffer + headerSize, bufferSize - headerSize, addr);
            break;

        case DUPLICATE: //this is already handled
        case ACK:  //this is only ack, do not forward to application
            break;

        case ERROR:
            PRINT_ERROR("Mysterious error occured in application layer protocol." << std::endl);
            break;
    }


    free(buffer);
}

bool ApplicationProtocol::sendFromServer(uint8_t *buffer, const Message* msg, const Address& addr, Ptr<Socket> socket, bool forward){

    uint16_t size;
    int msgId;

    msg->parseMessageId(std::string((char*)buffer), msgId);

    if(forward)
        size = (msg)->getForwardMessageSize(msgId);
    else
        size = msg->getMessageSize(msgId);

    if(this->socket == 0)
        this->socket = socket;

    uint16_t bytesSent = 0;

    char msgContents[30 + headerSize]; //TODO: hard-coded message size

    msgContents[0] = '\0';

    addAppProtoHeader(msgContents, msg->getReliable());
    strncpy(msgContents + headerSize, (char*)buffer, 30);

    if(socket->GetTxAvailable() < size + headerSize){        //TODO: how to remember messages when buffer overflows
        return false;
    }

    if((bytesSent =  sendAndFragment(socket,(uint8_t*)msgContents,size + headerSize, msg->getReliable(),&addr)) == -1){
        return false;
    }

    if(msg->getReliable()){
        uint8_t* tempMsg;
        tempMsg = (uint8_t*)malloc(size + headerSize);
        memcpy(tempMsg, msgContents, size + headerSize);
        packetsWaitingAcks.push_back(new ApplicationProtocol::ReliablePacket(reliableMsgNumber, size, tempMsg, addr));
        Simulator::Schedule(Time(MilliSeconds(retransmit)), &ApplicationProtocol::resendCheckServer, this, reliableMsgNumber, addr);
    }

    totalBytesSent += bytesSent;

    return true;
}

bool ApplicationProtocol::sendFromServer(const std::string &buffer, const Address &addr, Ptr<Socket> sock, bool reliable){

    uint16_t size = buffer.length();

    if(this->socket == 0)
        this->socket = sock;

    char* msgContents = new char[size + headerSize];
    msgContents[0] = 0;

    addAppProtoHeader(msgContents, reliable);
    memcpy(msgContents + headerSize, buffer.c_str(), size);

    if(sock->GetTxAvailable() < size + headerSize){        //TODO: how to remember messages when buffer overflows
        delete [] msgContents;
        return false;
    }

    if(sendAndFragment(sock,(uint8_t*)msgContents,size + headerSize, reliable, &addr) == -1){
        delete [] msgContents;
        return false;
    }

    if(reliable){
        uint8_t* tempMsg;
        tempMsg = (uint8_t*)malloc(size + headerSize);
        memcpy(tempMsg, msgContents, size + headerSize);
        packetsWaitingAcks.push_back(new ApplicationProtocol::ReliablePacket(reliableMsgNumber, size, tempMsg, addr));
        Simulator::Schedule(Time(MilliSeconds(retransmit)), &ApplicationProtocol::resendCheckServer, this, reliableMsgNumber, addr);
    }

    delete [] msgContents;

    return true;
}

void ApplicationProtocol::configureForStream(Callback<void, uint8_t*, uint16_t, Address&> memFunc){
    Simulator::Schedule(Time(MilliSeconds(delayedAck)), &ApplicationProtocol::ackAllPackets, this);
    forwardToApplication = memFunc;
}


void ApplicationProtocol::resendCheckClient(uint32_t reliableMsgNumber){

    for(std::list<ApplicationProtocol::ReliablePacket*>::const_iterator it = packetsWaitingAcks.begin(); it != packetsWaitingAcks.end(); it++){
        if((**it).msgNumber == reliableMsgNumber){
            if(socket->GetTxAvailable() < (**it).msgSize + headerSize){
                Simulator::Schedule(Time(MilliSeconds(retransmit)), &ApplicationProtocol::resendCheckClient, this, reliableMsgNumber);
                return;
            }
            sendAndFragment(socket, (**it).buffer, maxDatagramSize, true);
           // socket->Send((**it).buffer, (**it).msgSize + headerSize, 0);
            Simulator::Schedule(Time(MilliSeconds(retransmit)), &ApplicationProtocol::resendCheckClient, this, reliableMsgNumber);
            break;
        }
    }
}

void ApplicationProtocol::resendCheckServer(uint32_t reliableMsgNumber, Address addr){

    for(std::list<ApplicationProtocol::ReliablePacket*>::const_iterator it = packetsWaitingAcks.begin(); it != packetsWaitingAcks.end(); it++){
        if((**it).msgNumber == reliableMsgNumber && (**it).addr == addr){
            if(socket->GetTxAvailable() < (**it).msgSize + headerSize){
                Simulator::Schedule(Time(MilliSeconds(retransmit)), &ApplicationProtocol::resendCheckClient, this, reliableMsgNumber);
                return;
            }
            sendAndFragment(socket, (**it).buffer, maxDatagramSize, true, &(**it).addr);
           // socket->SendTo((**it).buffer, (**it).msgSize + headerSize, 0, (**it).addr);
            Simulator::Schedule(Time(MilliSeconds(retransmit)), &ApplicationProtocol::resendCheckServer, this, reliableMsgNumber, addr);
            break;
        }
    }
}

void ApplicationProtocol::addAppProtoHeader(char *buffer, bool reliable){

    std::stringstream str("");
    str << "\"app:";

    if(reliable)
        str << ++reliableMsgNumber;
    else
        str << 0;

    str << "\"";

    strncpy(buffer, str.str().c_str(), headerSize);
}

ApplicationProtocol::AppProtoPacketType ApplicationProtocol::parseAppProtoHeader(uint8_t* buffer, const Address& addr, Ptr<Socket> sock){

    if(strncmp((char*)buffer, "\"app:0\"", 7) == 0)
        return UNRELIABLE;

    if(strncmp((char*)buffer, "\"app:", 5) == 0){
        char* numberStr = strndup((char*)buffer + 5, strlen((char*)buffer) - 5);
        uint32_t msgNumber = atoi(numberStr);
        free(numberStr);

        if(packetsToAck.count(addr) == 1){
            for(std::list<uint32_t>::const_reverse_iterator it = alreadyAcked[addr].rbegin(); it != alreadyAcked[addr].rend(); it++){
                if(*it == msgNumber){
                    packetsToAck[addr].push_back(msgNumber);
                    return DUPLICATE;
                }
            }
            packetsToAck[addr].push_back(msgNumber);
        }else{
            packetsToAck.insert(std::make_pair<Address, std::deque<uint32_t> >(addr, std::deque<uint32_t>()));
            packetsToAck[addr].push_back(msgNumber);
        }

        return RELIABLE;
    }

    if(strncmp((char*)buffer, "\"ack:", 5) == 0){
        char* numberStr = strndup((char*)buffer + 5, strlen((char*)buffer) - 5);
        std::stringstream stream;
        uint32_t msgNumber;
        char c;
        stream << numberStr;
        std::list<ApplicationProtocol::ReliablePacket*>::iterator it;

        do{
            stream >> msgNumber;
            for(it = packetsWaitingAcks.begin(); it != packetsWaitingAcks.end(); it++){
                if((**it).msgNumber == msgNumber && ((**it).addr.IsInvalid() || (**it).addr == addr)){
                    delete *it;
                    packetsWaitingAcks.erase(it);
                    break;
                }
            }
        }while(stream >> c, c == ',');

        free(numberStr);
        return ACK;
    }

    return ERROR; //if this happens, there's a bug
}

void ApplicationProtocol::ackAllPackets(){
    int* messages = 0;
    int i;

    for(std::map<Address, std::deque<uint32_t> >::iterator it = packetsToAck.begin(); it != packetsToAck.end(); it++){
        if(it->second.empty())
            continue;

        messages = new int[it->second.size()];
        i = 0;

        while(!(it->second.empty())){
            messages[i++] = it->second.front();
            it->second.pop_front();
        }

        if(sendAck(messages, i, it->first, socket)){    //TODO: if there's problems with socket buffer, no ack is sent and packet gets "lost"
            if(alreadyAcked.count(it->first) == 0){
                alreadyAcked.insert(std::make_pair<Address, std::list<uint32_t> >(it->first, std::list<uint32_t>()));
                for(int h = 0; h < i; h++){
                    alreadyAcked[it->first].push_back(messages[h]);
                }

            }else{
                for(int h = 0; h < i; h++){
                    alreadyAcked[it->first].push_back(messages[h]);
                }
            }
        }
        delete [] messages;
    }

    Simulator::Schedule(Time(MilliSeconds(delayedAck)), &ApplicationProtocol::ackAllPackets, this);

}

bool ApplicationProtocol::sendAck(int *messagesToAck, uint16_t numberOfMessages, const Address &addr, Ptr<Socket> sock){

    char ack[ackSize * numberOfMessages];

    uint16_t ackSize = createAck(ack, messagesToAck, numberOfMessages);

    if(sock->GetTxAvailable() < (uint32_t)(ackSize))
        return false;

    if(sock->SendTo((uint8_t*)ack, ackSize, 0, addr) == -1)
        return false;

    return true;
}

uint16_t ApplicationProtocol::createAck(char *ack, int* number, uint16_t numberOfMessages){

    std::stringstream str;
    str << "\"ack:";
    for(int i = 0; i < numberOfMessages; i++){
        str << number[i];
        if((i + 1) < numberOfMessages){
            str << ",";
        }
    }
    str << "\"";

    uint16_t ackSize;

    if(str.str().length() <= this->ackSize)
        ackSize = this->ackSize;
    else
        ackSize = str.str().length();

    strncpy(ack, str.str().c_str(), ackSize);

    return ackSize;
}

int ApplicationProtocol::sendAndFragment(Ptr<Socket> socket, uint8_t *buffer, uint16_t size, bool reliable, const Address *const addr){

    if(size > maxDatagramSize){

        std::string tempBuffer((char*)buffer, size);
        return sendFragment(tempBuffer, 0, socket, maxDatagramSize, addr, this, reliable, headerSize);

    }else{

        if(addr == 0){
            return socket->Send(buffer, size, 0);
        }else{
            return socket->SendTo(buffer, size, 0, *addr);
        }
    }

    return -1;  //we should never get here
}

int ApplicationProtocol::sendFragment(const std::string& buffer, const size_t index, Ptr<Socket> sock, uint16_t maxDatagramSize, const Address* const addr, ApplicationProtocol* appProto, bool reliable,
                                      uint16_t headerSize){

    size_t tempIndex = 0, tempIndex2 = 0;
    tempIndex = buffer.find('"', index);   //find first "
    tempIndex = buffer.find('"', tempIndex + 1);  // find second "
    if((tempIndex = buffer.find('"', tempIndex + 1)) == std::string::npos){  //see if there's third
        if(addr == 0){
            return sock->Send((uint8_t*)buffer.substr(0, buffer.length()).c_str(), buffer.length(), 0);
        }
        else{
            return sock->SendTo((uint8_t*)buffer.substr(0, buffer.length()).c_str(), buffer.length(), 0, *addr);
        }
    }else{
        if( tempIndex >= maxDatagramSize || (tempIndex2 = buffer.find('"', tempIndex + 1)) >= maxDatagramSize){
            int retval = 0;
            char header[headerSize];
            std::string tempString;
            if(addr == 0){
                if(appProto){
                    appProto->addAppProtoHeader(header, reliable);
                    tempString.assign(header, headerSize);
                }

                tempString.append(buffer.substr(tempIndex, buffer.length()-tempIndex));
                retval = sock->Send((uint8_t*)buffer.substr(0, tempIndex).c_str(), tempIndex, 0);

                if(retval == -1){
                    return -1;
                }

                return (sendFragment(tempString, 0, sock,maxDatagramSize,  addr, appProto, reliable, headerSize) + retval);
            }
            else{
                if(appProto){
                    appProto->addAppProtoHeader(header, reliable);
                    tempString.assign(header, headerSize);
                }

                tempString.append(buffer.substr(tempIndex, buffer.length()-tempIndex));
                retval = sock->SendTo((uint8_t*)buffer.substr(0, tempIndex).c_str(), tempIndex, 0, *addr);

                if(retval == -1){
                    return -1;
                }

                return (sendFragment(tempString, 0, sock, maxDatagramSize, addr, appProto, reliable, headerSize) + retval);

            }
        }else{
            return sendFragment(buffer, tempIndex, sock, maxDatagramSize, addr, appProto, reliable, headerSize);
        }

    }

    return -1; //we should never get here
}

#endif // APPLICATION_PROTOCOL_H
