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
#include "ns3/animation-interface.h"
#include "ns3/nstime.h"
#include "ns3/core-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-interface-container.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/inet-socket-address.h"
#include "ns3/csma-helper.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/error-model.h"
#include "ns3/channel.h"
#include "ns3/flow-monitor-helper.h"
#include "XML_parser.h"
#include "Server.h"
#include "StatisticsCollector.h"
#include "Client.h"
#include "utilities.h"
#include <cstdlib>
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include "simulation_interface.h"


Args::Args(const std::string &fileName)
    : verbose(false),
      serverLog(false),
      clientLog(false),
      help(false),
      fileName(fileName)
{
}


Args::Args(bool verbose, bool clientLog, bool serverLog, bool help, const std::string &fileName)
    : verbose(verbose),
      serverLog(serverLog),
      clientLog(clientLog),
      help(help),
      fileName(fileName)
{
}



void printAddresses(NetDeviceContainer *deviceContainer, Ipv4InterfaceContainer *ipv4Container,  int count);
void printHelpAndQuit();

int start(Args args){

   // LogComponentEnable("nve_simulator", LOG_LEVEL_INFO);

    int runningTime;

    int i;
    std::stringstream str;
    std::string addressBase;
    uint16_t numberOfClients;
    uint16_t totalNumberOfNodes;
    Ipv4InterfaceContainer routerServerIpInterfaces;
    Ipv4AddressHelper address;
    NodeContainer allNodes;
    StatisticsCollector* stats;
    uint16_t* serverPorts;
    bool verbose = false, clientLog = false, serverLog = false;
    std::string XML_filename;
    Ptr<RateErrorModel>* packetLoss;
    FlowMonitorHelper flowMonHelper;
    struct stat resultDir, scriptDir;


    stat("./results", &resultDir);
    stat("./results/Rscripts", &scriptDir);

    if((resultDir.st_mode & S_IFMT) != S_IFDIR || (scriptDir.st_mode& S_IFMT) != S_IFDIR ){

        if(system("mkdir ./results") == -1 || system("mkdir ./results/Rscripts") == 1){
            PRINT_ERROR("Can't create directories results and results/Rscripts, please create them." << std::endl);

        }
    }

   // AnimationInterface anim = AnimationInterface("results/animation", true);
    DropTailQueue::GetTypeId();

    Config::SetDefault("ns3::DropTailQueue::MaxPackets", UintegerValue(2000));    //TODO: change to be configurable


    if(args.isVerbose())
    {
        verbose = clientLog = serverLog = true;
    }

    if(args.serverLoggingEnabled())
    {
        serverLog = true;
    }

    if(args.clientLoggingEnabled())
    {
        clientLog = true;
    }

    if(args.needHelp())
    {
        printHelpAndQuit();
    }

    if(args.getFileName() == "")
    {
        PRINT_ERROR( "No filename given." << std::endl);
        printHelpAndQuit();
    }

    XML_filename = args.getFileName();

    XMLParser parser = XMLParser(XML_filename);

    if(!parser.isFileCorrect()){
        PRINT_ERROR( "Terminating due to an incorrect XML file" << std::endl);
        return EXIT_FAILURE;
    }

    runningTime = parser.getRunningTime();

    stats = StatisticsCollector::createStatisticsCollector(verbose, clientLog, serverLog, parser.getNumberOfStreams(), runningTime);

    if(stats == NULL){
        PRINT_ERROR( "Can't create statistics collector!" << std::endl);
        return EXIT_FAILURE;
    }

    numberOfClients = parser.getNumberOfClients();
    totalNumberOfNodes = numberOfClients + 2;

    serverPorts = new uint16_t[parser.getNumberOfStreams()];

    allNodes.Create(totalNumberOfNodes);   //a node for each client, one for the router and one for the server

    NodeContainer clientRouterNodes[numberOfClients];

    for(int i = 0; i < numberOfClients; i++){
        clientRouterNodes[i] = NodeContainer(allNodes.Get(i), allNodes.Get(numberOfClients));
    }

    packetLoss = new Ptr<RateErrorModel>[numberOfClients];

    NodeContainer routerServerNodes = NodeContainer(allNodes.Get(numberOfClients), allNodes.Get(numberOfClients+1));

    PointToPointHelper csma[numberOfClients + 1];    //point-to-point connection for each client-router connection and one for router-server connection

    //for(int i = 0; i<numberOfClients; i++)
      //  csma[i].SetChannelAttribute("DataRate", StringValue("5Mbps"));

    //csma[numberOfClients].SetChannelAttribute("DataRate", StringValue("1Gbps"));
    //csma[numberOfClients + 1].SetChannelAttribute("DataRate", StringValue("1Gbps"));


    NetDeviceContainer clientRouterDevices[numberOfClients];

    for(i = 0; i < numberOfClients; i++){
        clientRouterDevices[i] = csma[i].Install(clientRouterNodes[i]);
    }

    NetDeviceContainer routerServerDevices = csma[numberOfClients].Install(routerServerNodes);

    InternetStackHelper stack;
    stack.Install(allNodes);

    str.str("");

    Ipv4InterfaceContainer clientRouterIpInterfaces[numberOfClients];

    for(i = 0; i < numberOfClients; i++, str.str("")){
        str << "10.1." << i+1 << ".0";
        addressBase = str.str();
        address.SetBase(addressBase.c_str(), "255.255.255.0", "0.0.0.1");
        clientRouterIpInterfaces[i] = address.Assign(clientRouterDevices[i]);
    }

    str << "10.1." << numberOfClients+1 << ".0";
    address.SetBase(str.str().c_str(), "255.255.255.0", "0.0.0.1");
    routerServerIpInterfaces = address.Assign(routerServerDevices);

    Address serverAddresses[parser.getNumberOfStreams()];

    for(i = 0; i < parser.getNumberOfStreams(); i++){
        serverAddresses[i] = InetSocketAddress(routerServerIpInterfaces.GetAddress(1), 10000 + i);
    }

    Client* clients[numberOfClients];
    Server server = Server(parser, runningTime, routerServerNodes.Get(1), serverAddresses);

    for(uint16_t i = 0; i < numberOfClients; i++){
        clients[i] = new Client(parser, i+1, runningTime, clientRouterNodes[i].Get(0), serverAddresses);
        PRINT_INFO(*(clients[i]) << std::endl);
    }

    for(i = 0; i < numberOfClients; i++){
        packetLoss[i] = CreateObjectWithAttributes<RateErrorModel>("ErrorUnit", StringValue ("ERROR_UNIT_PACKET"),
                                                                   "ErrorRate", DoubleValue(clients[i]->getLossRate()));
        clientRouterDevices[i].Get(0)->SetAttribute("ReceiveErrorModel", PointerValue(packetLoss[i]));
        clientRouterDevices[i].Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(packetLoss[i]));
        clientRouterDevices[i].Get(0)->SetMtu(1514);
        clientRouterDevices[i].Get(1)->SetMtu(1514);
        clientRouterDevices[i].Get(0)->SetAttribute("DataRate", DataRateValue(DataRate(clients[i]->getUplinkBandwidthInMegabits())));
        clientRouterDevices[i].Get(1)->SetAttribute("DataRate", DataRateValue(DataRate(clients[i]->getDownlinkBandwidthInMegabits())));
        clientRouterDevices[i].Get(0)->GetChannel()->SetAttribute("Delay", TimeValue(Time(clients[i]->getDelayInMilliseconds())));
    }

    //let's make sure that router-server link does not crash
    routerServerDevices.Get(0)->SetAttribute("DataRate", DataRateValue(DataRate("1Gbps")));
    routerServerDevices.Get(1)->SetAttribute("DataRate", DataRateValue(DataRate("1Gbps")));
    routerServerDevices.Get(0)->GetChannel()->SetAttribute("Delay", TimeValue(Time("0ms")));

    if(verbose){
        printAddresses(clientRouterDevices, clientRouterIpInterfaces, numberOfClients);
        printAddresses(&routerServerDevices, &routerServerIpInterfaces, 1);
    }

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

   //for(i = 0; i <= numberOfClients; i++){
        csma[i].EnablePcapAll("results/node");
       // csma[0].EnablePcap("results/node", clientRouterNodes[0]);
       // csma[numberOfClients].EnablePcap("results/node", routerServerNodes);
    //}

    //pointToPoint[numberOfClients].EnablePcapAll("results/simulated");
   // pointToPoint[numberOfClients].EnablePcap("results/simulated", routerServerDevices);
    //pointToPoint[0].EnablePcap("results/simulated", clientRouterDevices[0]);

    stats->addFlowMonitor(flowMonHelper.InstallAll(), flowMonHelper);   //TODO: something leaks memory in flow monitoring (ns-3 bug?)


  //  anim.StartAnimation();
    Simulator::Stop(Seconds(runningTime));   //+1 second because of giving the connections time to finish
    Simulator::Run();
    Simulator::Destroy();
    //anim.StopAnimation();

    for(i = 0; i < numberOfClients; i++){
        delete clients[i];
    }

    delete[] serverPorts;
    delete[] packetLoss;

    delete stats;

    return EXIT_SUCCESS;

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

            PRINT_INFO("Ipv4 address: " <<  address << std::endl);
        }
    }
}

void printHelpAndQuit(){

    PRINT_INFO("Usage: nve_simulator --filename <file>  [--verbose]\n" << "--help    Print this help message.\n"
              << "--filename <file>     Give filename (mandatory)\n"
              << "--verbose     Print info about configuration" << std::endl);

    exit(EXIT_SUCCESS);

}


