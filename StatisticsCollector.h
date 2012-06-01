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

#ifndef STATISTICSCOLLECTOR_H
#define STATISTICSCOLLECTOR_H

#include <iostream>
#include <vector>
#include <map>
#include <list>
#include "ns3/flow-monitor.h"
#include "ns3/nstime.h"
#include "ns3/flow-monitor-module.h"
#include "utilities.h"


class StatisticsCollector{

    class MessageStats{
    public:
        MessageStats(int no, Time time, uint32_t clientRequirement, uint32_t serverRequirement): messageNumber(no), sendTime(time), serverRecvTime(0), clientRecvTimes(0),
            clientTimeRequirement(clientRequirement), serverTimeRequirement(serverRequirement){}
        int messageNumber;
        Time sendTime;
        Time serverRecvTime;
        std::list<Time> clientRecvTimes;
        uint32_t clientTimeRequirement;
        uint32_t serverTimeRequirement;
    };

public:
    ~StatisticsCollector();
    static bool getVerbose() {return verbose;}
    static bool getClientLog() {return clientLog;}
    static bool getServerLog() {return serverLog;}
    void addFlowMonitor(Ptr<FlowMonitor> flowMon, FlowMonitorHelper& helper);
    static StatisticsCollector* createStatisticsCollector(bool, bool, bool, uint16_t, int);
    static void logMessagesSendFromClient(int messageNumber, Time, uint16_t streamNumber, uint32_t clientTimeRequirement, uint32_t serverTimeRequirement);//log times when user action messages are sent
    static void logMessageReceivedByServer(int messageNumber, Time, uint16_t streamNumber);    //log times when user action messages are received by the server
    static void logMessageReceivedByClient(int messageNumber, Time, uint16_t streamNumber);    //log times when user action messages are finally forwarded to other clients

private:
    StatisticsCollector(bool, bool, bool, uint16_t, int);
    void getStreamResults(std::vector<StatisticsCollector::MessageStats*>& stats, uint16_t streamNumber, Time& clientTimeResult, Time& serverTimeResult, uint32_t& clientMsgCount,
                          uint32_t& serverMsgCount, uint32_t& toServerInTime, uint32_t& toClientInTime);
    void getBandwidthResults();

    uint16_t streamCount;
    Ptr<FlowMonitor> flowMon;
    FlowMonitorHelper helper;
    int runningTime;

    static bool verbose;
    static bool clientLog;
    static bool serverLog;
    static bool collectorCreated;
    static std::vector<MessageStats*> *messageLog;

};


//Class StatisticsCollector function definitions

bool StatisticsCollector::collectorCreated = false;
bool StatisticsCollector::verbose = false;
bool StatisticsCollector::clientLog = false;
bool StatisticsCollector::serverLog = false;
std::vector<StatisticsCollector::MessageStats*>* StatisticsCollector::messageLog;

StatisticsCollector* StatisticsCollector::createStatisticsCollector(bool verbose, bool clientLog, bool serverLog, uint16_t streamNumber, int runningTime){

    if(!collectorCreated)
        return new StatisticsCollector(verbose, clientLog, serverLog, streamNumber, runningTime);

    else {
        PRINT_ERROR( "Already one StatisticsCollector exists." << std::endl);
        return NULL;
    }
}

StatisticsCollector::StatisticsCollector(bool verbose, bool clientLog, bool serverLog, uint16_t numberOfStreams, int runningTime): streamCount(numberOfStreams), runningTime(runningTime){

    StatisticsCollector::verbose = verbose;
    StatisticsCollector::clientLog = clientLog;
    StatisticsCollector::serverLog = serverLog;

    messageLog = new std::vector<StatisticsCollector::MessageStats*>[numberOfStreams];
}

