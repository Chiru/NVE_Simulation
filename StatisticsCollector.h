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
#include "RScriptGenerator.h"


class StatisticsCollector{

    class MessageStats{
    public:
        MessageStats(int no, Time time, uint32_t clientRequirement, uint32_t serverRequirement, uint16_t nameIndex): messageNumber(no), sendTime(time), serverRecvTime(Time("0ms")), clientRecvTimes(0),
            clientTimeRequirement(clientRequirement), serverTimeRequirement(serverRequirement), numberOfClientsForwarded(0), messageNameIndex(nameIndex){}
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

    ~StatisticsCollector();
    static bool getVerbose() {return verbose;}
    static bool getClientLog() {return clientLog;}
    static bool getServerLog() {return serverLog;}
    void addFlowMonitor(Ptr<FlowMonitor> flowMon, FlowMonitorHelper& helper);
    static StatisticsCollector* createStatisticsCollector(bool, bool, bool, uint16_t, int);
    static void logMessagesSentFromClient(int messageNumber, Time, uint16_t streamNumber, uint32_t clientTimeRequirement, uint32_t serverTimeRequirement,
                                          uint16_t messageNameIndex, uint16_t messageId, uint16_t size);//log times when user action messages are sent
    static void logUserActionMessageReceivedByServer(int messageNumber, Time, uint16_t streamNumber);    //log times when user action messages are received by the server
    static void logUserActionMessageReceivedByClient(int messageNumber, Time, uint16_t streamNumber);    //log times when user action messages are finally forwarded to other clients
    static void logServerMessageReceivedByClient(int messageNumber, Time, uint16_t streamNumber);        //log times when server messages reach client
    static void logMessageForwardedByServer(int messageNumber, uint16_t streamNumber, uint16_t size); //counts the messages forwarded to clients
    static void logMessagesSentFromServer(int messageNumber, Time, uint16_t streamNumber, uint32_t clientTimeRequirement, uint16_t messageNameIndex, uint16_t messageId, uint16_t size);
    static void countMessagesSentFromServer(int messageNumber, uint16_t streamNumber);
    static void updateMessageTimeIntervalSentFromClient(int messageNumber, uint16_t streamNumber, Time time);  //this is used for updating the sending time when gametick is used and sending is not immediate
    static void updateMessageTimeIntervalSentFromServer(int messageNumber, uint16_t streamNumber, Time time);  //this is used for updating the sending time when gametick is used and sending is not immediate
    static uint16_t userActionmessageCount;
    static uint16_t otherDataMessageCount;

private:
    StatisticsCollector(bool, bool, bool, uint16_t, int);
    void getStreamResults(std::vector<StatisticsCollector::MessageStats*>& clientStats, std::vector<StatisticsCollector::MessageStats*>& serverStats, uint16_t streamNumber, Time& clientToClientTimeResult,
                          Time& clientToServerTimeResult, Time& serverToClientTimeResult, uint32_t& clientToClientMsgCount,
                          uint32_t& clientToServerMsgCount, uint32_t& fromServerToClientCount, uint32_t& toServerInTime, uint32_t& formClientToClientInTime, uint32_t& fromServerToClientInTime,
                          std::list<int64_t>& clientToServerTimes,  std::list<int64_t>& clientToClientTimes, std::list<int64_t>& serverToClientTimes);
    void getBandwidthResults();
    void getMessageStats(uint16_t streamNumber);

    uint16_t streamCount;
    Ptr<FlowMonitor> flowMon;
    FlowMonitorHelper helper;
    int runningTime;
    RScriptGenerator* scriptGen;
    std::string scriptSourceFile;
    std::string scriptResultPdfFile;
    std::string scriptResultTextFile;

    static bool verbose;
    static bool clientLog;
    static bool serverLog;
    static bool collectorCreated;
    static std::vector<MessageStats*> *userActionMessageLog;
    static std::vector<MessageStats*> *serverMessageLog;

};


#endif // STATISTICSCOLLECTOR_H

