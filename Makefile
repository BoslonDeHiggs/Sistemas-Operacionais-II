C_SOURCES = pea.cpp client/client.cpp packet/packet.cpp
S_SOURCES = pea_server.cpp server/server.cpp packet/packet.cpp database/database.cpp
FLAGS = -std=c++11 -Wall -g

cmp: pea pea_server 

pea: $(C_SOURCES)
	g++ -o pea $(FLAGS) $(C_SOURCES)

pea_server: $(S_SOURCES)
	g++ -o pea_server $(FLAGS) $(S_SOURCES)