StatisticsCollector::~StatisticsCollector(){

    uint32_t clientMsgCount = 0, serverMsgCount = 0;
    uint32_t toServerInTime = 0, toClientInTime = 0;
    uint64_t timeInMilliseconds = 0;
    Time averageClientToServer("0ms");
    Time averageClientToClient("0ms");
    Time singleStreamClientToServer("0ms");
    Time singleStreamClientToClient("0ms");

    for(int h = 0; h < streamCount; h++){
        getStreamResults(messageLog[h], h+1, singleStreamClientToClient, singleStreamClientToServer, clientMsgCount, serverMsgCount, toServerInTime, toClientInTime);
        averageClientToServer += singleStreamClientToServer;
        if(!singleStreamClientToServer.IsZero()){
               singleStreamClientToServer = Time::FromInteger(0, Time::MS);
        }

        averageClientToClient += singleStreamClientToClient;
        if(!singleStreamClientToClient.IsZero()){
            singleStreamClientToClient = Time::FromInteger(0, Time::MS);
        }
    }

    if(serverMsgCount != 0){
        timeInMilliseconds = averageClientToServer.ToInteger(Time::MS);
        timeInMilliseconds /= serverMsgCount;
        averageClientToServer = Time::FromInteger(timeInMilliseconds, Time::MS);
    }else
        averageClientToServer = Time::FromInteger(0, Time::MS);

    if(clientMsgCount != 0){
        timeInMilliseconds = averageClientToClient.ToInteger(Time::MS);
        timeInMilliseconds /= clientMsgCount;
        averageClientToClient = Time::FromInteger(timeInMilliseconds, Time::MS);
    }else
        averageClientToClient = Time::FromInteger(0, Time::MS);


    for(int h = 0; h < streamCount; h++){
        for(std::vector<MessageStats*>::iterator it = messageLog[h].begin(); it != messageLog[h].end(); it++){
            delete *it;
        }
    }

    PRINT_RESULT("Overall average transmit times   clientToServer: " <<  averageClientToServer.GetMilliSeconds() << "   clientToClient: "
              << averageClientToClient.GetMilliSeconds() << "  in milliseconds" << std::endl);

    getBandwidthResults();

    delete[] messageLog;

}

void StatisticsCollector::logMessageReceivedByClient(int messageNumber, Time recvTime, uint16_t streamNumber){
    messageLog[streamNumber-1].at(messageNumber)->clientRecvTimes.push_back(recvTime);
}

void StatisticsCollector::logMessageReceivedByServer(int messageNumber, Time recvTime, uint16_t streamNumber){
    messageLog[streamNumber-1].at(messageNumber)->serverRecvTime = recvTime;
}

void StatisticsCollector::logMessagesSendFromClient(int messageNumber, Time sendTime, uint16_t streamNumber, uint32_t clientTimeRequirement, uint32_t serverTimeRequirement){
    messageLog[streamNumber-1].push_back(new MessageStats(messageNumber, sendTime, clientTimeRequirement, serverTimeRequirement));//messageNumber is the same as the index of MessageStats in messageLog because they are sent in order
}

