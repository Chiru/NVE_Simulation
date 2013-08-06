#include "StatisticsCollector.h"
#include <sys/stat.h>
#include <fcntl.h>

//Class StatisticsCollector function definitions

bool StatisticsCollector::verbose = false;
bool StatisticsCollector::clientLog = false;
bool StatisticsCollector::serverLog = false;
std::vector<StatisticsCollector::MessageStats*>* StatisticsCollector::userActionMessageLog;
std::vector <StatisticsCollector::MessageStats*>* StatisticsCollector::serverMessageLog;
std::string&(*StatisticsCollector::uam_fnptr)(uint16_t) = 0;
std::string&(*StatisticsCollector::odm_fnptr)(uint16_t) = 0;
uint16_t StatisticsCollector::userActionmessageCount = 0;
uint16_t StatisticsCollector::otherDataMessageCount = 0;


StatisticsCollector::StatisticsCollector(bool verbose, bool clientLog, bool serverLog, uint16_t numberOfStreams, int runningTime): streamCount(numberOfStreams), runningTime(runningTime),
    scriptSourceFile("results/rscriptfile.R"), scriptResultPdfFile("results/resultgraphs.pdf"), scriptResultTextFile("results/results.txt"){

    StatisticsCollector::verbose = verbose;
    StatisticsCollector::clientLog = clientLog;
    StatisticsCollector::serverLog = serverLog;

    userActionMessageLog = new std::vector<StatisticsCollector::MessageStats*>[numberOfStreams];
    serverMessageLog = new std::vector<StatisticsCollector::MessageStats*>[numberOfStreams];

    scriptGen = new RScriptGenerator(scriptSourceFile, scriptResultPdfFile, scriptResultTextFile);

}

