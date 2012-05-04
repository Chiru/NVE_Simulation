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
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-interface-container.h"
#include "XML_parser.h"
#include "Server.h"
#include "Client.h"
#include <fstream>
#include <sstream>



using namespace ns3;

void printAddresses(NetDeviceContainer *deviceContainer, Ipv4InterfaceContainer *ipv4Container,  int count);

int main(int argc, char** argv){

    int i;
    std::stringstream str;
    std::string addressBase;
    uint16_t numberOfClients;
    uint16_t totalNumberOfNodes;
    Ipv4InterfaceContainer* clientRouterIpInterfaces;
    Ipv4InterfaceContainer routerServerIpInterfaces;

    std::string XML_filename = "scratch/xmltest.txt";
    XMLParser parser = XMLParser(XML_filename);
    if(!parser.isFileCorrect()){
        std::cerr << "Terminating due to a incorrect XML format" << std::endl;
        return 0;
    }

    numberOfClients = parser.getNumberOfClients();
    totalNumberOfNodes = numberOfClients + 2;

    Server server = Server(parser);
    Client* clients[numberOfClients];

    for(uint16_t i = 0; i < numberOfClients; i++){
        clients[i] = new Client(parser, i+1);
        std::cout << *(clients[i]) << std::endl;
    }


    NodeContainer allNodes;
    allNodes.Create(totalNumberOfNodes);   //a node for each client, one for the router and one for the server

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

    //TODO: add network configurations maybe here

    Ipv4AddressHelper address;
    str.str("");

    clientRouterIpInterfaces = new Ipv4InterfaceContainer[numberOfClients];

    for(i = 0; i < numberOfClients; i++, str.str("")){
        str << "10.1." << i+1 << ".0";
        addressBase = str.str();
        address.SetBase(addressBase.c_str(), "255.255.255.0", "0.0.0.1");
        clientRouterIpInterfaces[i] = address.Assign(clientRouterDevices[i]);
    }

    str << "10.1." << numberOfClients+1 << ".0";
    address.SetBase(str.str().c_str(), "255.255.255.0", "0.0.0.1");
    routerServerIpInterfaces = address.Assign(routerServerDevices);

    printAddresses(clientRouterDevices, clientRouterIpInterfaces, numberOfClients);
    printAddresses(&routerServerDevices, &routerServerIpInterfaces, 1);

    Simulator::Run();
    Simulator::Destroy();

    for(i = 0; i < numberOfClients; i++){
        if(clients[i] != 0)
            delete clients[i];
    }

    delete[] clientRouterIpInterfaces;

}

void printAddresses(NetDeviceContainer *deviceContainer, Ipv4InterfaceContainer *ipv4Container, int count){

    NetDevice* device;
    Ipv4InterfaceAddress address;
    Ipv4* tempAddress;
    std::vector<Ptr<NetDevice> >::const_iterator macIt;
    std::vector<std::pair<Ptr<Ipv4>, uint32_t > >::const_iterator ipIt;

    for(int i = 0; i < count; i++){

        for(macIt =(deviceContainer +i)->Begin(), ipIt = (ipv4Container +i)->Begin(); (macIt != (deviceContainer +i)->End()) && (ipIt != (ipv4Container +i)->End()); macIt++, ipIt++){
            device = GetPointer(*macIt);
            tempAddress = GetPointer(ipIt->first);
            address = tempAddress->GetAddress(ipIt->second, 0);
            tempAddress->Unref();
            device->Unref();

            std::cout << "MAC address: "  << device->GetAddress() <<  "   ipv4 address: " <<  address << std::endl;
        }
    }
}

