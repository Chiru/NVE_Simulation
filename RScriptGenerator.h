#ifndef RSCRIPTGENERATOR_H
#define RSCRIPTGENERATOR_H

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <list>

class RScriptGenerator{

public:
    RScriptGenerator(const std::string& filename, const std::string& resultFile);
    ~RScriptGenerator();
    bool generateScriptForStream(const std::list<int64_t>& transmitTimesToClients,  const std::list<int64_t>& transmitTimesToServer, uint16_t streamNumber, int serverTimeRequirement,
                                 int clientTimeRequirement);
    bool writeAndExecuteResultScript();

private:
    std::list<int64_t>* transmitTimesToServer;
    std::list<int64_t>* transmitTimesToClients;
    std::ofstream* filestream;
    std::string clientScript;
    std::string serverScript;
};

RScriptGenerator::RScriptGenerator(const std::string &filename, const std::string& resultFile){

    std::string header("# Generated R script for NVE simulator statistics\n\n ");
    std::string result("pdf(file=\"");
    result.append(resultFile);
    result.append("\")\n");
    clientScript.assign("#Average trasnmit times from client to server\n");
    serverScript.assign("#Average transmit times from client to client\n");

    filestream =  new std::ofstream("results/Rscripts/resultscript.R", std::ios_base::out | std::ios_base::trunc);

    filestream->write(header.c_str(), header.size());
    filestream->write(result.c_str(), result.size());
}

RScriptGenerator::~RScriptGenerator(){

    filestream->close();
    delete filestream;
}

bool RScriptGenerator::generateScriptForStream(const std::list<int64_t> &transmitTimesToClients, const std::list<int64_t> &transmitTimesToServer, uint16_t streamNumber,
                                               int serverTimeRequirement, int clientTimeRequirement){

    static int count = 0;
    static std::string server("serverstream_");
    static std::string client("clientstream_");
    static std::string serverFunc("serverfunc_");
    static std::string clientFunc("clientfunc_");
    static std::string colors[] = {"col=\"green\"", "col=\"blue\"", "col=\"cyan\"", "col=\"yellow\"", "col=\"grey\""};

    std::stringstream stream;

    stream << "\n#Vector for server receive times on stream number: " << streamNumber << std::endl;
    stream << server << count;
    stream << " = c(";
    for(std::list<int64_t>::const_iterator it = transmitTimesToServer.begin(); it != transmitTimesToServer.end();){
        stream << *it;

        if(++it == transmitTimesToServer.end()){
            stream << ")\n";
            break;
        }
        else
            stream << ", ";
    }

    stream << "\n#Step function for stream number: " << streamNumber << "\n";
    stream << serverFunc << count << " = ecdf(" << server << count << ")\n";

    stream << "\n#Plotting for this stream\n";
    if(count == 0)  //then this is the first plot to draw to server
        stream << "plot(" << serverFunc << count << ", do.points=FALSE, verticals=TRUE, "<< colors[count] << ")\n";
    else
        stream << "plot(" << serverFunc << count << ", do.points=FALSE, add=TRUE, verticals=TRUE, "  << colors[count] << ")\n";

    stream << "lines(c(" << serverTimeRequirement << ", " << serverTimeRequirement << "), c(0,1), lty=2, " << colors[count] << ")\n";

    serverScript.append(stream.str());

    stream.str("");

    stream << "\n#Vector for client receive times on stream number: " << streamNumber << std::endl;
    stream << client << count;
    stream << " = c(";

    for(std::list<int64_t>::const_iterator it = transmitTimesToClients.begin(); it != transmitTimesToClients.end();){
        stream << *it;

        if(++it == transmitTimesToClients.end()){
            stream << ")\n";
            break;
        }
        else
            stream << ", ";
    }

    stream << "\n#Step function for stream number: " << streamNumber << "\n";
    stream << clientFunc << count << " = ecdf(" << client << count << ")\n";


    stream << "\n#Plotting for this stream\n";
    if(count == 0)   //then this is the first plot to draw to client
        stream << "plot(" << clientFunc << count << ", do.points=FALSE, verticals=TRUE, " << colors[count] << ")\n";
    else
       stream << "plot(" << clientFunc << count << ", do.points=FALSE, add=TRUE, verticals=TRUE, " << colors[count] << ")\n";

    stream << "lines(c(" << clientTimeRequirement << ", " << clientTimeRequirement << "), c(0,1), lty=2, " << colors[count] << ")\n";

   clientScript.append(stream.str());

   count++;

   return true;
}

bool RScriptGenerator::writeAndExecuteResultScript(){



    filestream->write(clientScript.c_str(), clientScript.size());
    filestream->write(serverScript.c_str(), serverScript.size());

    return true;
}

#endif // RSCRIPTGENERATOR_H
