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
#include "ns3/system-mutex.h"
#include "utilities.h"


class StatisticsCollector{

    class MessageStats{
    public:
        MessageStats(int no, Time time): messageNumber(no), sendTime(time){}
    //private:
        int messageNumber;
        Time sendTime;
    };

public:

    ~StatisticsCollector();
    static bool getVerbose() {return verbose;}
    static bool getClientLog() {return clientLog;}
    static bool getServerLog() {return serverLog;}
    static StatisticsCollector* createStatisticsCollector(bool, bool, bool);
    static void logMessageSendClient(int messageNumber, Time);             //log times when user action messages are sent
    static void logMessageReceiveServer(int messageNumber, Time);    //log times when user action messages are received by the server
    static void logMessageReceiveClient(int messageNumber, Time);    //log times when user action messages are finally forwarded to other clients

private:
    StatisticsCollector(bool, bool, bool);
    static bool verbose;
    static bool clientLog;
    static bool serverLog;
    static bool collectorCreated;
    static std::vector<MessageStats*> messageLog;
    static SystemMutex mutex;

};



//Class StatisticsCollector function definitions

bool StatisticsCollector::collectorCreated = false;
bool StatisticsCollector::verbose = false;
bool StatisticsCollector::clientLog = false;
bool StatisticsCollector::serverLog = false;
SystemMutex StatisticsCollector::mutex;
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

    for(std::vector<MessageStats*>::iterator it = messageLog.begin(); it != messageLog.end(); it++){
        std::cout << (*it)->messageNumber << " " << (*it)->sendTime << std::endl;
        delete *it;
    }
}

void StatisticsCollector::logMessageReceiveClient(int messageNumber, Time recvTime){


}

void StatisticsCollector::logMessageReceiveServer(int messageNumber, Time recvTime){


}

void StatisticsCollector::logMessageSendClient(int messageNumber, Time sendTime){

    mutex.Lock();
    messageLog.push_back(new MessageStats(messageNumber, sendTime));
    mutex.Unlock();
}


#endif // STATISTICSCOLLECTOR_H
