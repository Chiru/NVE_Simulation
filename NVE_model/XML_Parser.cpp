#include "XML_parser.h"

//class XMLParser function definitions


//this constructor is for GUI
XMLParser::XMLParser()
    : appProto(0),
      clientStreams(0),
      serverStreams(0),
      numberOfClients(0),
      numberOfStreams(0),
      clients(0)
{
    errorMessage = new std::stringstream();
}


//this constructor is for simulation usage
XMLParser::XMLParser(std::string filename)
    : filename(filename),
      correctFile(true),
      appProto(0),
      clientStreams(0),
      serverStreams(0),
      numberOfClients(0),
      numberOfStreams(0),
      clients(0)
{

    errorMessage = new std::stringstream();

    std::ifstream filestream(filename.c_str());
    std::string xmlFile;
    std::string token;

    if(filestream.fail()){
        *errorMessage << "XML file \"" << filename << "\" could not be opened." << std::endl;
        correctFile = false;
        return;
    }

    while(!filestream.eof()){
        filestream >> token;
        xmlFile.append(token);
    }

    toLowerCase(xmlFile);

    filestream.close();

    if(!(correctFile = parseClients(xmlFile)))
        return;

    if(!(correctFile = parseApplicationProtocol(xmlFile)))
        return;

    if(!(correctFile = parseStreams(xmlFile)))
        return;


    if(!(correctFile = parseSimulationParams(xmlFile)))
        return;
}

XMLParser::~XMLParser(){

    delete errorMessage;

    for(std::vector<XMLParser::Client*>::iterator it = clients.begin(); it != clients.end(); it++)
    {
        delete (*it);
    }

    for(int i = 0; i < numberOfStreams; i++){
        delete clientStreams[i];
    }

    delete[] serverStreams;
    delete[] clientStreams;

    delete appProto;

}


void XMLParser::toLowerCase(std::string& token)
{
    std::transform(token.begin(), token.end(), token.begin(), ::tolower);
}


bool XMLParser::getElement(const std::string &file, size_t position, const std::string &start, const std::string &end, std::string &result) const
{

    size_t end_position;
    end_position = file.find(end, position);

    if((position = file.find(start, position)) == std::string::npos)
        return false;

    if(end_position == std::string::npos){
        return false;
    }

    if(file.find(start, position+1) < end_position){
        return false;
    }

    result = file.substr(position, end_position - position +1);

    return true;
}

template <class T> bool XMLParser::readValue(const std::string &file, const std::string &variable, T &result, size_t position) const{

    std::string tempVariable = "";
    std::stringstream stream;
    size_t variable_begin;
    tempVariable.append(variable);
    tempVariable.append("=\"");

    variable_begin = file.find(tempVariable, position);
    if(variable_begin == std::string::npos)
        return false;

    variable_begin += tempVariable.length();
    while(file.at(variable_begin) != '\"'){

        if(!iswspace(file.at(variable_begin)))
            stream << file.at(variable_begin);

        variable_begin++;
    }

    stream >> result;

    PRINT_INFO(tempVariable <<  " " <<  result << std::endl);

    if(stream.fail())
        return false;

    return true;
}

bool XMLParser::readBoolVariable(const std::string &file, const std::string &variable, bool defaultValue, size_t position) const
{
    std::string result("");

    if(readValue<std::string>(file, variable, result, position))
    {
        if(result.compare("yes") == 0)
            return true;
        if(result.compare("no") == 0)
            return false;
    }

    return defaultValue;

}

bool XMLParser::getRunningValue(const std::string &value, uint16_t &from, uint16_t &to) const{

    std::stringstream stream;
    char delim;
    stream << value;

    if(value.find(":") != std::string::npos){
        stream >> from >> delim >> to;
        if(stream.fail())
           return false;
    }else{
        stream >> from;
        to = from;
        if(stream.fail())
            return false;
    }

    return true;
}

