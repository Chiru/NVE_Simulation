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


class ApplicationProtocol{

    friend class XMLParser;

public:

    ~ApplicationProtocol();

    bool write(std::string&);
    std::string read();

private:
    ApplicationProtocol(uint16_t packetSize, uint16_t delayedAck, uint16_t retransmit);
    ApplicationProtocol(const ApplicationProtocol&);
    std::vector<int> packetsWaitingAcks;
    ns3::Ptr<ns3::Socket> socket;
    uint16_t packetSize;
    uint16_t delayedAck;
    uint16_t retransmit;

    void writeToSocket(std::string&);
    void readFromSocket();
    void resend();
};


//Class ApplicationProtocol function definitions

ApplicationProtocol::ApplicationProtocol(uint16_t packetSize, uint16_t delayedAck, uint16_t retransmit)
    : packetSize(packetSize),
      delayedAck(delayedAck),
      retransmit(retransmit){


}

ApplicationProtocol::ApplicationProtocol(const ApplicationProtocol &appProto){




}

ApplicationProtocol::~ApplicationProtocol(){

}

std::string ApplicationProtocol::read(){

    return "test";

}

bool ApplicationProtocol::write(std::string &message){

    return true;

}

void ApplicationProtocol::readFromSocket(){

}

void ApplicationProtocol::writeToSocket(std::string &message){


}

void ApplicationProtocol::resend(){


}



#endif // APPLICATION_PROTOCOL_H
