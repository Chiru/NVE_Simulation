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
    static StatisticsCollector* createStatisticsCollector(bool, bool, bool);
    static void logMessagesSendFromClient(int messageNumber, Time);             //log times when user action messages are sent
    static void logMessageReceivedByServer(int messageNumber, Time);    //log times when user action messages are received by the server
    static void logMessageReceivedByClient(int messageNumber, Time);    //log times when user action messages are finally forwarded to other clients

private:
    StatisticsCollector(bool, bool, bool);
    static bool verbose;
    static bool clientLog;
    static bool serverLog;
    static bool collectorCreated;
    static std::vector<MessageStats*> messageLog;

};


//Class StatisticsCollector function definitions

bool StatisticsCollector::collectorCreated = false;
bool StatisticsCollector::verbose = false;
bool StatisticsCollector::clientLog = false;
bool StatisticsCollector::serverLog = false;
std::vector<StatisticsCollector::MessageStats*> StatisticsCollector::messageLog;

StatisticsCollector* StatisticsCollector::createStatisticsCollector(bool verbose, bool clientLog, bool serverLog){

    if(!collectorCreated)
        return new StatisticsCollector(verbose, clientLog, serverLog);

    else {
        PRINT_ERROR( "Already one StatisticsCollector exists." << std::endl);
        return NULL;
    }
}

StatisticsCollector::StatisticsCollector(bool verbose, bool clientLog, bool serverLog){

    StatisticsCollector::verbose = verbose;
    StatisticsCollector::clientLog = clientLog;
    StatisticsCollector::serverLog = serverLog;

}

StatisticsCollector::~StatisticsCollector(){

    uint32_t i = 0;
    uint64_t timeInMilliseconds = 0;
    Time transmitTimeAverage;
    std::list<Time>::const_iterator timeIter;

    for(std::vector<MessageStats*>::iterator it = messageLog.begin(); it != messageLog.end(); it++){
        //if(i == 1000){
            //std::cout << "Message number: " << (*it)->messageNumber << " Send time: " << (*it)->sendTime.GetSeconds() << " server recv time: " << (*it)->serverRecvTime.GetSeconds() << std::endl;
            for(timeIter = (*it)->clientRecvTimes.begin(); timeIter != (*it)->clientRecvTimes.end(); timeIter++){
               // std::cout << "\tClient receive time: " << clientReceives->GetSeconds() << std::endl;
                transmitTimeAverage += ((*timeIter) - (*it)->sendTime);
                i++;
            }
        delete *it;
    }

    timeInMilliseconds = transmitTimeAverage.ToInteger(Time::MS);
    std::cout << timeInMilliseconds <<  "  " << i <<std::endl;
    timeInMilliseconds /= i;

    std::cout << "Average transmit time: " <<  Time::FromInteger(timeInMilliseconds, Time::MS).GetMilliSeconds() << " milliseconds" << std::endl;

}

void StatisticsCollector::logMessageReceivedByClient(int messageNumber, Time recvTime){

    messageLog.at(messageNumber)->clientRecvTimes.push_back(recvTime);
}

void StatisticsCollector::logMessageReceivedByServer(int messageNumber, Time recvTime){

    messageLog.at(messageNumber)->serverRecvTime = recvTime;
}

void StatisticsCollector::logMessagesSendFromClient(int messageNumber, Time sendTime){

    messageLog.push_back(new MessageStats(messageNumber, sendTime));   //messageNumber is the same as the index of MessageStats in messageLog because they are sent in order
}


#endif // STATISTICSCOLLECTOR_H
