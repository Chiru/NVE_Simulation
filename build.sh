

#Check dependencies

tput setaf 4
echo "Checking dependencies..."
tput sgr0

#RScript
sudo apt-get install Rscript

#tshark
sudo apt-get install tshark


nprocs=`grep -c "^processor" /proc/cpuinfo`

#Build ns3

cd NS3
./waf -d optimized --disable-examples --disable-tests configure
./waf

if [ $? != 0 ]
then
    tput setaf 1
    echo "Error, couldn't build ns3!"
    tput sgr0
    return 1
else
    tput setaf 2
    echo "ns3 built succesfully."
    tput setaf 4
    echo "Building the NVE simulation tool..."
    tput sgr0
fi

cd ..


#Build the NVE_simulation tool
make clean

if [ $? != 0 ]
then
    tput setaf 1
    echo "Error, couldn't build the simulation tool!"
    tput sgr0
    return 1
fi

qmake

if [ $? != 0 ]
then
    tput setaf 1
    echo "Error, couldn't build the simulation tool!"
    tput sgr0
    return 1
fi

make -j $nprocs

if [ $? != 0 ]
then
    tput setaf 1
    echo "Error, couldn't build the simulation tool!"
    tput sgr0
    return 1
else
    tput setaf 2
    echo "Build has finished succesully."
    tput sgr0
fi

return 0