bool XMLParser::parseClients(std::string &file){

    size_t latest_token = 0, temp_position = 0;
    std::string token;
    std::string value;
    XMLParser::Client* tempClient;

    int count = 0;
    uint16_t from = 0, to = 0;

    if((temp_position = file.find("<clients>")) == std::string::npos){
        *errorMessage <<  "Incorrect format in XML file: no <clients> tag found" << std::endl;
        return false;
    }

    while((temp_position = file.find("<client>", temp_position+1)) != std::string::npos){

        if(!getElement(file, temp_position, "<client>", "</client>", token)){
            *errorMessage <<  "Incorrect format in client specifications" << std::endl;
            return false;
        }

        latest_token = token.find("no=");
        value = "";
        if(readValue<std::string>(token, "no", value, latest_token)){

            if(getRunningValue(value, from, to) && to-from+1 > 0){
                count += to-from+1;

                for(int i = 0; i < to-from+1; i++){
                    tempClient = new XMLParser::Client();
                    clients.push_back(tempClient);

                    tempClient->clientNumber = from + i;

                    if(!readValue<int>(token, "delay", tempClient->delay, latest_token)){
                        *errorMessage <<  "Incorrect format in client parameters" << std::endl;
                        return false;
                    }

                    if(!readValue<double>(token, "uplink", tempClient->uplink, latest_token)){
                        *errorMessage <<  "Incorrect format in client parameters" << std::endl;
                        return false;
                    }

                    if(!readValue<double>(token, "downlink", tempClient->downlink, latest_token)){
                        *errorMessage <<  "Incorrect format in client parameters" << std::endl;
                        return false;
                    }

                    if(!readValue<double>(token, "loss", tempClient->loss, latest_token)){
                        *errorMessage <<  "Incorrect format in client parameters" << std::endl;
                        return false;
                    }

                    if(!readValue<int>(token, "jointime", tempClient->joinTime, latest_token))
                    {
                        tempClient->joinTime = 0;
                    }

                    if(!readValue<int>(token, "exittime", tempClient->exitTime, latest_token))
                    {
                        tempClient->exitTime = 0;   //this means that client stays until the end of the simulation
                    }

                    tempClient->pcap = readBoolVariable(token, "pcap", false);

                    tempClient->graph = readBoolVariable(token, "graphs", false);
                }

            }else{
                *errorMessage <<  "Incorrect format in XML file." << value << std::endl;
                return false;
            }
        }else{
            *errorMessage <<  "Incorrect format in XML file." << value << std::endl;
            return false;
        }
    }

    if((latest_token = file.find("</clients>", latest_token)) == std::string::npos){
        *errorMessage <<  "Incorrect format in XML file: no </clients> tag found or clients defined after it" << std::endl;
        return false;
    }

    if(count == 0){
        *errorMessage <<  "No clients specified" << std::endl;
        return false;
    }

    numberOfClients = count;

    return true;
}

uint16_t XMLParser::countStreams(std::string &file){

    uint16_t count;
    size_t position = 0;

    for(count = 0;(position = file.find("<stream>", position+1)) != std::string::npos; count++);

    return count;

}

bool XMLParser::parseStream(std::string &streamElement, DataGenerator* &clientStream, DataGenerator* &serverStream, int streamNumber){

    DataGenerator::Protocol proto = DataGenerator::UDP;
    ApplicationProtocol* appProto;
    int position;
    std::string type;
    std::string nagle("");
    std::string useAppProto("");
    std::string messagesElement("");
    std::string ordered("");
    std::vector<Message*> messages;
    int serverGameTick = 0, clientGameTick = 0;

    if(!readValue<std::string>(streamElement, "type", type, 0)){
        *errorMessage <<  "No stream type specified in stream number: " << streamNumber << std::endl;
        return false;
    }

    readValue<std::string>(streamElement, "nagle", nagle, 0);
    if(nagle.compare("yes") == 0 && type.compare("tcp") == 0){
        proto = DataGenerator::TCP_NAGLE_ENABLED;
    }else if(type.compare("tcp") == 0){
        proto = DataGenerator::TCP_NAGLE_DISABLED;
    }else if(type.compare("udp") == 0)
        proto = DataGenerator::UDP;
    else {
        *errorMessage <<  "Invalid type in stream number: " << streamNumber << std::endl;
    }

    readValue<std::string>(streamElement, "appproto", useAppProto, 0);

    if(useAppProto.compare("yes")== 0 && type.compare("udp") == 0){
        if(!getApplicationProtocol(appProto)){
            appProto = 0;
        }

        if(!readValue<std::string>(streamElement, "ordered", ordered, 0)){
            *errorMessage << "Error in ordering specification in stream number " << streamNumber << std::endl;
            return false;
        }

    }else appProto = 0;


    if(!parseGameTick(streamElement, serverGameTick, clientGameTick)){
        delete appProto;
        return false;
    }

    if((position = streamElement.find("<messages>")) == std::string::npos){
        *errorMessage <<  "No messages specified in stream number " << streamNumber << std::endl;
        delete appProto;
        return false;
    }

    if(!getElement(streamElement, position, "<messages>", "</messages>", messagesElement)){
        *errorMessage <<  streamElement << std::endl;
        *errorMessage <<  "Incorrect format in message specification in stream number " << streamNumber << std::endl;
        delete appProto;
        return false;
    }

    if(!parseMessages(messagesElement, messages, streamNumber)){
        *errorMessage <<  "Incorrect format in message specifications." << std::endl;
        delete appProto;
        for(std::vector<Message*>::iterator it = messages.begin(); it != messages.end(); it++){
            delete *it;
        }
        return false;
    }

    clientStream = new ClientDataGenerator(streamNumber, proto, appProto, messages, clientGameTick, (ordered.compare("yes") == 0));
    serverStream = new ServerDataGenerator(streamNumber, proto, appProto, messages, serverGameTick, (ordered.compare("yes") == 0));

    return true;
}

