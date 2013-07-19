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
#include "DataGenerator.h"

class XMLParser;


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
    bool pcapEnabled() const {return pcap;}
    bool graphsEnabled() const {return graph;}

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
    bool pcap;
    bool graph;

};


#endif // CLIENT_H
