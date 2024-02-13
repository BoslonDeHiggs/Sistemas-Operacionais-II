C_SOURCES = pea.cpp client/client.cpp client/frontend.cpp packet/packet.cpp format/format.cpp
S_SOURCES = pea_server.cpp server/server.cpp packet/packet.cpp database/database.cpp format/format.cpp
T_SOURCES =  client/client.cpp server/server.cpp packet/packet.cpp database/database.cpp format/format.cpp tests/file_test.cpp
FLAGS = -std=c++20 -Wall -g

cmp: pea pea_server 

test: $(T_SOURCES) 
	g++ -o test $(FLAGS) $(T_SOURCES) 

pea: $(C_SOURCES)
	g++ -o pea $(FLAGS) $(C_SOURCES)

pea_server: $(S_SOURCES)
	g++ -o pea_server $(FLAGS) $(S_SOURCES)