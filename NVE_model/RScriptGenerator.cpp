#include "RScriptGenerator.h"

RScriptGenerator::RScriptGenerator(const std::string &filename, const std::string& resultPdfFile, std::string& resultTextFile): resultPdf(resultPdfFile), resultTxt(resultTextFile){

    std::string header("# Generated R script for NVE simulator statistics\n\n");
    std::string textFile("filename = \"");
    std::string result("pdf(file=\"");
    result.append(resultPdfFile);
    textFile.append(resultTextFile);
    result.append("\")\n");
    textFile.append("\"\n");
    streamScript.assign("#Average transmit times for streams\n");
    messageScript.assign("\n#Average transmit times for each message\n");

    filestream =  new std::ofstream("results/Rscripts/resultscript.R", std::ios_base::out | std::ios_base::trunc);

    filestream->write(header.c_str(), header.size());
    filestream->write(textFile.c_str(), textFile.size());
    filestream->write(result.c_str(), result.size());

    *filestream << "\nwrite(\"Numeric results for each stream and message\\n\\n\", filename)" << std::endl;

}

RScriptGenerator::~RScriptGenerator(){

    filestream->close();
    delete filestream;
}

bool RScriptGenerator::generateScriptForStream(const std::list<int64_t>* transmitTimesToClients, const std::list<int64_t>* transmitTimesToServer, const std::list<int64_t>* transmitTimesFromServer,
                                               uint16_t maxStreams){

    int count = 0;
    std::string server("serverstream_");
    std::string client("clientstream_");
    std::string serverToClient("servertoclientstream_");
    std::string serverFunc("serverfunc_");
    std::string clientFunc("clientfunc_");
    std::string serverToClientFunc("servertoclientfunc_");

    std::string colors[] = {"\"green\"", "\"blue\"", "\"purple\"", "\"red\"", "\"grey\""};
    std::string lineTypes[] = {"lty=1", "lty=2", "lty=3", "lty=4", "lty=5"};
    std::stringstream stream;

    for(count = 0; count < maxStreams; count++){

        stream.str("");

        stream << "\n#Vector for server transmission times on stream number: " << count +1 << std::endl;
        if(!transmitTimesToServer[count].empty()){
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
        }else
            stream << server << count << " = c(0)" << std::endl;

        stream << "write(\"Mean value of client to server transmission times for stream number : " << count << "\", filename, append=TRUE)\n";
        stream << "write(mean(" << server << count << "), filename, append=TRUE)\n";
        stream << "write(\"Quantiles of client to server transmission times for stream number : " << count << " (min, 25%, median, 75%, max) \", filename, append=TRUE)\n";
        stream << "write(quantile(" << server << count << "), filename, append=TRUE)\n";

        stream << "\n#Step function for stream number: " << count +1 << "\n";
        stream << serverFunc << count << " = ecdf(" << server << count << ")\n";

        stream << "\n#Vector for client to client transmission times on stream number: " << count +1 << std::endl;

        if(!transmitTimesToClients[count].empty()){
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
        }
        else
            stream << client << count << " = c(0)" << std::endl;

        stream << "write(\"Mean value of client to client transmission times for stream number : " << count << "\", filename, append=TRUE)\n";
        stream << "write(mean(" << client << count << "), filename, append=TRUE)\n";
        stream << "write(\"Quantiles of client to client transmission times for stream number : " << count << " (min, 25%, median, 75%, max) \", filename, append=TRUE)\n";
        stream << "write(quantile(" << client << count << "), filename, append=TRUE)\n";

        stream << "\n#Step function for stream number: " << count +1 << "\n";
        stream << clientFunc << count << " = ecdf(" << client << count << ")\n";

        stream << "\n#Vector for server to client transmission time for stream number: " << count + 1 << std::endl;

        if(!transmitTimesFromServer[count].empty()){
            stream << serverToClient << count;
            stream << " = c(";

            for(std::list<int64_t>::const_iterator it = transmitTimesFromServer[count].begin(); it != transmitTimesFromServer[count].end();){
                stream << *it;

                if(++it == transmitTimesFromServer[count].end()){
                    stream << ")\n";
                    break;
                }
                else
                    stream << ", ";
            }
        }else
            stream << serverToClient << count << " = c(0)" << std::endl;

        stream << "write(\"Mean value of server to client transmission times for stream number : " << count << "\", filename, append=TRUE)\n";
        stream << "write(mean(" << serverToClient << count << "), filename, append=TRUE)\n";
        stream << "write(\"Quantiles of server to client transmission times for stream number : " << count << " (min, 25%, median, 75%, max) \", filename, append=TRUE)\n";
        stream << "write(quantile(" << serverToClient << count << "), filename, append=TRUE)\n";

        stream << "\n#Step function for stream number: " << count +1 << "\n";
        stream << serverToClientFunc << count << " = ecdf(" << serverToClient << count << ")\n";

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

    stream << serverToClient << "max = max(c(";

    for(int i = 0; i < maxStreams; i++){
        stream << serverToClient << i;
        if(i+1 == maxStreams)
            stream << "))\n";
        else
            stream << ", ";
    }


    stream << "\n#Plotting for streams\n";

    for(int count = 0; count < maxStreams; count++){
        if(count == 0){  //then this is the first plot to draw to server
            stream << "plot(" << serverFunc << count<< ", do.points=FALSE, verticals=TRUE, main=\"Average stream transmit times from client to server\", col="<< colors[count] <<
                      ", " << lineTypes[count] << ", lwd = 2, xlab=\"Time(ms)\", ylab=\"Frequency\", xlim=(c(0," << server << "max))" << ")\n";
            stream << "abline(h=0.1*0:10,  col=\"lightgray\")\n";
        }
        else
            stream << "plot(" << serverFunc << count<< ", do.points=FALSE, add=TRUE, verticals=TRUE, col="  << colors[count] << ", " << lineTypes[count]  << ", lwd = 2)\n";

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

            stream << ", lty=c(";
            for(int i = 0; i <= count; i++){
                stream  << lineTypes[i];
                if(i != count)
                    stream << ", ";
                else
                    stream << ")";
            }

            stream << ", lwd = 2, inset=.05)" << std::endl;
        }

    }

    for(int count = 0; count < maxStreams; count++){
        if(count == 0)  { //then this is the first plot to draw to client
            stream << "plot(" << clientFunc << count << ", do.points=FALSE, verticals=TRUE, main=\"Average stream transmit times from client to client\", col=" << colors[count] <<
                      ", " << lineTypes[count]  << ", lwd = 2, xlab=\"Time(ms)\", ylab=\"Frequency\", xlim=(c(0," << client << "max))" << ")\n";
            stream << "abline(h=0.1*0:10,  col=\"lightgray\")\n";
        }
        else
           stream << "plot(" << clientFunc << count << ", do.points=FALSE, add=TRUE, verticals=TRUE, col=" << colors[count] <<  ", " << lineTypes[count]  <<", lwd = 2)\n";

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

            stream << ", lty=c(";
            for(int i = 0; i <= count; i++){
                stream  << lineTypes[i];
                if(i != count)
                    stream << ", ";
                else
                    stream << ")";
            }

            stream << ", lwd = 2, inset=.05)" << std::endl;
        }
    }

    for(int count = 0; count < maxStreams; count++){
        if(count == 0)  { //then this is the first plot to draw from server to client
            stream << "plot(" << serverToClientFunc << count << ", do.points=FALSE, verticals=TRUE, main=\"Average stream transmit times from server to client\", col=" << colors[count] <<
                      ", " << lineTypes[count]  << ", lwd = 2, xlab=\"Time(ms)\", ylab=\"Frequency\", xlim=(c(0," << serverToClient << "max))" << ")\n";
            stream << "abline(h=0.1*0:10,  col=\"lightgray\")\n";
        }
        else
           stream << "plot(" << serverToClientFunc << count << ", do.points=FALSE, add=TRUE, verticals=TRUE, col=" << colors[count] << ", " << lineTypes[count]  << ", lwd = 2)\n";

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

            stream << ", lty=c(";
            for(int i = 0; i <= count; i++){
                stream  << lineTypes[i];
                if(i != count)
                    stream << ", ";
                else
                    stream << ")";
            }

            stream << ", lwd = 2, inset=.05)" << std::endl;
        }
    }

    streamScript.append(stream.str());

   return true;
}