StatisticsCollector::~StatisticsCollector(){

    uint32_t clientToClientMsgCount = 0, clientToServerMsgCount = 0, serverToClientMsgCount = 0;   //these are the messages that actually reach their destination, lost packets ignored
    uint32_t sentFromClient = 0, sentFromServer = 0, forwardedFromServer = 0;                      //these are the sent messages, lost packets also considered
    uint32_t toServerInTime = 0, fromClientToClientInTime = 0, fromServerToClientInTime = 0;
    double averageServerInTime = 0, averageClientToClientInTime = 0, averageServerToClientInTime = 0;
    uint64_t timeInMilliseconds = 0;
    Time averageClientToServer("0ms");
    Time averageClientToClient("0ms");
    Time averageServerToClient("0ms");
    Time singleStreamClientToServer("0ms");
    Time singleStreamClientToClient("0ms");
    Time singleStreamServerToClient("0ms");
    std::list<int64_t> clientToClientTimes[streamCount];
    std::list<int64_t> clientToServerTimes[streamCount];
    std::list<int64_t> serverToClientTimes[streamCount];

    for(int h = 0; h < streamCount; h++){
        getStreamResults(userActionMessageLog[h], serverMessageLog[h], h+1, singleStreamClientToClient, singleStreamClientToServer,singleStreamServerToClient, clientToClientMsgCount,
                         clientToServerMsgCount,serverToClientMsgCount, toServerInTime, fromClientToClientInTime, fromServerToClientInTime, clientToServerTimes[h], clientToClientTimes[h],
                         serverToClientTimes[h]);

        averageClientToServer += singleStreamClientToServer;
        if(!singleStreamClientToServer.IsZero()){
               singleStreamClientToServer = Time::FromInteger(0, Time::MS);
        }

        averageClientToClient += singleStreamClientToClient;
        if(!singleStreamClientToClient.IsZero()){
            singleStreamClientToClient = Time::FromInteger(0, Time::MS);
        }

        averageServerToClient += singleStreamServerToClient;
        if(!singleStreamServerToClient.IsZero()){
            singleStreamServerToClient = Time::FromInteger(0, Time::MS);
        }

        getMessageStats(h);

        for(std::vector<StatisticsCollector::MessageStats*>::iterator it = userActionMessageLog[h].begin(); it != userActionMessageLog[h].end(); it++){
            sentFromClient++;
            forwardedFromServer += (*it)->numberOfClientsForwarded;
        }

        for(std::vector<StatisticsCollector::MessageStats*>::iterator it = serverMessageLog[h].begin(); it != serverMessageLog[h].end(); it++){
            sentFromServer += (*it)->numberOfClientsForwarded;
        }

    }

    scriptGen->generateScriptForStream(clientToClientTimes, clientToServerTimes, serverToClientTimes, streamCount);

    if(clientToServerMsgCount != 0){
        timeInMilliseconds = averageClientToServer.ToInteger(Time::MS);
        timeInMilliseconds /= clientToServerMsgCount;
        averageClientToServer = Time::FromInteger(timeInMilliseconds, Time::MS);
        averageServerInTime = static_cast<double>(toServerInTime)/static_cast<double>(sentFromClient);
    }else
        averageClientToServer = Time::FromInteger(0, Time::MS);

    if(clientToClientMsgCount != 0){
        timeInMilliseconds = averageClientToClient.ToInteger(Time::MS);
        timeInMilliseconds /= clientToClientMsgCount;
        averageClientToClient = Time::FromInteger(timeInMilliseconds, Time::MS);
        averageClientToClientInTime = static_cast<double>(fromClientToClientInTime)/static_cast<double>(forwardedFromServer);
    }else
        averageClientToClient = Time::FromInteger(0, Time::MS);

    if(serverToClientMsgCount != 0){
        timeInMilliseconds = averageServerToClient.ToInteger(Time::MS);
        timeInMilliseconds /= serverToClientMsgCount;
        averageServerToClient = Time::FromInteger(timeInMilliseconds, Time::MS);
        averageServerToClientInTime = static_cast<double>(fromServerToClientInTime)/static_cast<double>(sentFromServer);
    }else
        averageServerToClient = Time::FromInteger(0, Time::MS);

    for(int h = 0; h < streamCount; h++){
        for(std::vector<MessageStats*>::iterator it = userActionMessageLog[h].begin(); it != userActionMessageLog[h].end(); it++){
            delete *it;
        }

        for(std::vector<MessageStats*>::iterator it = serverMessageLog[h].begin(); it != serverMessageLog[h].end(); it++){
            delete *it;
        }
    }


    PRINT_RESULT("Overall stats: "  <<"\n\tAverage transmit times   clientToServer: " <<  averageClientToServer.GetMilliSeconds() << "   clientToClient: "
                 << averageClientToClient.GetMilliSeconds() << "   serverToClient: " << averageServerToClient.GetMilliSeconds()  << "  (in milliseconds)"
                 << "\n\tAverage percentage of messages to reach server in time:" <<  averageServerInTime << "\n\tAverage percentage of messages from client to reach client in time: "
                 << averageClientToClientInTime << "\n\tAverage percentage of messages from server to reach client in time: " << averageServerToClientInTime << std::endl);

    generateSingleNodeStatsFromPcap();
    generateOverallGraphFromPcap();

    getBandwidthResults();

    if(scriptGen->writeAndExecuteResultScript())
        PRINT_RESULT(std::endl << "Generated a result file for graphs: " << scriptResultPdfFile  << std::endl);
    else
        PRINT_RESULT(std::endl << "Could not generate a result file for graphs." << std::endl);

    delete[] userActionMessageLog;
    delete[] serverMessageLog;
    delete scriptGen;

}

void StatisticsCollector::logUserActionMessageReceivedByClient(int messageNumber, Time recvTime, uint16_t streamNumber){

    userActionMessageLog[streamNumber-1].at(messageNumber)->clientRecvTimes.push_back(recvTime);

}

void StatisticsCollector::logServerMessageReceivedByClient(int messageNumber, Time recvTime, uint16_t streamNumber){
    serverMessageLog[streamNumber - 1].at(messageNumber)->clientRecvTimes.push_back(recvTime);
}

