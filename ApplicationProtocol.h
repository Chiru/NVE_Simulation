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
#include "utilities.h"
#include "Messages.h"


class ApplicationProtocol{

    friend class XMLParser;

public:

   //send data to socket

    //a function to forward the data packet to the application
    //

    bool sendFromClient(Message*, uint8_t*, Ptr<Socket>);
    bool sendFromServer(uint8_t*, uint16_t, const Address&, Ptr<Socket>);
    std::string recv();

private:
    ApplicationProtocol(uint16_t packetSize, uint16_t delayedAck, uint16_t retransmit, uint16_t headerSize);

    std::map<uint32_t, std::pair <uint16_t, uint8_t*> > packetsWaitingAcks;  // remember message number, message size and data buffer
    std::map<uint32_t, const Address&> clientAddress; //server needs to remember the client address for each message
    uint16_t packetSize;
    uint16_t delayedAck;
    uint16_t retransmit;
    uint16_t headerSize;
    uint64_t totalBytesSent;
    uint32_t reliableMsgNumber;
    Ptr<Socket> socket;

    void writeToSocket(std::string&);   //send data to socket
    void readFromSocket();  //recv data from socket
    void resendCheckClient(uint32_t reliableMsgNumber);   //resend data without an ack before the timer runs out
    void resendCheckServer(uint32_t reliableMsgNumber);
    void addAppProtoHeader(char* buffer);
};


//Class ApplicationProtocol function definitions

ApplicationProtocol::ApplicationProtocol(uint16_t packetSize, uint16_t delayedAck, uint16_t retransmit, uint16_t headerSize)
    : packetSize(packetSize),
      delayedAck(delayedAck),
      retransmit(retransmit),
      headerSize(headerSize),
      reliableMsgNumber(0){

}

bool ApplicationProtocol::sendFromClient(Message *msg, uint8_t *buffer, Ptr<Socket> socket){

    std::pair<uint16_t, uint8_t*> message;
    char msgContents[30 + headerSize +1];
    uint16_t bytesSent = 0;

    message = std::make_pair<uint16_t, uint8_t*>(msg->getMessageSize(), (uint8_t*)msgContents);

    addAppProtoHeader(msgContents);
    strncat(msgContents, (char*)buffer, 30);

    if(socket->GetTxAvailable() < msg->getMessageSize() + headerSize){        //TODO: how to remember messages when buffer overflows
        return false;
    }

    if((bytesSent = socket->Send((uint8_t*)msgContents, msg->getMessageSize() + headerSize, 0)) == -1){
        return false;
    }

    packetsWaitingAcks.insert(std::make_pair<uint16_t, std::pair<uint16_t, uint8_t*> >(reliableMsgNumber, message));
    totalBytesSent += bytesSent;

    Simulator::Schedule(Time(MilliSeconds(retransmit)), &ApplicationProtocol::resendCheckClient, this, reliableMsgNumber);

    reliableMsgNumber++;
    return true;
}

std::string recv(){

    return "test";
}

bool ApplicationProtocol::sendFromServer(uint8_t *buffer, uint16_t msgSize, const Address& addr, Ptr<Socket> socket){

    std::pair<uint16_t, uint8_t*> message;
    char msgContents[30 + headerSize +1]; //TODO: hard-coded message size
    uint16_t bytesSent = 0;

    message = std::make_pair<uint16_t, uint8_t*>(msgSize, (uint8_t*)msgContents);
    clientAddress.insert(std::make_pair<uint32_t, const Address&>(reliableMsgNumber, addr));

    addAppProtoHeader(msgContents);
    strncat(msgContents, (char*)buffer, 30);

    if(socket->GetTxAvailable() < msgSize + headerSize){        //TODO: how to remember messages when buffer overflows
        return false;
    }

    if((bytesSent = socket->SendTo((uint8_t*)msgContents, msgSize + headerSize, 0, addr)) == -1){
        return false;
    }

    packetsWaitingAcks.insert(std::make_pair<uint16_t, std::pair<uint16_t, uint8_t*> >(reliableMsgNumber, message));
    totalBytesSent += bytesSent;

    Simulator::Schedule(Time(MilliSeconds(retransmit)), &ApplicationProtocol::resendCheckClient, this, reliableMsgNumber);

    reliableMsgNumber++;
    return true;


    return true;
}

void ApplicationProtocol::readFromSocket(){

}

void ApplicationProtocol::writeToSocket(std::string &message){


}

void ApplicationProtocol::resendCheckClient(uint32_t reliableMsgNumber){

    if(packetsWaitingAcks.count(reliableMsgNumber) == 1){
        if(socket->GetTxAvailable() < packetsWaitingAcks[reliableMsgNumber].first + headerSize){        //TODO: how to remember messages when buffer overflows
            Simulator::Schedule(Time(MilliSeconds(retransmit)), &ApplicationProtocol::resendCheckClient, this, reliableMsgNumber);
            return;
        }
        socket->Send(packetsWaitingAcks[reliableMsgNumber].second, packetsWaitingAcks[reliableMsgNumber].first, 0);
    }
}

void ApplicationProtocol::resendCheckServer(uint32_t reliableMsgNumber){

    if(packetsWaitingAcks.count(reliableMsgNumber) == 1){
        if(socket->GetTxAvailable() < packetsWaitingAcks[reliableMsgNumber].first + headerSize){        //TODO: how to remember messages when buffer overflows
            Simulator::Schedule(Time(MilliSeconds(retransmit)), &ApplicationProtocol::resendCheckServer, this, reliableMsgNumber);
            return;
        }
       const Address& addr = clientAddress.find(reliableMsgNumber)->second;

        socket->SendTo(packetsWaitingAcks[reliableMsgNumber].second, packetsWaitingAcks[reliableMsgNumber].first, 0, addr);
    }
}

void ApplicationProtocol::addAppProtoHeader(char *buffer){

    std::stringstream str;
    str << "\"app:";
    str << reliableMsgNumber;
    str << "\"";

    const char* header;
    header = str.str().c_str();

    strncat(buffer, header, headerSize);
}

#endif // APPLICATION_PROTOCOL_H