bool RScriptGenerator::generateScriptForClientMessage(std::list<int> clientRecvTimes, std::list<int> serverRecvTimes, std::list<int> sendIntervals, const std::string &name, int serverTimeReq,
                                                int clientTimeReq, uint32_t numberOfClientsForwarded, std::list<uint16_t> sizes){


    static std::string color[2] = {"\"green\"", "\"blue\""};
    static std::string server("servermsg_");
    static std::string client("clientmsg_");
    static std::string serverFunc("servermsgfunc_");
    static std::string clientFunc("clientmsgfunc_");
    std::stringstream stream;
    uint32_t clientToServerInTime = 0;
    uint32_t clientToClientInTime = 0;

    stream << "\n#Vector of server transmission times for message: " << name << std::endl;
    stream << server << name;
    stream << " = c(";


    for(std::list<int>::const_iterator it = serverRecvTimes.begin(); it != serverRecvTimes.end();){
        stream << *it;

        if(*it <= serverTimeReq)
            clientToServerInTime++;

        if(++it == serverRecvTimes.end()){
            stream << ")\n";
            break;
        }
        else
            stream << ", ";
    }

    stream << "write(\"Mean value of client to server transmission times for message: " << name << "\", filename, append=TRUE)\n";
    stream << "write(mean(" << server << name << "), filename, append=TRUE)\n";
    stream << "write(\"Quantiles of client to server transmission times for message: " << name << " (min, 25%, median, 75%, max) \", filename, append=TRUE)\n";
    stream << "write(quantile(" << server << name << "), filename, append=TRUE)\n";
    stream << "write(\"Percentage of messages to reach server in " << serverTimeReq << " ms for message: " << name << "\", filename, append=TRUE)\n";
    stream << "write(" << ((double)clientToServerInTime/ ((double)sendIntervals.size() )) << ", filename, append=TRUE)\n";


    stream << "\n#Vector of client transmission times for message: " << name << std::endl;
    if(!clientRecvTimes.empty()){
        stream << client << name;
        stream << " = c(";

        for(std::list<int>::const_iterator it = clientRecvTimes.begin(); it != clientRecvTimes.end();){
            stream << *it;

            if(*it <= clientTimeReq)
                clientToClientInTime++;

            if(++it == clientRecvTimes.end()){
                stream << ")\n";
                break;
            }
            else
                stream << ", ";
        }
    }
    else
        stream << client << name << " = c(0)" << std::endl;


    stream << "write(\"Mean value of client to client transmission times for message: " << name << "\", filename, append=TRUE)\n";
    stream << "write(mean(" << client << name << "), filename, append=TRUE)\n";
    stream << "write(\"Quantiles of client to client transmission times for message: " << name << " (min, 25%, median, 75%, max) \", filename, append=TRUE)\n";
    stream << "write(quantile(" << client << name << "), filename, append=TRUE)\n";
    stream << "write(\"Percentage of messages from client to reach client in " << clientTimeReq << " ms for message: " << name << "\", filename, append=TRUE)\n";
    stream << "write(" << (numberOfClientsForwarded == 0 ? 0 :  (double)clientToClientInTime/ ((double)numberOfClientsForwarded)) << ", filename, append=TRUE)\n";

    stream << "\n#Vector for message send time intervals for message " << name;
    stream << "\nsendtimes_" << name << " = c(";
    for(std::list<int>::const_iterator it = sendIntervals.begin(); it != sendIntervals.end();){
        stream << *it;
        if(++it == sendIntervals.end()){
            stream << ")\n";
            break;
        }
        else
            stream << ", ";
    }

    stream << "write(\"Mean value of send time intervals for message: " << name << "\", filename, append=TRUE)\n";
    stream << "write(mean(sendtimes_" << name << "), filename, append=TRUE)\n";
    stream << "write(\"Quantiles of send time intervals for message: " << name << " (min, 25%, median, 75%, max) \", filename, append=TRUE)\n";
    stream << "write(quantile(sendtimes_" << name << "), filename, append=TRUE)\n";

    stream << "\n#Vector for message sizes for message " << name;
    stream << "\nmessagesizes_" << name << " = c(";
    for(std::list<uint16_t>::const_iterator it = sizes.begin(); it != sizes.end();){
        stream << *it;
        if(++it == sizes.end()){
            stream << ")\n";
            break;
        }
        else
            stream << ", ";
    }

    stream << "write(\"Mean value of message sizes for message: " << name << "\", filename, append=TRUE)\n";
    stream << "write(mean(messagesizes_" << name << "), filename, append=TRUE)\n";
    stream << "write(\"Quantiles of message sizes for message: " << name << " (min, 25%, median, 75%, max) \", filename, append=TRUE)\n";
    stream << "write(quantile(messagesizes_" << name << "), filename, append=TRUE)\n";

    stream << "\n#Setting max x value\n";
    stream <<  name << "_max = max(" << server << name << ", " << client << name << ", " << serverTimeReq << ", " << clientTimeReq << ")\n";

    stream << "#Lost packets\n";
    stream << server << name << "_lost = c(";

    if(sendIntervals.size() - serverRecvTimes.size() != 0){
        for(uint h = 0; h <  sendIntervals.size() - serverRecvTimes.size();){
            stream << name << "_max*2";
            if(++h == sendIntervals.size() - serverRecvTimes.size()){
                stream << ")\n";
                break;
            }
            else
                stream << ", ";
        }
    }else
        stream << ")\n";

    stream << "#Lost packets\n";
    stream << client << name << "_lost = c(";
    if(numberOfClientsForwarded - clientRecvTimes.size() > 0){
        for(uint h = 0; h <  numberOfClientsForwarded - clientRecvTimes.size();){
            stream << name << "_max*2";
            if(++h == numberOfClientsForwarded - clientRecvTimes.size()){
                stream << ")\n";
                break;
            }
            else
                stream << ", ";
        }
    }else
        stream << ")\n";


    stream << "\n#Server step function for message: " << name << std::endl;
    stream << serverFunc << name << " = ecdf(c(" << server << name << ", " << server << name << "_lost)" << ");\n";

    stream << "\n#Client step function for message: " << name << std::endl;
    stream << clientFunc << name << " = ecdf(c(" << client << name << ", " << client << name << "_lost)" << ");\n";

    stream << "\n#Plotting for message: " << name << std::endl;
    stream << "plot(" << serverFunc << name << ", do.points=FALSE, verticals=TRUE, main=\"message: " << name << "\", col=\"black\", xlim=(c(0," << name << "_max" << ")), xlab=\"Time(ms)\","
           << " ylab=\"Frequency\")" << std::endl;
    stream << "plot(" << clientFunc << name << ", do.points=FALSE, verticals=TRUE, add=TRUE, col=\"darkgrey\")" << std::endl;

    stream << "#Requirement lines to message: " << name << std::endl;
    stream << "lines(" << "c(" << serverTimeReq << "," << serverTimeReq << "), c(0,1), lty=2, col=\"black\", )\n";
    stream << "lines(" << "c(" << clientTimeReq << "," << clientTimeReq << "), c(0,1), lty=2, col=\"grey\")\n";
    stream << "#Legend\n";
    stream << "legend(\"bottomright\", c(\"client to server\", \"client to client\", \"client to server requirement\", \"client to client requirement\"), cex=0.8, col=c("
           << "\"black\", \"darkgrey\", \"black\", \"darkgrey\"), lty=c(1,1,2,2), inset=.05)" << std::endl;

    stream << "#Message generation time intervals for " << name;
    stream << "\nplot(tabulate(sendtimes_" << name << "), type=\"h\", xlab=\"Time(ms)\", ylab =\"Message count\", main=\"Message generation time intervals for: " <<  name <<"\")" << std::endl;

    stream << "\n#Packet sizes for message " << name << std::endl;
    stream << "\nplot(tabulate(messagesizes_" << name << "), type=\"h\", xlab=\"Message size(bytes)\", ylab =\"Message count\", main=\"Message sizes for: " <<  name <<"\")" << std::endl;


    messageScript.append(stream.str());

    return true;
}