bool XMLParser::parseStreams(std::string &file){

    size_t latestToken = 0, temp_position = 0;
    int count = 0;
    std::string streams;
    std::string streamElement;

    if((latestToken = file.find("<streams>")) == std::string::npos){
        *errorMessage <<  "Incorrect format in XML file: no <streams> tag found" << std::endl;
        return false;
    }

    if(!getElement(file, latestToken, "<streams>", "</streams>", streams)){
        *errorMessage <<  "Incorrect format in streams specifications" << std::endl;
        return false;
    }

    numberOfStreams = countStreams(streams);
    this->clientStreams = new DataGenerator*[numberOfStreams];
    this->serverStreams = new DataGenerator*[numberOfStreams];

    for(int i = 0; i < numberOfStreams; i++){
        this->clientStreams[i] = 0;
        this->serverStreams[i] = 0;
    }

    latestToken = 0;
    int streamNumber = 1;

    while((temp_position = streams.find("<stream>", latestToken+1)) != std::string::npos){
        latestToken = temp_position;
        if(!getElement(streams, latestToken, "<stream>", "</stream>", streamElement)){
            *errorMessage <<  "Incorrect format in stream specifications." << std::endl;
            return false;
        }

        if(!parseStream(streamElement, this->clientStreams[count], this->serverStreams[count], streamNumber)){
            *errorMessage <<  "Incorrect format in stream specification." << std::endl;
            return false;
        }

        streamNumber++;
        count++;

    }

    if((latestToken = file.find("</streams>", latestToken)) == std::string::npos){
        *errorMessage <<  "Incorrect format in XML file: no </streams> tag found or streams defined after it" << std::endl;
        return false;
    }

    if(count == 0){
        *errorMessage <<  "No streams specified" << std::endl;
        return false;
    }

    return true;

}

