#ifndef SIMULATION_INTERFACE_H
#define SIMULATION_INTERFACE_H
#include <string>
#include "../GUI/mainwindow.h"

//The purpose of this file is only to give the start function signature to the module that
//wants to run the simulation. Also a class to pass arguments is defined.

class Args
{
public:
    Args(bool verbose, bool clientLog, bool serverLog, bool help, const std::string& fileName);
    Args(const std::string& fileName);

    bool isVerbose() const {return verbose;}
    bool serverLoggingEnabled() const {return serverLog;}
    bool clientLoggingEnabled() const {return clientLog;}
    bool needHelp() const {return help;}
    std::string getFileName() const {return fileName;}

private:
    bool verbose;
    bool serverLog;
    bool clientLog;
    bool help;
    std::string fileName;
};

void deleteStats();
int start(Args args, MainWindow* mw = 0);


#endif // SIMULATION_INTERFACE_H