bool RScriptGenerator::generateScriptForServerMessage(std::list<int> clientRecvTimes, std::list<int> sendIntervals, const std::string &name, int clientTimeReq, uint16_t numberOfClientsForwarded,
                                                      std::list<uint16_t> sizes){

    static std::string color[2] = {"\"green\"", "\"blue\""};
    static std::string client("servertoclientmsg_");
    static std::string clientFunc("servertoclientmsgfunc_");
    std::stringstream stream;
    uint32_t serverToClientInTime = 0;

    stream << "\n#Vector of server to client transmission times for message: " << name << std::endl;
    if(!clientRecvTimes.empty()){
        stream << client << name;
        stream << " = c(";

        for(std::list<int>::const_iterator it = clientRecvTimes.begin(); it != clientRecvTimes.end();){
            stream << *it;

            if(*it <= clientTimeReq)
                serverToClientInTime++;

            if(++it == clientRecvTimes.end()){
                stream << ")\n";
                break;
            }
            else
                stream << ", ";
        }
    }
    else
        stream << client << name << " = c(0)" << std::endl;

    stream << "write(\"Mean value of server to client transmission times for message: " << name << "\", filename, append=TRUE)\n";
    stream << "write(mean(" << client << name << "), filename, append=TRUE)\n";
    stream << "write(\"Quantiles of server to client transmission times for message: " << name << " (min, 25%, median, 75%, max) \", filename, append=TRUE)\n";
    stream << "write(quantile(" << client << name << "), filename, append=TRUE)\n";
    stream << "write(\"Percentage of messages to reach client in " << clientTimeReq << " ms for message: " << name << "\", filename, append=TRUE)\n";
    stream << "write(" << ((double)serverToClientInTime/ ((double)numberOfClientsForwarded)) << ", filename, append=TRUE)\n";

    stream << "\n#Setting max x value\n";
    stream <<  name << "_max = max(" << client << name << ", " << clientTimeReq << ")\n";

    stream << "#Lost packets\n";
    stream << client << name << "_lost = c(";

    if(numberOfClientsForwarded - clientRecvTimes.size() != 0){
        for(uint h = 0; h < numberOfClientsForwarded - clientRecvTimes.size();){
            stream << name << "_max*2";
            if(++h == numberOfClientsForwarded - clientRecvTimes.size()){
                stream << ")\n";
                break;
            }
            else
                stream << ", ";
        }
    }else
        stream << ")\n";

    stream << "\n#Vector for message send time intervals for server message " << name;
    stream << "\nsendtimes_" << name << " = c(";
    for(std::list<int>::const_iterator it = sendIntervals.begin(); it != sendIntervals.end();){
        stream << *it;
        if(++it == sendIntervals.end()){
            stream << ")\n";
            break;
        }
        else
            stream << ", ";
    }

    stream << "write(\"Mean value of send time intervals for message: " << name << "\", filename, append=TRUE)\n";
    stream << "write(mean(sendtimes_" << name << "), filename, append=TRUE)\n";
    stream << "write(\"Quantiles of send time intervals for message: " << name << " (min, 25%, median, 75%, max) \", filename, append=TRUE)\n";
    stream << "write(quantile(sendtimes_" << name << "), filename, append=TRUE)\n";

    stream << "\n#Vector for message sizes for message " << name;
    stream << "\nmessagesizes_" << name << " = c(";
    for(std::list<uint16_t>::const_iterator it = sizes.begin(); it != sizes.end();){
        stream << *it;
        if(++it == sizes.end()){
            stream << ")\n";
            break;
        }
        else
            stream << ", ";
    }

    stream << "write(\"Mean value of message sizes for message: " << name << "\", filename, append=TRUE)\n";
    stream << "write(mean(messagesizes_" << name << "), filename, append=TRUE)\n";
    stream << "write(\"Quantiles of message sizes for message: " << name << " (min, 25%, median, 75%, max) \", filename, append=TRUE)\n";
    stream << "write(quantile(messagesizes_" << name << "), filename, append=TRUE)\n";

    stream << "\n#Server to client step function for message: " << name << std::endl;
    stream << clientFunc << name << " = ecdf(c(" << client << name << ", " << client << name << "_lost)" << ");\n";


    stream << "\n#Plotting for message: " << name << std::endl;
    stream << "plot(" << clientFunc << name << ", do.points=FALSE, verticals=TRUE, main=\"message: " << name << "\", col=\"black\", xlim=(c(0," << name << "_max" << ")), xlab=\"Time(ms)\","
           << " ylab=\"Frequency\")" << std::endl;

    stream << "#Requirement lines to message: " << name << std::endl;
    stream << "lines(" << "c(" << clientTimeReq << "," << clientTimeReq << "), c(0,1), lty=2, col=\"black\")\n";
    stream << "#Legend\n";
    stream << "legend(\"bottomright\", c(\"server to client\", \"server to client requirement\"), cex=0.8, col=c("
              << "\"black\", \"darkgrey\"), lty=c(1,2), inset=.05)" << std::endl;

    stream << "#Message generation time intervals for " << name;
    stream << "\nplot(tabulate(sendtimes_" << name << "), type=\"h\", xlab=\"Time(ms)\", ylab =\"Message count\", main=\"Message genereation time intervals for: " <<  name <<"\")" << std::endl;

    stream << "\n#Packet sizes for message " << name << std::endl;
    stream << "\nplot(tabulate(messagesizes_" << name << "), type=\"h\", xlab=\"Message size(bytes)\", ylab =\"Message count\", main=\"Message sizes for: " <<  name <<"\")" << std::endl;

    messageScript.append(stream.str());

    return true;
}