bool XMLParser::parseMessages(std::string &messagesElement, std::vector<Message*> &messages, uint16_t stream_number){

    size_t latest_token = 0;
    std::string messageElement("");
    std::string type, reliable, name, returnToSender;
    std::string forwardSizeStr("");
    bool rcv = false;
    int size = 0, timeInterval = 0, clientTimeRequirement = 0, serverTimeRequirement = 0, forwardSize = 0;
    double clientsOfInterest;
    RandomVariable* ranvarTimeInterval = 0;
    RandomVariable* ranvarSize = 0;
    RandomVariable* ranvarForwardSize = 0;
    DistributionEnum distribution;

    if((latest_token = messagesElement.find("<message>")) == std::string::npos){
        *errorMessage <<  "Error in message specifications" << std::endl;
        return false;
    }

    while(getElement(messagesElement, latest_token, "<message>", "</message>", messageElement)){

        ranvarTimeInterval = ranvarSize = ranvarForwardSize = 0;

        if(!readValue<std::string>(messageElement, "type", type, 0)){
            *errorMessage <<  "Error in message type specification." << std::endl;
            return false;
        }

        if(!readValue<std::string>(messageElement, "name", name, 0)){
            *errorMessage <<  "No message name specified." << std::endl;
            return false;
        }

        if(!readValue<std::string>(messageElement, "reliable", reliable, 0)){
            *errorMessage <<  "Error in reliability specification." << std::endl;
            return false;
        }

        if(!readRandomVariable(messageElement, ranvarSize, distribution, "size")){                  //if no distribution is specified, read simple timeinterval
            if(!readValue<int>(messageElement, "size", size, 0)){
                *errorMessage <<  "Error in message size specification." << std::endl;
                return false;
            }
       }

        if(!readRandomVariable(messageElement, ranvarTimeInterval, distribution, "timeinterval")){          //if no distribution is specified, read simple timeinterval
            if(!readValue<int>(messageElement, "timeinterval", timeInterval, 0)){
                *errorMessage <<  "Error in message timeinterval specification." << std::endl;
                return false;
            }
        }

        if(!readValue<std::string>(messageElement, "returntosender", returnToSender, 0)){
            *errorMessage <<  "Error in returntosender specification." << std::endl;
            return false;
        }


        if(!readValue<std::string>(messageElement, "forwardmessagesize", forwardSizeStr, 0))
        {
            *errorMessage <<  "Error in message forwardmessagesize specification." << std::endl;
            return false;
        }
        else
        {
            if(forwardSizeStr == "rcv")           //if "rcv", use received message size, otherwise read distribution or constant value
            {
                rcv = true;
            }
            else
            {
                if(!readRandomVariable(messageElement, ranvarForwardSize, distribution, "forwardmessagesize"))
                {
                    readValue<int>(messageElement, "forwardmessagesize", forwardSize, 0);
                }
            }
        }

        if(size <= 0 && ranvarSize == 0){
            *errorMessage <<  "Message size must be more than 0 ot distribution must be specified." << std::endl;
            return false;
        }

        if(timeInterval <= 0 && ranvarTimeInterval == 0){
            *errorMessage <<  "TimeInterval value must be either more than 0 or distribution must be specified." << std::endl;
            return false;
        }

        if(!readValue<int>(messageElement, "timerequirementclient", clientTimeRequirement, 0)){
            *errorMessage <<  "Error in message timerequirementclient specification." << std::endl;
            return false;
        }

        if(type.compare("uam") == 0){


            if(!readValue<int>(messageElement, "timerequirementserver", serverTimeRequirement, 0)){
                *errorMessage <<  "Error in message timerequirementserver specification." << std::endl;
                return false;
            }

            if(serverTimeRequirement <= 0 || clientTimeRequirement <= 0){
                *errorMessage <<  "TimeRequirement must be more than 0." << std::endl;
                return false;
            }

            if(!readValue<double>(messageElement, "clientsofinterest", clientsOfInterest, 0)){
                *errorMessage <<  "Error in message clients of interest specification." << std::endl;
                return false;
            }

            if(clientsOfInterest < 0 || clientsOfInterest > 1){
                *errorMessage <<  "ClientsOfInterest must be between 0 and 1." << std::endl;
                return false;
            }

           messages.push_back(new UserActionMessage(name, reliable.compare("no") == 0 ? false : true, timeInterval, size, clientsOfInterest, clientTimeRequirement,
                                                         serverTimeRequirement, stream_number, forwardSize, returnToSender.compare("no") == 0 ? false : true,
                                                         rcv, ranvarTimeInterval, ranvarSize, ranvarForwardSize));
        }

        else if(type.compare("odt") == 0){

            if(clientTimeRequirement <= 0){
                *errorMessage <<  "TimeRequirement must be more than 0." << std::endl;
                return false;
            }

           messages.push_back(new OtherDataMessage(name, reliable.compare("no") == 0 ? false : true, timeInterval, size, stream_number, forwardSize,
                                                   returnToSender.compare("no") == 0 ? false : true, rcv,clientTimeRequirement, ranvarTimeInterval, ranvarSize,
                                                   ranvarForwardSize));
        }

        else{
            *errorMessage << "Unknown message type: " << type <<std::endl;
            return false;
        }

        if((latest_token = messagesElement.find("</message>", latest_token)) == std::string::npos){
            *errorMessage <<  "Error in messages specification: missing </message> tag." << std::endl;
            return false;
        }

        latest_token++;
    }

    return true;

}

bool XMLParser::parseApplicationProtocol(std::string &file){

    size_t position;
    std::string token, value;
    int acksize, delack, retransmit, headerSize;

    if((position = file.find("<appproto>")) == std::string::npos){
        appProto = 0;
        *errorMessage <<  "No application protocol found" << std::endl;
        return true;
    }

    if(!getElement(file, position, "<appproto>", "</appproto>", token)){
        *errorMessage <<  "Incorrect format in application protocol specifications." << std::endl;
        return false;
    }

    value = "";

    if(!readValue<int>(token, "acksize", acksize)){
        *errorMessage <<  "Incorrect format in application protocol parameter: acksize" << std::endl;
        return false;
    }

    if(!readValue<int>(token, "delayedack", delack)){
        *errorMessage <<  "Incorrect format in application protocol parameter: delayedack" << std::endl;
        return false;
    }

    if(!readValue<int>(token, "retransmit", retransmit)){
        *errorMessage <<  "Incorrect format in application protocol parameter: retransmit" << std::endl;
        return false;
    }

    if(!readValue<int>(token, "headersize", headerSize)){
        *errorMessage << "Incorrect format in application protocol parameter: headersize" << std::endl;
        return false;
    }


    appProto = new ApplicationProtocol(acksize, delack, retransmit, headerSize);

    return true;

}

