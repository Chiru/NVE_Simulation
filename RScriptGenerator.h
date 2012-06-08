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
    RScriptGenerator(const std::string& filename, const std::string& resultFile);
    ~RScriptGenerator();
    bool generateScriptForStream(const std::list<int64_t>& transmitTimesToClients,  const std::list<int64_t>& transmitTimesToServer, uint16_t streamNumber);
    bool generateScriptForMessage(std::list<int> clientRecvTimes, std::list<int> serverRecvTimes, const std::string& name, int serverTimeReq, int clientTimeReq);
    bool writeAndExecuteResultScript();

private:
    std::ofstream* filestream;
    std::string result;
    std::string clientScript;
    std::string serverScript;
    std::string messageScript;
};

RScriptGenerator::RScriptGenerator(const std::string &filename, const std::string& resultFile): result(resultFile){

    std::string header("# Generated R script for NVE simulator statistics\n\n ");
    std::string result("pdf(file=\"");
    result.append(resultFile);
    result.append("\")\n");
    clientScript.assign("#Average trasnmit times from client to server\n");
    serverScript.assign("#Average transmit times from client to client\n");
    messageScript.assign("\n#Average trasnmit times for each message\n");

    filestream =  new std::ofstream("results/Rscripts/resultscript.R", std::ios_base::out | std::ios_base::trunc);

    filestream->write(header.c_str(), header.size());
    filestream->write(result.c_str(), result.size());
}

RScriptGenerator::~RScriptGenerator(){

    filestream->close();
    delete filestream;
}

bool RScriptGenerator::generateScriptForStream(const std::list<int64_t> &transmitTimesToClients, const std::list<int64_t> &transmitTimesToServer, uint16_t streamNumber){

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

   clientScript.append(stream.str());

   count++;

   return true;
}

bool RScriptGenerator::generateScriptForMessage(std::list<int> clientRecvTimes, std::list<int> serverRecvTimes, const std::string &name, int serverTimeReq, int clientTimeReq){

    static std::string color[4] = {"col=\"green\"", "col=\"blue\"", "col=\"red\"", "col=\"orange\""};
    static std::string server("servermsg_");
    static std::string client("clientmsg_");
    static std::string serverFunc("servermsgfunc_");
    static std::string clientFunc("clientmsgfunc_");
    std::stringstream stream;

    stream << "\n#Vector of server receive times for message: " << name << std::endl;
    stream << server << name;
    stream << " = c(";

    for(std::list<int>::const_iterator it = serverRecvTimes.begin(); it != serverRecvTimes.end();){
        stream << *it;
        if(++it == serverRecvTimes.end()){
            stream << ")\n";
            break;
        }
        else
            stream << ", ";
    }
    stream << "\n#Server step function for message: " << name << std::endl;
    stream << serverFunc << name << " = ecdf(" << server << name << ");\n";

    stream << "\n#Vector of client receive times for message: " << name << std::endl;
    stream << client << name;
    stream << " = c(";

    for(std::list<int>::const_iterator it = clientRecvTimes.begin(); it != clientRecvTimes.end();){
        stream << *it;
        if(++it == clientRecvTimes.end()){
            stream << ")\n";
            break;
        }
        else
            stream << ", ";
    }
    stream << "\n#Client step function for message: " << name << std::endl;
    stream << clientFunc << name << " = ecdf(" << client << name << ");\n";

    stream << "\n#Setting max x value\n";
    stream <<  name << "_max = max(" << server << name << ", " << client << name << ")\n";

    stream << "\n#Plotting for message: " << name << std::endl;
    stream << "plot(" << serverFunc << name << ", do.points=FALSE, verticals=TRUE, " << color[0] << ", xlim=(c(0," << name << "_max" << ")))" << std::endl;
    stream << "plot(" << clientFunc << name << ", do.points=FALSE, verticals=TRUE, add=TRUE, " << color[1] <<  ")" << std::endl;

    stream << "#Requirement lines to message: " << name << std::endl;
    stream << "lines(" << "c(" << serverTimeReq << "," << serverTimeReq << "), c(0,1), lty=2, " << color[2] << ")\n";
    stream << "lines(" << "c(" << clientTimeReq << "," << clientTimeReq << "), c(0,1), lty=2, " << color[3] << ")\n";

    messageScript.append(stream.str());

    return true;
}

bool RScriptGenerator::writeAndExecuteResultScript(){

    pid_t pid;
    int status;

    filestream->write(clientScript.c_str(), clientScript.size());
    filestream->write(serverScript.c_str(), serverScript.size());
    filestream->write(messageScript.c_str(), messageScript.size());

    if((pid = fork()) == 0){
        const char* const args[] = {"Rscript", "results/Rscripts/resultscript.R", NULL};
        execvp("Rscript", (char* const*)args);
    }
    else{
        waitpid(pid, &status, 0);
        if(status == EXIT_SUCCESS){
            return true;
        }else{
            return false;
        }
    }
    return true;
}

#endif // RSCRIPTGENERATOR_H
