

#Check dependencies

#RScript
#tshark

#Build ns3
cd NS3
./waf -d optimized --disable-examples --disable-tests configure
./waf
cd ..

#Build the NVE_simulation tool
qmake
make