bool XMLParser::getStreams(DataGenerator** &streams, bool isClient, uint16_t clientNumber) const{

    streams = new DataGenerator*[numberOfStreams];

    for(int i = 0; i < numberOfStreams; i++)
        streams[i] = 0;

    if(this->clientStreams[0] != 0 && isClient){
        for(int i = 0; i < numberOfStreams; i++){
            ((ClientDataGenerator*)this->clientStreams[i])->setClientNumber(clientNumber);
            streams[i] = new ClientDataGenerator(*(this->clientStreams[i]));
        }
    }
    else if(this->serverStreams[0]){
        for(int i = 0; i < numberOfStreams; i++){
            streams[i] = new ServerDataGenerator(*(this->serverStreams[i]));
        }
    }else return false;

    return true;
}


bool XMLParser::getClientStats(uint16_t clientIndex, uint16_t &clientNumber, int &delay, double &uplink, double &downlink, double &loss, bool& pcap, bool& graphs,
                               int &joinTime, int &exitTime) const
{

    std::vector<XMLParser::Client*>::const_iterator it;
    int i;

    for(it = clients.begin(), i = 1; it != clients.end(); it++, i++){
        if(i == clientIndex){
            clientNumber = (*it)->clientNumber;
            delay = (*it)->delay;
            uplink = (*it)->uplink;
            downlink = (*it)->downlink;
            loss = (*it)->loss;
            pcap = (*it)->pcap;
            graphs = (*it)->graph;
            joinTime = (*it)->joinTime;

            if((*it)->exitTime != 0 && (*it)->exitTime > joinTime)
                exitTime = (*it)->exitTime;
            else
                exitTime = runningTime;

            break;
        }
        if(it == clients.end())
            return false;
    }

    return true;
}

bool XMLParser::parseGameTick(std::string& streamElement, int& serverGameTick, int& clientGameTick){

    if(!readValue<int>(streamElement, "<servergametick", serverGameTick) || serverGameTick < 0){
        *errorMessage << " Incorrect servergametick value." << std::endl;
        return false;
    }

    if(!readValue<int>(streamElement, "<clientgametick", clientGameTick) || clientGameTick < 0){
        *errorMessage << " Incorrect clientgametick value." << std::endl;
        return false;
    }

    return true;
}

bool XMLParser::parseSimulationParams(std::string &file){

    if(!readValue<int>(file, "<runningtime", runningTime) || runningTime <= 0)
    {
        *errorMessage << " Incorrect runningtime value." << std::endl;
        return false;
    }

    serverPcap = readBoolVariable(file, "<serverpcap", true);

    return true;
}

 bool XMLParser::getApplicationProtocol(ApplicationProtocol* &proto) const{

    if(appProto == 0)
        return false;

    else
        proto = new ApplicationProtocol(*appProto);

    return true;

}

 bool XMLParser::readRandomVariable(const std::string& element, RandomVariable*& ranvar, DistributionEnum& distribution, const std::string& variableName){

     std::string result;
     std::string filename("");
     if(variableName.empty()){
        result = element;
     }else{
        readValue<std::string>(element, variableName, result, 0);
     }

     std::string distName(result.substr(0, result.find('(')));

     if(distName.compare("empirical") == 0){
         getElement(result, 0, "(", ")", filename);
         filename = filename.substr(1, filename.length()-2);
     }

     distribution = DistributionEnum(distName, this);
     if(distribution == None)
         return false;

     ranvar = distribution.constructRandomVariable(result.substr(distName.length(), result.length() - distName.length()), filename);
     if(ranvar == 0)
         return false;

     return true;
 }


 //nested class DistributionEnum definitions

 const int XMLParser::DistributionEnum::distCount = 17;
 const std::string XMLParser::DistributionEnum::distributionStrings[XMLParser::DistributionEnum::distCount] = {"uniform", "constant", "sequential", "exponential", "pareto", "weibull", "normal",
                                                                                                               "lognormal", "gamma", "erlang", "zipf", "zeta",
                                                                                                               "triangular", "empirical", "extreme", "split", "none"};

 XMLParser::DistributionEnum::DistributionEnum(): distribution(XMLParser::Uniform){}

 XMLParser::DistributionEnum::DistributionEnum(XMLParser::Distributions dist, XMLParser* owner) : owner(owner){
    setDistribution(dist);
 }

