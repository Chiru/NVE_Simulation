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
    bool generateScriptForStream(const std::list<int64_t>* transmitTimesToClients,  const std::list<int64_t>* transmitTimesToServer, uint16_t maxStreams);
    bool generateScriptForMessage(std::list<int> clientRecvTimes, std::list<int> serverRecvTimes, const std::string& name, int serverTimeReq, int clientTimeReq);
    bool writeAndExecuteResultScript();

private:
    std::ofstream* filestream;
    std::string result;
    std::string streamScript;
    std::string messageScript;
};

RScriptGenerator::RScriptGenerator(const std::string &filename, const std::string& resultFile): result(resultFile){

    std::string header("# Generated R script for NVE simulator statistics\n\n ");
    std::string result("pdf(file=\"");
    result.append(resultFile);
    result.append("\")\n");
    streamScript.assign("#Average trasnmit times for streams\n");
    messageScript.assign("\n#Average trasnmit times for each message\n");

    filestream =  new std::ofstream("results/Rscripts/resultscript.R", std::ios_base::out | std::ios_base::trunc);

    filestream->write(header.c_str(), header.size());
    filestream->write(result.c_str(), result.size());
}

RScriptGenerator::~RScriptGenerator(){

    filestream->close();
    delete filestream;
}

bool RScriptGenerator::generateScriptForStream(const std::list<int64_t>* transmitTimesToClients, const std::list<int64_t>* transmitTimesToServer, uint16_t maxStreams){

    int count = 0;
    std::string server("serverstream_");
    std::string client("clientstream_");
    std::string serverFunc("serverfunc_");
    std::string clientFunc("clientfunc_");
    std::string colors[] = {"\"green\"", "\"blue\"", "\"cyan\"", "\"yellow\"", "\"grey\""};
    std::stringstream stream;

    for(count = 0; count < maxStreams; count++){

        stream.str("");

        stream << "\n#Vector for server receive times on stream number: " << count +1 << std::endl;
        stream << server << count;
        stream << " = c(";
        for(std::list<int64_t>::const_iterator it = transmitTimesToServer[count].begin(); it != transmitTimesToServer[count].end();){
            stream << *it;

            if(++it == transmitTimesToServer[count].end()){
                stream << ")\n";
                break;
            }
            else
                stream << ", ";
        }

        stream << "\n#Step function for stream number: " << count +1 << "\n";
        stream << serverFunc << count << " = ecdf(" << server << count << ")\n";

        stream << "\n#Vector for client receive times on stream number: " << count +1 << std::endl;
        stream << client << count;
        stream << " = c(";

        for(std::list<int64_t>::const_iterator it = transmitTimesToClients[count].begin(); it != transmitTimesToClients[count].end();){
            stream << *it;

            if(++it == transmitTimesToClients[count].end()){
                stream << ")\n";
                break;
            }
            else
                stream << ", ";
        }

        stream << "\n#Step function for stream number: " << count +1 << "\n";
        stream << clientFunc << count << " = ecdf(" << client << count << ")\n";

       streamScript.append(stream.str());

    }

    stream.str("");

    stream << "\n#Max x values\n";
    stream << server << "max = max(c(";

    for(int i = 0; i < maxStreams; i++){
        stream << server << i;
        if(i+1 == maxStreams)
            stream << "))\n";
        else
            stream << ", ";
    }

    stream << client << "max = max(c(";

    for(int i = 0; i < maxStreams; i++){
        stream << client << i;
        if(i+1 == maxStreams)
            stream << "))\n";
        else
            stream << ", ";
    }

    stream << "\n#Plotting for streams\n";

    for(int count = 0; count < maxStreams; count++){
        if(count == 0){  //then this is the first plot to draw to server
            stream << "plot(" << serverFunc << count<< ", do.points=FALSE, verticals=TRUE, main=\"Average stream transmit times from client to server\", col="<< colors[count] <<
                      ", xlab=\"Time(ms)\", ylab=\"Frequency\", xlim=(c(0," << server << "max))" << ")\n";
            stream << "abline(h=0.1*0:10,  col=\"lightgray\")\n";
        }
        else
            stream << "plot(" << serverFunc << count<< ", do.points=FALSE, add=TRUE, verticals=TRUE, col="  << colors[count] << ")\n";

        if(count +1 == maxStreams){
            stream << "#Legend\n";
            stream << "legend(\"bottomright\", c(";
            for(int i = 0; i <= count; i++){
                stream << "\"stream " << i+1 << "\"";
                if(i != count)
                    stream << ", ";
                else
                    stream << ")";
            }
            stream << ", cex=0.8, col=c(";
            for(int i = 0; i <= count; i++){
                stream << "col=" << colors[i];
                if(i != count)
                    stream << ", ";
                else
                    stream << ")";
            }

            stream << ", lty=1, inset=.05)" << std::endl;
        }

    }

    for(int count = 0; count < maxStreams; count++){
        if(count == 0)  { //then this is the first plot to draw to client
            stream << "plot(" << clientFunc << count << ", do.points=FALSE, verticals=TRUE, main=\"Average stream transmit times from client to client\", col=" << colors[count] <<
                      ", xlab=\"Time(ms)\", ylab=\"Frequency\", xlim=(c(0," << client << "max))" << ")\n";
            stream << "abline(h=0.1*0:10,  col=\"lightgray\")\n";
        }
        else
           stream << "plot(" << clientFunc << count << ", do.points=FALSE, add=TRUE, verticals=TRUE, col=" << colors[count] << ")\n";

        if(count +1 == maxStreams){
            stream << "#Legend\n";
            stream << "legend(\"bottomright\", c(";
            for(int i = 0; i <= count; i++){
                stream << "\"stream " << i+1 << "\"";
                if(i != count)
                    stream << ", ";
                else
                    stream << ")";
            }
            stream << ", cex=0.8, col=c(";
            for(int i = 0; i <= count; i++){
                stream << "col=" << colors[i];
                if(i != count)
                    stream << ", ";
                else
                    stream << ")";
            }

            stream << ", lty=1, inset=.05)" << std::endl;
        }
    }

    streamScript.append(stream.str());

   return true;
}

