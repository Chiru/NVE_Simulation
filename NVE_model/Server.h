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

#ifndef SERVER_H
#define SERVER_H
#include "utilities.h"
#include "DataGenerator.h"
#include "ns3/address.h"


class XMLParser;

class Server{

public:
    Server(XMLParser&, int, Ptr<Node>, Address*);
    ~Server();
    bool pcapEnabled() const {return pcap;}

private:
    XMLParser &parser;
    uint16_t numberOfStreams;
    DataGenerator** streams;
    int runningTime;
    Ptr<Node> node;
    Address* address;
    bool pcap;

};


#endif // SERVER_H