XMLParser::DistributionEnum::DistributionEnum(int dist, XMLParser* owner): owner(owner){
     setDistribution(getDistribution(dist));
 }

  XMLParser::DistributionEnum::DistributionEnum(const std::string& distStr, XMLParser* owner) : owner(owner){

      for(distribution = XMLParser::Uniform; this->operator<(XMLParser::None); distribution = getDistribution(((int)distribution+1))){
         if(distributionStrings[distribution].compare(distStr) == 0){
             break;
         }
     }
 }

  XMLParser::DistributionEnum::operator int() const{
     for(int i = 0; i < distCount; i++){
         if(i == this->distribution)
             return i;
     }
     return XMLParser::None;
 }

 bool  XMLParser::DistributionEnum::operator==(const XMLParser::Distributions &d){
     if(d == this->distribution)
         return true;
     else
         return false;
 }

 bool  XMLParser::DistributionEnum::operator<(const XMLParser::Distributions &d){
     if(d > distribution)
         return true;
     else
         return false;
 }

 bool  XMLParser::DistributionEnum::operator>(const XMLParser::Distributions &d){
     if(d < distribution)
         return true;
     else
         return false;
 }

 void  XMLParser::DistributionEnum::setDistribution(XMLParser::Distributions dist){
     distribution = dist;
 }

 XMLParser::Distributions  XMLParser::DistributionEnum::getDistribution() const{
     return distribution;
 }

 XMLParser::Distributions  XMLParser::DistributionEnum::getDistribution(int i) const{

     switch(i){
         case 0: return XMLParser::Uniform;
         case 1: return XMLParser::Constant;
         case 2: return XMLParser::Sequential;
         case 3: return XMLParser::Exponential;
         case 4: return XMLParser::Pareto;
         case 5: return XMLParser::Weibull;
         case 6: return XMLParser::Normal;
         case 7: return XMLParser::Lognormal;
         case 8: return XMLParser::Gamma;
         case 9: return XMLParser::Erlang;
         case 10: return XMLParser::Zipf;
         case 11: return XMLParser::Zeta;
         case 12: return XMLParser::Triangular;
         case 13: return XMLParser::Empirical;
         case 14: return XMLParser::Extreme;
         case 15: return XMLParser::Split;
         case 16: return XMLParser::None;
         default: return XMLParser::None;
     }
}

 template <class T, class U, class V, class W> bool XMLParser::DistributionEnum::readCommaSeparatedString(const std::string &csvString, int paramCount, T& val1, U& val2, V& val3, W& val4){

      std::stringstream stream;
      stream << csvString;

      char c, c2,c3;
      stream >> c;
      if(c != '(' || stream.fail())
          return false;

      switch(paramCount){
      case 1:
          stream >> val1;
          if(stream.fail())
              return false;
          break;
      case 2:
          stream >> val1;
          stream >> c;
          stream >> val2;
          if(stream.fail() || c != ',')
              return false;
          break;
      case 3:
          stream >> val1;
          stream >> c;
          stream >> val2;
          stream >> c2;
          stream >> val3;
          if(stream.fail() || c != ',' || c2 != ',')
              return false;
          break;
      case 4:
          stream >> val1;
          stream >> c;
          stream >> val2;
          stream >> c2;
          stream >> val3;
          stream >> c3;
          stream >> val4;
          if(stream.fail() || c != ',' || c2 != ',' || c3 != ',')
              return false;
          break;
      }

      stream >> c;

      if(c != ')' || stream.fail())
          return false;

     return true;
 }