bool RScriptGenerator::generateBandwidthHistogram(double clientDownlink, double clientUplink, double serverDownlink, double serverUplink){

    std::string color[2] = {"\"red\"", "\"blue\""};
    std::stringstream stream;

    stream << "\n#Barplot for bandwidths\n";
    stream << "maxbandwidth = max(" << serverDownlink << ", " << serverUplink << ", " << clientDownlink << ", " << clientUplink << ")";
    stream << "\nbarplot(c(" << clientDownlink <<  ", " << clientUplink << ", " << serverDownlink << ", " << serverUplink << "), names.arg=c(\"avg. client dl\","
                                                     << " \"avg. client ul\", \"server downlink\", \"server uplink\"), col=c(" << color[0] << ", " << color[1]
                                                     << "), ylim=c(0, maxbandwidth * 1.2))" << std::endl;
    stream << "title(main=\"Throughputs\",  ylab=\"Bandwidth (Mbps)\")" << std::endl;
    stream << "abline(h=seq(from=0, to=maxbandwidth, by = (signif(maxbandwidth, 1)/(signif(maxbandwidth, 1)*10))/2), col =\"lightgrey\")";

    messageScript.append(stream.str());
    return true;
}


bool RScriptGenerator::addClientBandwidth(const Ipv4Address &addr, double downLink, double upLink, bool isClient)
{
    std::stringstream stream;

    stream << "\nwrite(\"" << (isClient ? "Average throughput for client " : "Average throughput for server ") << addr << " uplink: " << upLink
           << " downlink " << downLink << " Mbps\n\", filename, append=TRUE)\n";

    messageScript.append(stream.str());
    return true;

}