void StatisticsCollector::logUserActionMessageReceivedByServer(int messageNumber, Time recvTime, uint16_t streamNumber){
    userActionMessageLog[streamNumber-1].at(messageNumber)->serverRecvTime = recvTime;

}

void StatisticsCollector::logMessagesSentFromClient(int messageNumber, Time sendTime, uint16_t streamNumber, uint32_t clientTimeRequirement, uint32_t serverTimeRequirement, uint16_t nameIndex,
                                                    uint16_t messageId, uint16_t size){

    static std::map<int, Time> lastTimes;

    userActionMessageLog[streamNumber-1].push_back(new MessageStats(messageNumber, sendTime, clientTimeRequirement, serverTimeRequirement, nameIndex));
                                                //messageNumber is the same as the index of MessageStats in messageLog because they are sent in order

    if(!lastTimes.count(messageId)){
        lastTimes.insert(std::make_pair<int, Time>(messageId, sendTime));
    }
    else{
        userActionMessageLog[streamNumber-1].back()->sendTimeInterval = sendTime - lastTimes[messageId];
        lastTimes[messageId] = sendTime;
    }

    userActionMessageLog[streamNumber-1].back()->messageSize = size;


}

void StatisticsCollector::logMessagesSentFromServer(int messageNumber, Time sendTime, uint16_t streamNumber, uint32_t clientTimeRequirement, uint16_t messageNameIndex, uint16_t messageId, uint16_t size){
    static std::map<int, Time> lastTimes;

    serverMessageLog[streamNumber - 1].push_back(new MessageStats(messageNumber, sendTime, clientTimeRequirement, 0, messageNameIndex));

    if(!lastTimes.count(messageId)){
        serverMessageLog[streamNumber - 1].back()->sendTimeInterval = sendTime; //TODO: remove?
        lastTimes.insert(std::make_pair<int, Time>(messageId, sendTime));
    }
    else{
        serverMessageLog[streamNumber-1].back()->sendTimeInterval = sendTime - lastTimes[messageId];
        lastTimes[messageId] = sendTime;
    }

    serverMessageLog[streamNumber-1].back()->messageSize = size;
}

void StatisticsCollector::countMessagesSentFromServer(int messageNumber, uint16_t streamNumber){
    serverMessageLog[streamNumber - 1].at(messageNumber)->numberOfClientsForwarded++;
}

void StatisticsCollector::logMessageForwardedByServer(int messageNumber, uint16_t streamNumber, uint16_t forwardMessageSize){
    userActionMessageLog[streamNumber-1].at(messageNumber)->numberOfClientsForwarded++;
    userActionMessageLog[streamNumber-1].at(messageNumber)->forwardMessageSize = forwardMessageSize;
}


