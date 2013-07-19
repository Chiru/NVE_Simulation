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

#ifndef XML_PARSER_H
#define XML_PARSER_H


#include "utilities.h"
#include "ApplicationProtocol.h"
#include "DataGenerator.h"
#include "Messages.h"
#include "ns3/random-variable.h"
#include <string>
#include <vector>
#include <typeinfo>


class XMLParser{

     struct Client{
        uint16_t clientNumber;
        double uplink;
        double downlink;
        double loss;
        int delay;
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
    bool getClientStats(uint16_t clientIndex, uint16_t &clientNumber, int &delay, double &uplink, double &downlink, double &loss) const;
    uint16_t getRunningTime() const{return runningTime;}
    bool getElement(const std::string& file, size_t position,const  std::string& start, const std::string& end, std::string &result) const;
    template <class T> bool readValue(const std::string& file, const std::string& variable, T& result, size_t position = 0) const;
    bool getRunningValue(const std::string& value, uint16_t &from, uint16_t &to) const;
    bool readBoolVariable(const std::string& file, const std::string& variable, bool defaultValue, size_t position = 0) const;  //does the same as readValue<bool>




private:

    typedef union{
        double doubleVal;
        long longIntVal;
        uint32_t uintVal;
    } RanvarValues;   //these are the different variables that are passed as a parameter to ns-3 distribution functions

    //distributions supported by ns-3
    typedef enum{Uniform = 0, Constant, Sequential, Exponential, Pareto, Weibull, Normal, Lognormal, Gamma, Erlang, Zipf, Zeta, Triangular, Empirical, Extreme, Split, None} Distributions;


    //convenience class to operate with distribution enum types
    class DistributionEnum{

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

        template <class T, class U, class V, class W> bool readCommaSeparatedString(const std::string& csvString, int paramCount, T& val1, U& val2, V& val3, W& val4);
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
    bool parseRunningTime(std::string& file);
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

    std::vector<struct XMLParser::Client*> clients;

};


#endif // XML_PARSER_H