void StatisticsCollector::getStreamResults(std::vector<StatisticsCollector::MessageStats*>& stats, uint16_t streamnumber, Time& clientTimeResult, Time& serverTimeResult,
                                           uint32_t& clientMsgCount, uint32_t& serverMsgCount, uint32_t& toServerInTime, uint32_t& toClientInTime){

    uint32_t tempClientMsgCount = 0, tempServerMsgCount = 0;
    uint32_t tempServerInTime = 0, tempClientInTime = 0;
    uint64_t timeInMilliseconds = 0;
    double clientPercentage = 0, serverPercentage = 0;
    std::list<Time>::const_iterator timeIter;
    Time clientStreamTime("0ms");
    Time serverStreamTime("0ms");

    for(std::vector<StatisticsCollector::MessageStats*>::iterator it = stats.begin(); it != stats.end(); it++, tempServerMsgCount++){
        for(timeIter = (*it)->clientRecvTimes.begin(); timeIter != (*it)->clientRecvTimes.end(); timeIter++){
            if(!(*timeIter).IsZero()){
                clientTimeResult += ((*timeIter) - (*it)->sendTime);

                if(((*timeIter) - (*it)->sendTime).GetMilliSeconds() <= (*it)->clientTimeRequirement)
                    tempClientInTime++;

                tempClientMsgCount++;
            }
        }
        if(!(*it)->serverRecvTime.IsZero()){
            serverTimeResult += ((*it)->serverRecvTime - (*it)->sendTime);


            if(((*it)->serverRecvTime - (*it)->sendTime).GetMilliSeconds() <= (*it)->serverTimeRequirement)
                tempServerInTime++;
        }
    }

    if(tempClientMsgCount != 0){
        timeInMilliseconds = clientTimeResult.ToInteger(Time::MS);
        timeInMilliseconds /= tempClientMsgCount;
        clientStreamTime = Time::FromInteger(timeInMilliseconds, Time::MS);
    }else
        clientStreamTime =  Time::FromInteger(0, Time::MS);

    if(tempServerMsgCount != 0){
        timeInMilliseconds = serverTimeResult.ToInteger(Time::MS);
        timeInMilliseconds /= tempServerMsgCount;
        serverStreamTime = Time::FromInteger(timeInMilliseconds, Time::MS);
    }else
        serverStreamTime = Time::FromInteger(0, Time::MS);

    clientMsgCount += tempClientMsgCount;
    serverMsgCount += tempServerMsgCount;
    toServerInTime += tempServerInTime;
    toClientInTime += tempClientInTime;

    clientPercentage = static_cast<double>(tempClientInTime) / static_cast<double>(tempClientMsgCount);
    serverPercentage = static_cast<double>(tempServerInTime) / static_cast<double>(tempServerMsgCount);

    PRINT_RESULT("Average transmit times for stream number: " << streamnumber << "\n\tclientToServer: " << serverStreamTime.GetMilliSeconds() << "\n\tclientToClient: "
              << clientStreamTime.GetMilliSeconds() <<  "\n\tPercentage of packets to reach server in time: " << serverPercentage <<
                  "\n\tPercentage of packets from client to client in time: " << clientPercentage << std::endl);

}

void StatisticsCollector::addFlowMonitor(Ptr<FlowMonitor> flowMon, FlowMonitorHelper& helper){

    this->flowMon = flowMon;
    this->helper = helper;

    this->flowMon->Start(Time("0ms"));
    this->flowMon->Stop(Time(runningTime));
}

void StatisticsCollector::getBandwidthResults(){

    std::map<FlowId, FlowMonitor::FlowStats> flowStats;
    Ipv4FlowClassifier::FiveTuple flowId;
    std::map<Ipv4Address, std::pair<uint64_t, uint64_t> >nodesAndBandwidths;
    std::map<Ipv4Address, std::pair<uint64_t, uint64_t> >::iterator addrIt;

    //flowMon->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (helper.GetClassifier());
    flowStats = flowMon->GetFlowStats();
    for(std::map<FlowId, FlowMonitor::FlowStats>::const_iterator it = flowStats.begin(); it != flowStats.end(); it++){
        flowId = classifier->FindFlow(it->first);

        if(nodesAndBandwidths.find(flowId.sourceAddress) == nodesAndBandwidths.end())
            nodesAndBandwidths.insert(std::make_pair<Ipv4Address, std::pair<uint64_t, uint64_t> >(flowId.sourceAddress, std::make_pair<uint64_t, uint64_t>(0, 0)));

        if(nodesAndBandwidths.find(flowId.destinationAddress) == nodesAndBandwidths.end())
            nodesAndBandwidths.insert(std::make_pair<Ipv4Address, std::pair<uint64_t, uint64_t> >(flowId.destinationAddress, std::make_pair<uint64_t, uint64_t>(0, 0)));

        for(addrIt = nodesAndBandwidths.begin(); addrIt != nodesAndBandwidths.end(); addrIt++){
            if(addrIt->first.IsEqual(flowId.sourceAddress)){
                addrIt->second.first += it->second.txBytes;
            }
            if(addrIt->first.IsEqual(flowId.destinationAddress)){
                addrIt->second.second += it->second.rxBytes;
            }
        }
    }

    for(addrIt = nodesAndBandwidths.begin(); addrIt != nodesAndBandwidths.end(); addrIt++){         //NOTE: network headers are not calculated into this average
        PRINT_RESULT("Average throughput for client " << addrIt->first << " downlink: "  << addrIt->second.second *8.0 / runningTime /1024 /1024 << "Mbps  "
                    << "uplink: " << addrIt->second.first *8.0/ runningTime / 1024 /1024 << "Mbps" << std::endl);
    }
}

#endif // STATISTICSCOLLECTOR_H