bool RScriptGenerator::generateScriptForMessage(std::list<int> clientRecvTimes, std::list<int> serverRecvTimes, const std::string &name, int serverTimeReq, int clientTimeReq){

    static std::string color[2] = {"\"green\"", "\"blue\""};
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
    stream <<  name << "_max = max(" << server << name << ", " << client << name << ", " << serverTimeReq << ", " << clientTimeReq << ")\n";

    stream << "\n#Plotting for message: " << name << std::endl;
    stream << "plot(" << serverFunc << name << ", do.points=FALSE, verticals=TRUE, main=\"message: " << name << "\", col=" << color[0] << ", xlim=(c(0," << name << "_max" << ")), xlab=\"Time(ms)\","
           << " ylab=\"Frequency\")" << std::endl;
    stream << "abline(h=0.1*0:10,  col=\"lightgray\")\n";
    stream << "plot(" << clientFunc << name << ", do.points=FALSE, verticals=TRUE, add=TRUE, col=" << color[1] <<  ")" << std::endl;

    stream << "#Requirement lines to message: " << name << std::endl;
    stream << "lines(" << "c(" << serverTimeReq << "," << serverTimeReq << "), c(0,1), lty=2, col=" << color[0] << ")\n";
    stream << "lines(" << "c(" << clientTimeReq << "," << clientTimeReq << "), c(0,1), lty=2, col=" << color[1] << ")\n";
    stream << "#Legend\n";
    stream << "legend(\"bottomright\", c(\"client to server\", \"client to client\", \"client to server requirement\", \"client to client requirement\"), cex=0.8, col=c("
           << color[0] << ", " << color[1] << ", " << color[0] << ", " << color[1] << "), lty=c(1,1,2,2), inset=.05)" << std::endl;

    messageScript.append(stream.str());

    return true;
}

bool RScriptGenerator::writeAndExecuteResultScript(){

    pid_t pid;
    int status;

    filestream->write(streamScript.c_str(), streamScript.size());
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