bool RScriptGenerator::parsePcapStats(const std::string &sourceFile, bool size, const Ipv4Address& addr, int clientNumber)
{
    std::stringstream stream;
    std::ifstream file(sourceFile.c_str());
    int value;
    double tempValue;
    bool first = true;

    if(size)
    {
        stream << "\n\n#Network  packet sizes for node: " << addr << "\n";
        stream << "sendsizes_client_"  << clientNumber << " = c(";
    }
    else
    {
        stream << "\n\n#Network packet send time intervals for node: " << addr << "\n";
        stream << "sendtimes_client_"  << clientNumber << " = c(";
    }

    if(file.fail())
        return false;

    if(!size)       //with time intervals, the first value is always 0
    {
        file >> tempValue;
    }


    while(!file.eof())
    {
        if(first)
            first = false;
        else
            stream << ", ";

        if(size)
        {
            file >> value;
        }
        else
        {
            file >> tempValue;
            value = int(1000*tempValue + 0.5);
        }

        stream << value;
    }

    stream << ")\n";

    if(size)
    {
        stream << "plot(tabulate(sendsizes_client_" << clientNumber << "), type=\"h\", xlab=\"Packet size(bytes)\", ylab=\"Message count\", ";
        stream << "main=\"Network packet sizes for client " << clientNumber << " (" << addr << ")\"\n)";
    }
    else
    {
        stream << "plot(tabulate(sendtimes_client_" << clientNumber << "), type=\"h\", xlab=\"Time interval (ms)\", ylab=\"Message count\", ";
        stream << "main=\"Network packet send intervals for client " << clientNumber << " (" << addr << ")\"\n)";
    }

    file.close();
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




