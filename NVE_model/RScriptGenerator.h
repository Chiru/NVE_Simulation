#ifndef RSCRIPTGENERATOR_H
#define RSCRIPTGENERATOR_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <list>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "utilities.h"

class RScriptGenerator{

public:
    RScriptGenerator(const std::string& filename, const std::string& resultPdfFile, std::string& resultTextFile);
    ~RScriptGenerator();
    bool generateScriptForStream(const std::list<int64_t>* transmitTimesToClients,  const std::list<int64_t>* transmitTimesToServer, const std::list<int64_t>* transmitTimesFromServer,
                                 uint16_t maxStreams);
    bool generateScriptForClientMessage(std::list<int> clientRecvTimes, std::list<int> serverRecvTimes, std::list<int> sendIntervals, const std::string& name, int serverTimeReq,
                                        int clientTimeReq, uint32_t numberOfClientsForwarded, std::list<uint16_t> sizes);
    bool generateScriptForServerMessage(std::list<int> clientRecvTimes, std::list<int> sendIntervals, const std::string& name, int clientTimeReq, uint16_t numberOfClientsForwarded,
                                        std::list<uint16_t> sizes);
    bool writeAndExecuteResultScript();
    bool generateBandwidthHistogram(double clientDownlink, double clientUplink, double serverDownlink, double serverUplink);
    bool addClientBandwidth(const Ipv4Address& addr, double downLink, double upLink, bool isClient = true);

private:
    std::ofstream* filestream;
    std::string resultPdf;
    std::string resultTxt;
    std::string streamScript;
    std::string messageScript;
};

#endif // RSCRIPTGENERATOR_H