void StatisticsCollector::getStreamResults(std::vector<StatisticsCollector::MessageStats*>& clientStats, std::vector<StatisticsCollector::MessageStats*>& serverStats,
                                           uint16_t streamnumber, Time& clientToClientTimeResult, Time& clientToServerTimeResult, Time& serverToClientTimeResult, uint32_t& clientToClientMsgCount,
                                           uint32_t& clientToServerMsgCount, uint32_t& serverToClientMsgCount,  uint32_t& toServerInTime, uint32_t& fromClientToClientInTime,
                                           uint32_t& fromServerToClientInTime,std::list<int64_t>& clientToServerTimes, std::list<int64_t>& clientToClientTimes, std::list<int64_t>& serverToClientTimes){

    uint32_t tempClientToClientMsgCount = 0, tempServerMsgCount = 0, tempServerToClientMsgCount = 0;
    uint32_t tempServerInTime = 0, tempClientToClientInTime = 0, tempServerToClientInTime = 0;
    uint64_t timeInMilliseconds = 0;
    uint32_t messagesSentFromClient = clientStats.size();
    uint32_t messagesSentFromServer = 0;
    uint32_t messagesForwardedFromServer = 0;
    double clientToClientPercentage = 0, clientToServerPercentage = 0, serverToClientPercentage = 0;
    std::list<Time>::const_iterator timeIter;
    Time clientToClientStreamTime("0ms");
    Time serverStreamTime("0ms");
    Time serverToClientStreamTime("0ms");

    for(std::vector<StatisticsCollector::MessageStats*>::iterator it = clientStats.begin(); it != clientStats.end(); it++){
        for(timeIter = (*it)->clientRecvTimes.begin(); timeIter != (*it)->clientRecvTimes.end(); timeIter++){
            if(!(*timeIter).IsZero()){
                clientToClientTimeResult += ((*timeIter) - (*it)->sendTime);
                clientToClientTimes.push_back(((*timeIter) - (*it)->sendTime).GetMilliSeconds());
                if(((*timeIter) - (*it)->sendTime).GetMilliSeconds() <= (*it)->clientTimeRequirement)
                    tempClientToClientInTime++;

                tempClientToClientMsgCount++;
            }
        }

        if(!(*it)->serverRecvTime.IsZero()){
            clientToServerTimeResult += ((*it)->serverRecvTime - (*it)->sendTime);
            clientToServerTimes.push_back(((*it)->serverRecvTime - (*it)->sendTime).GetMilliSeconds());
            if(((*it)->serverRecvTime - (*it)->sendTime).GetMilliSeconds() <= (*it)->serverTimeRequirement)
                tempServerInTime++;

            tempServerMsgCount++;
        }

        messagesForwardedFromServer += (*it)->numberOfClientsForwarded;
    }

    for(std::vector<StatisticsCollector::MessageStats*>::iterator it = serverStats.begin(); it != serverStats.end(); it++){
        for(timeIter = (*it)->clientRecvTimes.begin(); timeIter != (*it)->clientRecvTimes.end(); timeIter++){
            if(!(*timeIter).IsZero()){
                serverToClientTimeResult += ((*timeIter) - (*it)->sendTime);
                serverToClientTimes.push_back(((*timeIter) - (*it)->sendTime).GetMilliSeconds());
                if(((*timeIter) - (*it)->sendTime).GetMilliSeconds() <= (*it)->clientTimeRequirement)
                    tempServerToClientInTime++;

                tempServerToClientMsgCount++;
            }

        }

        messagesSentFromServer += (*it)->numberOfClientsForwarded;

    }

    if(tempClientToClientMsgCount != 0){
        timeInMilliseconds = clientToClientTimeResult.ToInteger(Time::MS);
        timeInMilliseconds /= tempClientToClientMsgCount;
        clientToClientStreamTime = Time::FromInteger(timeInMilliseconds, Time::MS);
    }else
        clientToClientStreamTime =  Time::FromInteger(0, Time::MS);

    if(tempServerMsgCount != 0){
        timeInMilliseconds = clientToServerTimeResult.ToInteger(Time::MS);
        timeInMilliseconds /= tempServerMsgCount;
        serverStreamTime = Time::FromInteger(timeInMilliseconds, Time::MS);
    }else
        serverStreamTime = Time::FromInteger(0, Time::MS);

    if(tempServerToClientMsgCount != 0){
        timeInMilliseconds = serverToClientTimeResult.ToInteger((Time::MS));
        timeInMilliseconds /= tempServerToClientMsgCount;
        serverToClientStreamTime = Time::FromInteger(timeInMilliseconds, Time::MS);
    }else
        serverToClientStreamTime = Time::FromInteger(0, Time::MS);

    clientToClientMsgCount += tempClientToClientMsgCount;
    serverToClientMsgCount += tempServerToClientMsgCount;
    toServerInTime += tempServerInTime;
    fromClientToClientInTime += tempClientToClientInTime;
    fromServerToClientInTime += tempServerToClientInTime;
    clientToServerMsgCount += tempServerMsgCount;


    if(messagesForwardedFromServer != 0)
        clientToClientPercentage = static_cast<double>(tempClientToClientInTime) / static_cast<double>(messagesForwardedFromServer);
    else
        clientToClientPercentage = -1;

    if(messagesSentFromClient != 0)
        clientToServerPercentage = static_cast<double>(tempServerInTime) / static_cast<double>(messagesSentFromClient);
    else
        clientToServerPercentage = -1;

    if(messagesSentFromServer != 0)
        serverToClientPercentage = static_cast<double>(tempServerToClientInTime) / static_cast<double>(messagesSentFromServer);


    PRINT_RESULT("Average transmit times for stream number: " << streamnumber << std::endl);
    PRINT_RESULT("\tclientToServer: ");
    if(clientToServerPercentage == -1)
        PRINT_RESULT("N/A" << std::endl);
    else
        PRINT_RESULT(serverStreamTime.GetMilliSeconds() << std::endl);

    PRINT_RESULT("\tclientToClient: ");
    if(clientToClientPercentage == -1)
        PRINT_RESULT("N/A" << std::endl);
    else
        PRINT_RESULT(clientToClientStreamTime.GetMilliSeconds() << std::endl);

    PRINT_RESULT("\tserverToClient: ");
    if(serverToClientPercentage == -1)
        PRINT_RESULT("N/A" << std::endl);
    else
        PRINT_RESULT(serverToClientStreamTime.GetMilliSeconds() << std::endl);

    PRINT_RESULT("\tPercentage of packets to reach server in time: ");
    if(clientToServerPercentage == -1)
        PRINT_RESULT("N/A" << std::endl);
    else
        PRINT_RESULT(clientToServerPercentage << std::endl);

    PRINT_RESULT("\tPercentage of packets from client to reach client in time: ");
    if(clientToClientPercentage == -1)
        PRINT_RESULT("N/A" << std::endl);
    else
        PRINT_RESULT(clientToClientPercentage << std::endl);

    PRINT_RESULT("\tPercentage of packets from server to reach client in time: ");
    if(serverToClientPercentage == -1)
        PRINT_RESULT("N/A" << std::endl);
    else
        PRINT_RESULT(serverToClientPercentage << std::endl);

}

