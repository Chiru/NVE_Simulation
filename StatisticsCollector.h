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
#include <list>
#include "ns3/nstime.h"
#include "utilities.h"


class StatisticsCollector{

    class MessageStats{
    public:
        MessageStats(int no, Time time): messageNumber(no), sendTime(time), serverRecvTime(0), clientRecvTimes(0){}
    //private:
        int messageNumber;
        Time sendTime;
        Time serverRecvTime;
        std::list<Time> clientRecvTimes;
    };

public:
    ~StatisticsCollector();
    static bool getVerbose() {return verbose;}
    static bool getClientLog() {return clientLog;}
    static bool getServerLog() {return serverLog;}
    static StatisticsCollector* createStatisticsCollector(bool, bool, bool, uint16_t);
    static void logMessagesSendFromClient(int messageNumber, Time, uint16_t streamNumber);             //log times when user action messages are sent
    static void logMessageReceivedByServer(int messageNumber, Time, uint16_t streamNumber);    //log times when user action messages are received by the server
    static void logMessageReceivedByClient(int messageNumber, Time, uint16_t streamNumber);    //log times when user action messages are finally forwarded to other clients

private:
    StatisticsCollector(bool, bool, bool, uint16_t);
    void getResults(std::vector<StatisticsCollector::MessageStats*>& stats, uint16_t streamNumber, Time& clientTimeResult, Time& serverTimeResult);
    uint16_t streamCount;
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

StatisticsCollector* StatisticsCollector::createStatisticsCollector(bool verbose, bool clientLog, bool serverLog, uint16_t streamNumber){

    if(!collectorCreated)
        return new StatisticsCollector(verbose, clientLog, serverLog, streamNumber);

    else {
        PRINT_ERROR( "Already one StatisticsCollector exists." << std::endl);
        return NULL;
    }
}

StatisticsCollector::StatisticsCollector(bool verbose, bool clientLog, bool serverLog, uint16_t numberOfStreams): streamCount(numberOfStreams){

    StatisticsCollector::verbose = verbose;
    StatisticsCollector::clientLog = clientLog;
    StatisticsCollector::serverLog = serverLog;

    messageLog = new std::vector<StatisticsCollector::MessageStats*>[numberOfStreams];
}

StatisticsCollector::~StatisticsCollector(){

    uint32_t clientMsgCount = 0, serverMsgCount = 0;
    uint64_t timeInMilliseconds = 0;
    Time averageClientToServer("0ms");
    Time averageClientToClient("0ms");
    Time singleStreamClientToServer("0ms");
    Time singleStreamClientToClient("0ms");


    for(int h = 0; h < streamCount; h++){
        getResults(messageLog[h], h+1, singleStreamClientToClient, singleStreamClientToServer);
        averageClientToServer += singleStreamClientToServer;
        if(!singleStreamClientToServer.IsZero()){
               serverMsgCount++;
               singleStreamClientToServer = Time::FromInteger(0, Time::MS);
        }

        averageClientToClient += singleStreamClientToClient;
        if(!singleStreamClientToClient.IsZero()){
            clientMsgCount++;
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


    std::cout << "Overall average transmit times   clientToServer: " <<  averageClientToServer << "   clientToClient: " << averageClientToClient << "  in milliseconds" << std::endl;

    delete[] messageLog;

}

void StatisticsCollector::logMessageReceivedByClient(int messageNumber, Time recvTime, uint16_t streamNumber){

    messageLog[streamNumber-1].at(messageNumber)->clientRecvTimes.push_back(recvTime);
}

void StatisticsCollector::logMessageReceivedByServer(int messageNumber, Time recvTime, uint16_t streamNumber){

    messageLog[streamNumber-1].at(messageNumber)->serverRecvTime = recvTime;
}

void StatisticsCollector::logMessagesSendFromClient(int messageNumber, Time sendTime, uint16_t streamNumber){

    messageLog[streamNumber-1].push_back(new MessageStats(messageNumber, sendTime));   //messageNumber is the same as the index of MessageStats in messageLog because they are sent in order
}

void StatisticsCollector::getResults(std::vector<StatisticsCollector::MessageStats*>& stats, uint16_t streamnumber, Time& clientTimeResult, Time& serverTimeResult){

    uint32_t i = 0, h = 0;
    uint64_t timeInMilliseconds = 0;
    std::list<Time>::const_iterator timeIter;

    for(std::vector<StatisticsCollector::MessageStats*>::iterator it = stats.begin(); it != stats.end(); it++, h++){
        for(timeIter = (*it)->clientRecvTimes.begin(); timeIter != (*it)->clientRecvTimes.end(); timeIter++){
            clientTimeResult += (*timeIter);
            i++;
        }
        serverTimeResult += (*it)->serverRecvTime;
    }

    if(i != 0){
        timeInMilliseconds = clientTimeResult.ToInteger(Time::MS);
        timeInMilliseconds /= i;
        clientTimeResult = Time::FromInteger(timeInMilliseconds, Time::MS);
    }else
        clientTimeResult =  Time::FromInteger(0, Time::MS);

    if(h != 0){
        timeInMilliseconds = serverTimeResult.ToInteger(Time::MS);
        timeInMilliseconds /= h;
        serverTimeResult = Time::FromInteger(timeInMilliseconds, Time::MS);
    }else
        serverTimeResult = Time::FromInteger(0, Time::MS);

    std::cout << "Average transmit times for stream number: " << streamnumber << "  clientToServer: " << serverTimeResult << "  clientToClient: " << clientTimeResult << std::endl;

}

#endif // STATISTICSCOLLECTOR_H
