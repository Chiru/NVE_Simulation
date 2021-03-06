/**
* Copyright (c) 2013 Center for Internet Excellence, University of Oulu, All Rights Reserved
* For conditions of distribution and use, see copyright notice in license.txt
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
#include "RScriptGenerator.h"


class StatisticsCollector
{

    class MessageStats
    {
    public:
        MessageStats(int no, Time time, uint32_t clientRequirement, uint32_t serverRequirement, uint16_t nameIndex)
            : messageNumber(no), sendTime(time), serverRecvTime(Time("0ms")), clientRecvTimes(0),
              clientTimeRequirement(clientRequirement), serverTimeRequirement(serverRequirement),
              numberOfClientsForwarded(0), messageNameIndex(nameIndex){}

        int messageNumber;
        Time sendTime;
        Time serverRecvTime;
        std::list<Time> clientRecvTimes;
        Time sendTimeInterval;
        uint32_t clientTimeRequirement;
        uint32_t serverTimeRequirement;
        uint16_t numberOfClientsForwarded;
        uint16_t messageNameIndex;
        uint16_t messageSize;
        uint16_t forwardMessageSize;
    };

public:
    static std::string&(*uam_fnptr)(uint16_t);
    static std::string&(*odm_fnptr)(uint16_t);

    StatisticsCollector(bool, bool, bool, uint16_t, int);
    ~StatisticsCollector();
    static bool getVerbose() {return verbose;}
    static bool getClientLog() {return clientLog;}
    static bool getServerLog() {return serverLog;}
    void addFlowMonitor(Ptr<FlowMonitor> flowMon, FlowMonitorHelper& helper);
    void addClientInfo(const Ipv4Address &addr, int runningTime, int joinTime, int exitTime, bool pcap, bool graphs);
    void setServerAddress(const Ipv4Address& addr);
    void setServerPcap(bool pcap);
    void setStartTime(double time);
    static void logMessagesSentFromClient(int messageNumber, Time, uint16_t streamNumber, uint32_t clientTimeRequirement, uint32_t serverTimeRequirement,
                                          uint16_t messageNameIndex, uint16_t messageId, uint16_t size);//log times when user action messages are sent
    static void logUserActionMessageReceivedByServer(int messageNumber, Time, uint16_t streamNumber);    //log times when user action messages are received by the server
    static void logUserActionMessageReceivedByClient(int messageNumber, Time, uint16_t streamNumber);    //log times when user action messages are finally forwarded to other clients
    static void logServerMessageReceivedByClient(int messageNumber, Time, uint16_t streamNumber);        //log times when server messages reach client
    static void logMessageForwardedByServer(int messageNumber, uint16_t streamNumber, uint16_t size); //counts the messages forwarded to clients
    static void logMessagesSentFromServer(int messageNumber, Time, uint16_t streamNumber, uint32_t clientTimeRequirement, uint16_t messageNameIndex,
                                          uint16_t messageId, uint16_t size);
    static void countMessagesSentFromServer(int messageNumber, uint16_t streamNumber);
    static uint16_t userActionmessageCount;
    static uint16_t otherDataMessageCount;

private:
    void getStreamResults(std::vector<StatisticsCollector::MessageStats*>& clientStats, std::vector<StatisticsCollector::MessageStats*>& serverStats,
                          uint16_t streamNumber, Time& clientToClientTimeResult, Time& clientToServerTimeResult, Time& serverToClientTimeResult,
                          uint32_t& clientToClientMsgCount, uint32_t& clientToServerMsgCount, uint32_t& fromServerToClientCount,
                          uint32_t& toServerInTime, uint32_t& formClientToClientInTime, uint32_t& fromServerToClientInTime,
                          std::list<int64_t>& clientToServerTimes,  std::list<int64_t>& clientToClientTimes, std::list<int64_t>& serverToClientTimes);
    void getBandwidthResults();
    void getMessageStats(uint16_t streamNumber);
    bool generateSingleNodeStatsFromPcap();
    bool generateOverallGraphFromPcap();

    uint16_t streamCount;
    Ptr<FlowMonitor> flowMon;
    FlowMonitorHelper helper;
    int runningTime;
    double startTime; //time of the first packet arriving at the server
    bool serverPcap;
    RScriptGenerator* scriptGen;
    std::string scriptSourceFile;
    std::string scriptResultPdfFile;
    std::string scriptResultTextFile;


    //this struct is needed since clients have to be deleted before statistics collector
    struct ClientInfo
    {
        int joinTime;
        int exitTime;
        int runningTime;
        bool pcap;
        bool graph;
    };

    std::map<Ipv4Address, ClientInfo> clientsInformation;

    Ipv4Address serverAddr;

    static bool verbose;
    static bool clientLog;
    static bool serverLog;
    static std::vector<MessageStats*> *userActionMessageLog;
    static std::vector<MessageStats*> *serverMessageLog;

};


#endif // STATISTICSCOLLECTOR_H