void StatisticsCollector::addFlowMonitor(Ptr<FlowMonitor> flowMon, FlowMonitorHelper& helper){

    this->flowMon = flowMon;
    this->helper = helper;

    this->flowMon->Start(Time("0ms"));
    this->flowMon->Stop(Time(runningTime));
}

void StatisticsCollector::addClientRunningTime(const Ipv4Address &addr, int runningTime)
{
    clientRunningTimes.insert(std::pair<Ipv4Address, int>(addr, runningTime));
}


void StatisticsCollector::setServerAddress(const Ipv4Address &addr)
{
    serverAddr = addr;
}

void StatisticsCollector::getBandwidthResults(){

    std::map<FlowId, FlowMonitor::FlowStats> flowStats;
    Ipv4FlowClassifier::FiveTuple flowId;
    std::map<Ipv4Address, std::pair<uint64_t, uint64_t> >nodesAndBandwidths;   //received and sent data for each address
    std::map<Ipv4Address, std::pair<uint32_t, uint32_t> >nodesAndPackets;      //received and sent packets for each address
    std::map<Ipv4Address, std::pair<uint64_t, uint64_t> >::iterator addrIt;
    std::map<Ipv4Address, std::pair<uint64_t, uint64_t> >::const_reverse_iterator revAddrIt;
    double averageClientUplink = 0, averageClientDownlink = 0, serverUplink = 0, serverDownlink = 0;
    int counter;
    int runningTime;

    //flowMon->CheckForLostPackets();
    Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (helper.GetClassifier());
    flowStats = flowMon->GetFlowStats();
    for(std::map<FlowId, FlowMonitor::FlowStats>::const_iterator it = flowStats.begin(); it != flowStats.end(); it++){
        flowId = classifier->FindFlow(it->first);

        if(nodesAndBandwidths.find(flowId.sourceAddress) == nodesAndBandwidths.end()){
            nodesAndBandwidths.insert(std::make_pair<Ipv4Address, std::pair<uint64_t, uint64_t> >(flowId.sourceAddress, std::make_pair<uint64_t, uint64_t>(0, 0)));
            nodesAndPackets.insert(std::make_pair<Ipv4Address, std::pair<uint64_t, uint64_t> >(flowId.sourceAddress, std::make_pair<uint32_t, uint32_t>(0,0)));
        }

        if(nodesAndBandwidths.find(flowId.destinationAddress) == nodesAndBandwidths.end()){
            nodesAndBandwidths.insert(std::make_pair<Ipv4Address, std::pair<uint64_t, uint64_t> >(flowId.destinationAddress, std::make_pair<uint64_t, uint64_t>(0, 0)));
            nodesAndPackets.insert(std::make_pair<Ipv4Address, std::pair<uint64_t, uint64_t> >(flowId.destinationAddress, std::make_pair<uint32_t, uint32_t>(0,0)));
        }

        for(addrIt = nodesAndBandwidths.begin(); addrIt != nodesAndBandwidths.end(); addrIt++){
            if(addrIt->first.IsEqual(flowId.sourceAddress)){
                addrIt->second.first += it->second.txBytes;
                nodesAndPackets[addrIt->first].first = it->second.txPackets;
            }
            if(addrIt->first.IsEqual(flowId.destinationAddress)){
                addrIt->second.second += it->second.rxBytes;
                nodesAndPackets[addrIt->first].second = it->second.rxPackets;;

            }
        }
    }

    revAddrIt = nodesAndBandwidths.rbegin(); //this points to the server node



    for(addrIt = nodesAndBandwidths.begin(), counter = 0; addrIt != nodesAndBandwidths.end(); addrIt++){ //NOTE: network headers are not calculated into this average

        double upLink = 0;
        double downLink = 0;

        if(addrIt->first != revAddrIt->first){
            runningTime = clientRunningTimes[addrIt->first];
            averageClientUplink += (upLink = (addrIt->second.first + (nodesAndPackets[addrIt->first].first * 12))*8.0 / runningTime / 1024 /1024);
            averageClientDownlink += (downLink = (addrIt->second.second + (nodesAndPackets[addrIt->first].second * 12))*8.0 / runningTime /1024 /1024);
            counter++;
        }
        else{
            runningTime = this->runningTime;
            serverUplink +=  (upLink = (addrIt->second.first + (nodesAndPackets[addrIt->first].first * 12))*8.0/ runningTime / 1024 /1024);
            serverDownlink += (downLink = (addrIt->second.second + (nodesAndPackets[addrIt->first].second * 12))*8.0 / runningTime /1024 /1024);
        }

        PRINT_RESULT((*addrIt == *revAddrIt ? "Average throughput for server " : "Average throughput for client ") << addrIt->first << " downlink: "  << downLink
                     << "Mbps  " << "uplink: " << upLink << "Mbps" << std::endl);

        scriptGen->addClientBandwidth(addrIt->first, downLink, upLink, (*addrIt != *revAddrIt));

    }

    scriptGen->generateBandwidthHistogram(averageClientDownlink / counter, averageClientUplink / counter, serverDownlink, serverUplink);
}


