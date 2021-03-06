/**
* Copyright (c) 2013 Center for Internet Excellence, University of Oulu, All Rights Reserved
* For conditions of distribution and use, see copyright notice in license.txt
*/


#ifndef XML_PARSER_H
#define XML_PARSER_H


#include "utilities.h"
#include "ApplicationProtocol.h"
#include "DataGenerator.h"
#include "Messages.h"
#include <sstream>
#include "ns3/random-variable.h"
#include <string>
#include <vector>
#include <typeinfo>


class XMLParser
{
     struct Client
     {
        uint16_t clientNumber;
        double uplink;
        double downlink;
        double loss;
        int delay;
        bool pcap;
        bool graph;
        int joinTime;
        int exitTime;
    };

public:
    XMLParser();
    XMLParser(std::string filename);
    ~XMLParser();

    bool isFileCorrect() const{return correctFile;}
    uint16_t getNumberOfClients()const {return numberOfClients;}
    bool getStreams(DataGenerator**&, bool client, uint16_t clientNumber = 0) const;
    uint16_t getNumberOfStreams()const {return numberOfStreams;}
    bool getApplicationProtocol(ApplicationProtocol*&) const;
    bool getClientStats(uint16_t clientIndex, uint16_t &clientNumber, int &delay, double &uplink, double &downlink, double &loss, bool &pcap, bool &graphs,
                        int& joinTime, int& exitTime) const;
    int getRunningTime() const{return runningTime;}
    bool isServerPcapEnabled() const{return serverPcap;}
    bool getElement(const std::string& file, size_t position,const  std::string& start, const std::string& end, std::string &result) const;
    template <class T> bool readValue(const std::string& file, const std::string& variable, T& result, size_t position = 0) const;
    bool getRunningValue(const std::string& value, uint16_t &from, uint16_t &to) const;
    bool readBoolVariable(const std::string& file, const std::string& variable, bool defaultValue, size_t position = 0) const;  //does the same as readValue<bool>
    std::string getErrorMessage() const;


private:

    typedef union
    {
        double doubleVal;
        long longIntVal;
        uint32_t uintVal;
    } RanvarValues;   //these are the different variables that are passed as a parameter to ns-3 distribution functions

    //distributions supported by ns-3
    typedef enum{Uniform = 0, Constant, Sequential, Exponential, Pareto, Weibull, Normal, Lognormal,
                 Gamma, Erlang, Zipf, Zeta, Triangular, Empirical, Extreme, Split, None} Distributions;


    //convenience class to operate with distribution enum types
    class DistributionEnum
    {

    public:
        DistributionEnum();
        explicit DistributionEnum(Distributions dist, XMLParser* owner = 0);
        explicit DistributionEnum(int dist, XMLParser* owner = 0);

        DistributionEnum(const std::string& distStr, XMLParser* owner = 0);
        operator int() const;
        bool operator==(const Distributions &d);
        bool operator<(const Distributions &d);
        bool operator>(const Distributions &d);
        void setDistribution(Distributions dist);
        Distributions getDistribution() const;
        Distributions getDistribution(int i) const;
        RandomVariable* constructRandomVariable(const std::string& params, const std::string& fileName);

    private:
        Distributions distribution;
        static const std::string distributionStrings[];
        static const int distCount;

        template <class T, class U, class V, class W> bool readCommaSeparatedString(const std::string& csvString, int paramCount,
                                                                                    T& val1, U& val2, V& val3, W& val4);
        bool readEmpiricalDataFile(const std::string& filename, EmpiricalVariable& ranvar);
        RandomVariable* readSplitDistribution(const std::string& params);
        bool readAndRemovePercentage(std::string& dist, double& result);


        XMLParser* owner;

    }; //end of nested class


    bool parseClients(std::string& file);
    bool parseStreams(std::string& file);
    bool parseStream(std::string& streamElement, DataGenerator*& clientStream, DataGenerator*& serverStream, int streamNumber);
    bool parseMessages(std::string& messagesElement, std::vector<Message*>& messages, uint16_t stream_number);
    bool parseApplicationProtocol(std::string& file);
    bool parseGameTick(std::string& streamElement, int& serverGameTick, int& clientGameTick);
    bool parseSimulationParams(std::string& file);
    uint16_t countStreams(std::string& file);
    bool readRandomVariable(const std::string& element, RandomVariable*& ranvar, DistributionEnum& distribution, const std::string& variableName);
    void toLowerCase(std::string& token);

    std::string filename;
    bool correctFile;
    bool appProtoExists;
    ApplicationProtocol* appProto;
    DataGenerator **clientStreams;
    DataGenerator **serverStreams;
    uint16_t numberOfClients;
    uint16_t numberOfStreams;
    int serverGameTick;
    int clientGameTick;
    int runningTime;
    bool serverPcap;
    std::stringstream* errorMessage;

    std::vector<struct XMLParser::Client*> clients;

};


#endif // XML_PARSER_H
