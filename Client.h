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

#ifndef CLIENT_H
#define CLIENT_H

#include "utilities.h"
#include "ns3/address.h"

class XMLParser;
class DataGenerator;

class Client{

    friend std::ostream& operator<<(std::ostream &out, const Client &client){

        out << "Client number: " << client.clientNumber << "  Network delay: " << client.networkDelay
            << "  Downlink bandwidth: " << client.downlinkBandwidth << "  Uplink bandwidth: " << client.uplinkBandwidth
            << "  Packet loss rate: " << client.lossRate;

        for(int i = 0; i < client.numberOfStreams; i++){
             if((client.streams[i]) != 0)
                out << *(client.streams[i]);
        }

        return out;
    }

public:
    Client(XMLParser&, uint16_t no, int runningTime, Ptr<Node>, Address* peerAddr);
    ~Client();

    std::string getDelayInMilliseconds() const;
    std::string getUplinkBandwidthInMegabits() const;
    std::string getDownlinkBandwidthInMegabits() const;
    double getLossRate() const {return lossRate;}

private:
    XMLParser &parser;
    DataGenerator** streams;
    int networkDelay;
    double uplinkBandwidth;
    double downlinkBandwidth;
    double lossRate;
    uint16_t clientNumber;
    uint16_t numberOfStreams;
    int runningTime;
    Ptr<Node> node;
    Address* peerAddr;

};



//Class Client function definitions

Client::Client(XMLParser& parser, uint16_t no, int runningTime, Ptr<Node> node, Address* peerAddr)
    : parser(parser), streams(0), runningTime(runningTime), node(node), peerAddr(peerAddr){

   parser.getStreams(streams, true, no);
   numberOfStreams = parser.getNumberOfStreams();

   if(!parser.getClientStats(no, clientNumber, networkDelay, uplinkBandwidth, downlinkBandwidth, lossRate))
       PRINT_ERROR( "Mysterious error while creating " << no << ". client." << std::endl);

   for(int i = 0; i < numberOfStreams; i++){
       streams[i]->SetStartTime(Seconds(0));
       streams[i]->SetStopTime(Seconds(runningTime));
       streams[i]->setupStream(node, peerAddr[i]);
       node->AddApplication(streams[i]);
   }

}

Client::~Client(){

    uint64_t bytesSent = 0L;

    for(int i = 0; i < parser.getNumberOfStreams(); i++){
        if(streams != 0 && streams[i] != 0){
            bytesSent += streams[i]->getBytesSent();
            delete streams[i];
        }
    }

    if(streams != 0)
         delete[] streams;

    CLIENT_INFO("Client number: " << clientNumber << " finishing, sent " << bytesSent << " bytes in total." << std::endl);
}

std::string Client::getDelayInMilliseconds() const{

    std::stringstream stream;

    stream << networkDelay << "ms";

    return stream.str();
}

std::string Client::getDownlinkBandwidthInMegabits() const{

    std::stringstream stream;

    stream << downlinkBandwidth << "Mbps";

    return stream.str();
}

std::string Client::getUplinkBandwidthInMegabits() const{

    std::stringstream stream;

    stream << uplinkBandwidth << "Mbps";

    return stream.str();
}


#endif // CLIENT_H