RandomVariable* XMLParser::DistributionEnum::constructRandomVariable(const std::string &params, const std::string& fileName){

    RandomVariable* retVal = 0;
    XMLParser::RanvarValues v1, v2, v3, v4;

     switch(distribution){

     case XMLParser::Uniform:
         if(!readCommaSeparatedString<double, double, uint32_t, uint32_t>(params, 2, v1.doubleVal, v2.doubleVal, v3.uintVal, v4.uintVal)){
             (*owner->errorMessage) << "Error in timeinterval distribution parameters" << std::endl;
         }else{
             retVal = new UniformVariable(v1.doubleVal, v2.doubleVal);
         }
         break;
     case XMLParser::Constant:
         if(!readCommaSeparatedString<double, uint32_t, uint32_t, uint32_t>(params, 1, v1.doubleVal, v2.uintVal, v3.uintVal, v4.uintVal)){
             (*owner->errorMessage) << "Error in timeinterval distribution parameters" << std::endl;
         }else{
             retVal = new ConstantVariable(v1.doubleVal);
         }
         break;
     case XMLParser::Sequential:
         if(!readCommaSeparatedString<double, double, double, uint32_t>(params, 4, v1.doubleVal, v2.doubleVal, v3.doubleVal, v3.uintVal)){
             (*owner->errorMessage) << "Error in timeinterval distribution parameters" << std::endl;
         }else{
             retVal = new SequentialVariable(v1.doubleVal, v2.doubleVal, v3.doubleVal, v4.uintVal);
         }
         break;

     case XMLParser::Exponential:
         if(!readCommaSeparatedString<double, double, uint32_t, uint32_t>(params, 2, v1.doubleVal, v2.doubleVal, v3.uintVal, v4.uintVal)){
             (*owner->errorMessage) << "Error in timeinterval distribution parameters" << std::endl;
         }else{
             retVal = new ExponentialVariable(v1.doubleVal, v2.doubleVal);
         }
         break;
     case XMLParser::Pareto:
         if(!readCommaSeparatedString<double, double, double, uint32_t>(params, 3, v1.doubleVal, v2.doubleVal, v3.doubleVal, v4.uintVal)){
             (*owner->errorMessage) << "Error in timeinterval distribution parameters" << std::endl;
         }else{
             retVal = new ParetoVariable(v1.doubleVal, v2.doubleVal, v3.doubleVal);
         }
         break;
     case XMLParser::Weibull:
         if(!readCommaSeparatedString<double, double, double, double>(params, 4, v1.doubleVal, v2.doubleVal, v3.doubleVal, v4.doubleVal)){
             (*owner->errorMessage) << "Error in timeinterval distribution parameters" << std::endl;
         }else{
             retVal = new WeibullVariable(v1.doubleVal, v2.doubleVal, v3.doubleVal, v4.doubleVal);
         }
         break;
     case XMLParser::Normal:
         if(!readCommaSeparatedString<double, double, uint32_t, uint32_t>(params, 2, v1.doubleVal, v2.doubleVal, v3.uintVal, v4.uintVal)){
             (*owner->errorMessage) << "Error in timeinterval distribution parameters" << std::endl;
         }else{
             retVal = new NormalVariable(v1.doubleVal, v2.doubleVal);
         }
         break;
     case XMLParser::Lognormal:
         if(!readCommaSeparatedString<double, double, uint32_t, uint32_t>(params, 2, v1.doubleVal, v2.doubleVal, v3.uintVal, v4.uintVal)){
             (*owner->errorMessage) << "Error in timeinterval distribution parameters" << std::endl;
         }else{
             retVal = new LogNormalVariable(v1.doubleVal, v2.doubleVal);
         }
         break;
     case XMLParser::Gamma:
         if(!readCommaSeparatedString<double, double, uint32_t, uint32_t>(params, 2, v1.doubleVal, v2.doubleVal, v3.uintVal, v4.uintVal)){
             (*owner->errorMessage) << "Error in timeinterval distribution parameters" << std::endl;
         }else{
             retVal = new GammaVariable(v1.doubleVal, v2.doubleVal);
         }
         break;
     case XMLParser::Erlang:
         if(!readCommaSeparatedString<uint32_t, double, uint32_t, uint32_t>(params, 2, v1.uintVal, v2.doubleVal, v3.uintVal, v4.uintVal)){
             (*owner->errorMessage) << "Error in timeinterval distribution parameters" << std::endl;
         }else{
             retVal = new ErlangVariable(v1.uintVal, v2.doubleVal);
         }
         break;
     case XMLParser::Zipf:
         if(!readCommaSeparatedString<long, double, uint32_t, uint32_t>(params, 2, v1.longIntVal, v2.doubleVal, v3.uintVal, v4.uintVal)){
             (*owner->errorMessage) << "Error in timeinterval distribution parameters" << std::endl;
         }else{
             retVal = new ZipfVariable(v1.longIntVal, v2.doubleVal);
         }
         break;
     case XMLParser::Zeta:
         if(!readCommaSeparatedString<double, uint32_t, uint32_t, uint32_t>(params, 1, v1.doubleVal, v2.uintVal, v3.uintVal, v4.uintVal)){
             (*owner->errorMessage) << "Error in timeinterval distribution parameters" << std::endl;
         }else{
             retVal = new ZetaVariable(v1.doubleVal);
         }
         break;
     case XMLParser::Triangular:
         if(!readCommaSeparatedString<double, double, double, uint32_t>(params, 3, v1.doubleVal, v2.doubleVal, v3.doubleVal, v4.uintVal)){
             (*owner->errorMessage) << "Error in timeinterval distribution parameters" << std::endl;
         }else{
             retVal = new TriangularVariable(v1.doubleVal, v2.doubleVal, v3.doubleVal);
         }
         break;
         //TODO: get the empirical data from a file
     case XMLParser::Empirical:
         retVal = new EmpiricalVariable();

         if(!readEmpiricalDataFile(fileName, *((EmpiricalVariable*)retVal))){
             (*owner->errorMessage) << "Couldn't read empirical values from file: " << fileName << std::endl;
             delete retVal;
             retVal = 0;
         }
         break;

     case XMLParser::Extreme:
         if(!readCommaSeparatedString<double, double, double, uint32_t>(params, 3, v1.doubleVal, v2.doubleVal, v3.doubleVal, v4.uintVal)){
             (*owner->errorMessage) << "Error in timeinterval distribution parameters" << std::endl;
         }else{
             retVal = new ExtremeVariable(v1.doubleVal, v2.doubleVal, v3.doubleVal);
         }
         break;

     case XMLParser::Split:
            retVal = readSplitDistribution(params);
         break;

     case XMLParser::None:
     default:
         return 0;
     }

     return retVal;
}

