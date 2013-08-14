/**
* Copyright (c) 2013 Center for Internet Excellence, University of Oulu, All Rights Reserved
* For conditions of distribution and use, see copyright notice in license.txt
*/


#include <QtGui/QApplication>
#include "mainwindow.h"
#include "simulation_interface.h"
#include <cstring>

int main(int argc, char *argv[])
{


    if(argc == 1)     //the simulation tool is executed with graphical user interface
    {
        QApplication a(argc, argv);
        MainWindow w;
        w.show();
        return a.exec();
    }
    else              //the execution has started from the console
    {

        std::string filename;
        bool verbose = false;
        bool clientLog = false;
        bool serverLog = false;
        bool help = false;

        for(int i = 0; i < argc; i++)
        {
            if(strcmp(argv[i], "--verbose") == 0)
            {
                verbose = clientLog = serverLog = true;
            }

            if(strcmp(argv[i], "--serverLog") == 0)
            {
                serverLog = true;
            }

            if(strcmp(argv[i], "--clientLog") == 0)
            {
                clientLog = true;
            }

            if(strcmp(argv[i], "--filename") == 0)
            {
                if(++i >= argc)
                {
                    filename = "";
                    break;
                }
                else
                {
                    filename = std::string(argv[i]);
                }
            }

            if(strcmp(argv[i], "--help") == 0)
            {
                help = true;
            }
        }

        Args args(verbose, clientLog, serverLog, help, filename);

        int retval = start(args);

        if(retval == EXIT_SUCCESS)
            std::cout << "Gathering results and generating graphs..." << std::endl;

        deleteStats();

        return retval;
    }
}
