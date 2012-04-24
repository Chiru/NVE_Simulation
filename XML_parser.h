#ifndef XML_PARSER_H
#define XML_PARSER_H


class string;
class vector;
class DataGenerator;
class ApplicationProtocol;

class XMLParser{

     struct Client{
        uint16_t clientNumber;
        double uplink;
        double downlink;
        double loss;
        int delay;
    };

public:
    XMLParser(std::string& filename);
    ~XMLParser();

    bool isFileCorrect(){return correctFile;}
    uint16_t getNumberOfClients()const {return numberOfClients;}
    bool getStreams(DataGenerator**&);
    uint16_t getNumberOfStreams()const {return numberOfStreams;}
    bool getApplicationProtocol(ApplicationProtocol*);
    bool getAppProtoPacketSize(uint16_t &size);
    bool getAppProtoDelayedAck(uint16_t &ack);
    bool getAppProtoRetransmit(uint16_t &retransmit);
    bool getClientStats(uint16_t clientIndex, uint16_t &clientNumber, int &delay, double &uplink, double &downlink, double &loss);

private:
    bool parseClients(std::string& file);
    bool parseStreams(std::string& file);
    template <class T> bool readValue(const std::string& file, const std::string& variable, T& result, size_t position = 0);
    bool getRunningValue(const std::string& value, uint16_t &from, uint16_t &to);
    bool getElement(const std::string& file, size_t position,const  std::string& start, const std::string& end, std::string &result);

    std::string filename;
    std::string file;
    bool correctFile;
    ApplicationProtocol* appProto;
    uint16_t numberOfClients;
    uint16_t numberOfStreams;

    std::vector<struct XMLParser::Client*> clients;

    //std::ifstream &fileStream;

};

#endif // XML_PARSER_H