void StatisticsCollector::getMessageStats(uint16_t streamNumber){

    std::pair<std::pair<std::string, int>, std::list<int> > messageRecvTimesForClientToClient[userActionmessageCount];
    std::pair<std::pair<std::string, int>, std::list<int> > messageRecvTimesForServer[userActionmessageCount];
    std::pair<std::pair<std::string, int>, std::list<int> > messageRecvTimesFromServerToClient[otherDataMessageCount];
    std::pair<std::string, std::list<uint16_t> > uamSizes[userActionmessageCount];
    std::pair<std::string, std::list<uint16_t> >odmSizes[otherDataMessageCount];
    std::list<int> messageSendIntervalsFromClient[userActionmessageCount];
    std::list<int> messagesSendIntervalFromServer[otherDataMessageCount];
    uint32_t* messagesForwardedFromServer = (uint32_t*)calloc(userActionmessageCount, sizeof(uint32_t));
    uint32_t* messagesSentFromServer =(uint32_t*)calloc(otherDataMessageCount, sizeof(uint32_t));

        for(std::vector<MessageStats*>::iterator it = userActionMessageLog[streamNumber].begin(); it != userActionMessageLog[streamNumber].end(); it++){
            messageRecvTimesForServer[(*it)->messageNameIndex].first.first =  uam_fnptr((*it)->messageNameIndex);
            messageRecvTimesForClientToClient[(*it)->messageNameIndex].first.first =  uam_fnptr((*it)->messageNameIndex);
            uamSizes[(*it)->messageNameIndex].first = uam_fnptr((*it)->messageNameIndex);

            messageRecvTimesForServer[(*it)->messageNameIndex].first.second = (*it)->serverTimeRequirement;
            messageRecvTimesForClientToClient[(*it)->messageNameIndex].first.second = (*it)->clientTimeRequirement;
            messagesForwardedFromServer[(*it)->messageNameIndex] += (*it)->numberOfClientsForwarded;
            messageSendIntervalsFromClient[(*it)->messageNameIndex].push_back((*it)->sendTimeInterval.GetMilliSeconds());
            uamSizes[(*it)->messageNameIndex].second.push_back((*it)->messageSize);

            if(!(*it)->serverRecvTime.IsZero())
                messageRecvTimesForServer[(*it)->messageNameIndex].second.push_back(((*it)->serverRecvTime - (*it)->sendTime).GetMilliSeconds());

            for(std::list<Time>::iterator ctit = (*it)->clientRecvTimes.begin(); ctit != (*it)->clientRecvTimes.end(); ctit++){
                if(!(*ctit).IsZero()){
                    messageRecvTimesForClientToClient[(*it)->messageNameIndex].second.push_back(((*ctit) - (*it)->sendTime).GetMilliSeconds());
                }
            }
        }


        for(std::vector<MessageStats*>::iterator it = serverMessageLog[streamNumber].begin(); it != serverMessageLog[streamNumber].end(); it++){
            messageRecvTimesFromServerToClient[(*it)->messageNameIndex].first.first = odm_fnptr((*it)->messageNameIndex);
            odmSizes[(*it)->messageNameIndex].first = odm_fnptr((*it)->messageNameIndex);

            messageRecvTimesFromServerToClient[(*it)->messageNameIndex].first.second = (*it)->clientTimeRequirement;
            messagesSendIntervalFromServer[(*it)->messageNameIndex].push_back((*it)->sendTimeInterval.GetMilliSeconds());
            messagesSentFromServer[(*it)->messageNameIndex] += (*it)->numberOfClientsForwarded;
            odmSizes[(*it)->messageNameIndex].second.push_back((*it)->messageSize);

            for(std::list<Time>::iterator ctit = (*it)->clientRecvTimes.begin(); ctit != (*it)->clientRecvTimes.end(); ctit++){
                if(!(*ctit).IsZero()){
                    messageRecvTimesFromServerToClient[(*it)->messageNameIndex].second.push_back(((*ctit) - (*it)->sendTime).GetMilliSeconds());
                }
            }

        }

        for(int h = 0; h < userActionmessageCount; h++){
            if(messageRecvTimesForServer[h].first.first.compare("") != 0){
                scriptGen->generateScriptForClientMessage(messageRecvTimesForClientToClient[h].second, messageRecvTimesForServer[h].second, messageSendIntervalsFromClient[h],
                                                          messageRecvTimesForServer[h].first.first, messageRecvTimesForServer[h].first.second, messageRecvTimesForClientToClient[h].first.second,
                                                          messagesForwardedFromServer[h], uamSizes[h].second);
            }
        }

        for(int h = 0; h < otherDataMessageCount; h++){
            if(messageRecvTimesFromServerToClient[h].first.first.compare("") != 0){
                scriptGen->generateScriptForServerMessage(messageRecvTimesFromServerToClient[h].second, messagesSendIntervalFromServer[h], messageRecvTimesFromServerToClient[h].first.first,
                                                          messageRecvTimesFromServerToClient[h].first.second, messagesSentFromServer[h], odmSizes[h].second);
            }

        }

        free(messagesForwardedFromServer);
        free(messagesSentFromServer);

}


