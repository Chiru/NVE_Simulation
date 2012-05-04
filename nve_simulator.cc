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


#include "ns3/applications-module.h"
#include "ns3/nstime.h"
#include "ns3/core-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/internet-stack-helper.h"
#include "XML_parser.h"
#include "Server.h"
#include "Client.h"
#include <fstream>
#include <sstream>



using namespace ns3;


int main(int argc, char** argv){

    int i;

    std::string XML_filename = "scratch/xmltest.txt";
    XMLParser parser = XMLParser(XML_filename);
    if(!parser.isFileCorrect()){
        std::cerr << "Terminating due to a incorrect XML format" << std::endl;
        return 0;
    }

    uint16_t numberOfClients = parser.getNumberOfClients();

    Server server = Server(parser);
    Client* clients[numberOfClients];

    for(uint16_t i = 0; i < numberOfClients; i++){
        clients[i] = new Client(parser, i+1);
        std::cout << *(clients[i]) << std::endl;
    }



    NodeContainer allNodes;
    allNodes.Create(numberOfClients +2);   //a node for each client, one for the router and one for the server

    NodeContainer clientRouterNodes[numberOfClients];

    for(int i = 0; i < numberOfClients; i++){
        clientRouterNodes[i] = NodeContainer(allNodes.Get(i), allNodes.Get(numberOfClients));
    }

    NodeContainer routerServerNodes = NodeContainer(allNodes.Get(numberOfClients), allNodes.Get(numberOfClients+1));

    PointToPointHelper pointToPoint[numberOfClients + 1];    //point-to-point connection for each client-router connection and one for router-server connection

    for(i = 0; i < numberOfClients; i++){
        //TODO: implement data rate configuration
        pointToPoint[i].SetChannelAttribute("Delay", StringValue(clients[i]->getDelayInMilliseconds()));
    }

    NetDeviceContainer clientRouterDevices[numberOfClients];

    for(i = 0; i < numberOfClients; i++){
        clientRouterDevices[i] = pointToPoint[i].Install(clientRouterNodes[i]);
    }

    NetDeviceContainer routerServerDevices = pointToPoint[numberOfClients].Install(routerServerNodes);

    InternetStackHelper stack;
    stack.Install(allNodes);

    Simulator::Run();
    Simulator::Destroy();

    for(int i = 0; i < numberOfClients; i++){
        if(clients[i] != 0)
            delete clients[i];
    }

}
