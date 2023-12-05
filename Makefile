C_SOURCES = aep.cpp client/client.cpp packet/packet.cpp
S_SOURCES = aep_server.cpp server/server.cpp packet/packet.cpp
FLAGS = -std=c++11 -Wall -g

cmp: aep aep_server 

aep: $(C_SOURCES)
	g++ -o aep $(FLAGS) $(C_SOURCES)

aep_server: $(S_SOURCES)
	g++ -o aep_server $(FLAGS) $(S_SOURCES)