bool StatisticsCollector::generateSingleNodeStatsFromPcap()
{
    std::stringstream stream;
    std::string addrStr;
    int clientNumber;
    char dot;
    struct stat pcapFile;
    std::string fileName;
    pid_t pid;
    int status;
    bool cont = true;
    bool isServer = false;
    Ipv4Address addr;


    for(std::map<Ipv4Address, int>::const_iterator it = clientRunningTimes.begin(); cont; it++)
    {

        if(it == clientRunningTimes.end())
        {
            cont = false;
            isServer = true;
            addr = serverAddr;
        }
        else
        {
            addr = it->first;
        }

        stream.str("");
        stream << addr;

        addrStr = stream.str();
        stream >> clientNumber >> dot >> clientNumber >> dot >> clientNumber;

        stream.str("");
        stream << clientNumber;

        if(isServer)
        {
            fileName = "./results/server.pcap";
        }
        else
        {
            fileName = "./results/client-";
            fileName.append(stream.str());
            fileName.append(".pcap");
        }

        stat(fileName.c_str(), &pcapFile);

        if((pcapFile.st_mode & S_IFMT) == S_IFREG)   //if pcap-file exists
        {
            if((pid = fork()) == 0)
            {
                int fd = open("temp.txt", O_RDWR | O_CREAT | O_TRUNC, S_IRWXO | S_IRWXG | S_IRWXU);
                dup2(fd, 1);
                close(fd);

                std::string addrArg("ip.src==");
                addrArg.append(addrStr);
                const char* const args[] = {"tshark", "-r", fileName.c_str(), "-R", addrArg.c_str(), "-T", "fields", "-e", "frame.time_delta_displayed", NULL};
                execvp("tshark", (char* const*)args);
            }

            waitpid(pid, &status, 0);

            if(status == EXIT_FAILURE)
                return false;

            scriptGen->parseSingleNodePcapStats("temp.txt", false, addr, clientNumber, isServer);

            if((pid = fork()) == 0)
            {
                int fd = open("temp.txt", O_RDWR | O_CREAT | O_TRUNC, S_IRWXO | S_IRWXG | S_IRWXU);
                dup2(fd, 1);
                close(fd);

                std::string addrArg("ip.src==");
                addrArg.append(addrStr);
                const char* const args[] = {"tshark", "-r", fileName.c_str(), "-R", addrArg.c_str(), "-T", "fields", "-e", "frame.cap_len", NULL};
                execvp("tshark", (char* const*)args);
            }

            waitpid(pid, &status, 0);

            if(status == EXIT_FAILURE)
                return false;

            scriptGen->parseSingleNodePcapStats("temp.txt", true, addr, clientNumber, isServer);
        }
    }

    return true;
}