bool XMLParser::DistributionEnum::readEmpiricalDataFile(const std::string &filename, EmpiricalVariable &ranvar){
//data file must contain each the amount of packets for each x-value (time) {1, ... , n} where n is the max time value
    std::ifstream file(filename.c_str(), std::ios_base::in);

    if(file.fail())
        return false;

    char c;
    double yval;
    double xval = 1;
    double totalValues = 0;
    double valuesThisFar = 0;  //these exist in order to calculate the probabilities


    do{
        file >> yval;

        if(!file.eof() && !file.fail()){
            totalValues += yval;
        }

    }while(file >> c, !file.fail() && !file.eof());

    file.clear(std::ios_base::goodbit);

    file.seekg(std::ios_base::beg);

    do{
        file >> yval;
        valuesThisFar += yval;

        if(yval > 0){
            ranvar.CDF(xval, valuesThisFar/totalValues);
        }

        xval += 1;
    }while(file >> c, !file.fail() && !file.eof());

    file.close();
    return true;
}

RandomVariable* XMLParser::DistributionEnum::readSplitDistribution(const std::string& params){

    std::stringstream stream;
    stream << params;
    SplitDistribution* retVal = 0;
    RandomVariable* ranvar;
    DistributionEnum distribution;
    char c;
    size_t pos = 0;
    size_t previous_pos = 0;
    bool cont = true;
    std::string temp;
    int distCount = 0;
    double percentage = 0;

    stream >> c;

    if(c != '(' || stream.fail())
        return 0;

    retVal = new SplitDistribution();

    stream >> temp;

    std::string dist = "";

    while(cont){

        pos = temp.find_first_of(',', temp.find_first_of(')', pos));

        if(pos == std::string::npos){
            dist = temp.substr(previous_pos, temp.find_first_of(')', previous_pos)- (previous_pos - 1));
            cont = false;
        }else{
            dist = temp.substr(previous_pos, pos - previous_pos);
        }

        if(!readAndRemovePercentage(dist, percentage)){
            delete retVal;
            return 0;
        }

        owner->readRandomVariable(dist, ranvar, distribution, "");

        if(ranvar != 0){
            retVal->AddDistribution(*ranvar, percentage);
            distCount++;
        }

        previous_pos = pos + 1;

    }

    if(distCount == 0){
        delete retVal;
        retVal = 0;

    }

    return retVal;

}

bool XMLParser::DistributionEnum::readAndRemovePercentage(std::string &dist, double &result){

    std::stringstream stream;

    stream << dist;

    stream >> result;

    if(stream.fail())
        return false;

    char c;

    stream >> c;

    if(stream.fail() || c != '%')
        return false;

    stream >> c;

    if(stream.fail() || c != ':')
        return false;

    stream >> dist;

    return true;

}


std::string XMLParser::getErrorMessage() const
{
    return errorMessage->str();
}