bool StatisticsCollector::generateOverallGraphFromPcap()
{
    std::stringstream stream;
    Ipv4Address addr;
    std::string addrStr;
    int clientNumber;
    char dot;
    std::string fileName;
    pid_t pid;
    int status;
    struct stat pcapFile;

    addr = serverAddr;
    stream << addr;
    addrStr = stream.str();



    stat("./results/server.pcap", &pcapFile);

    if((pcapFile.st_mode & S_IFMT) == S_IFREG)   //if pcap-file exists
    {
        if((pid = fork()) == 0)
        {
            int fd = open("temp.txt",  O_RDWR | O_CREAT | O_TRUNC, S_IRWXO | S_IRWXG | S_IRWXU);
            dup2(fd, 1);
            close(fd);

            std::string statsArg = "io,stat,1,ip.src==";
            statsArg.append(addrStr);
            statsArg.append(",ip.dst==");
            statsArg.append(addrStr);

            const char* const args[] = {"tshark", "-r", "./results/server.pcap", "-qz", statsArg.c_str(), NULL};
            execvp("tshark", (char* const*)args);
        }

        waitpid(pid, &status, 0);

        if(status == EXIT_FAILURE)
            return false;

        scriptGen->parseOverallPcapStats("temp.txt", addr, 0, true);

    }
    else
        return false;

    return true;

}
















