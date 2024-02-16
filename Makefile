C_SOURCES = src/exe_client.cpp src/client/client.cpp src/packet/packet.cpp src/format/format.cpp
S_SOURCES = src/exe_server.cpp src/server/server.cpp src/packet/packet.cpp src/database/database.cpp src/format/format.cpp
T_SOURCES = src/client/client.cpp src/server/server.cpp src/packet/packet.cpp src/database/database.cpp src/format/format.cpp src/tests/file_test.cpp
FLAGS = -std=c++20 -Wall
BIN_DIR = bin

# Make bin directory if it doesn't exist
$(shell mkdir -p $(BIN_DIR))

cmp: $(BIN_DIR)/client $(BIN_DIR)/server

test: $(T_SOURCES) 
	g++ -o $(BIN_DIR)/test $(FLAGS) $(T_SOURCES) 

$(BIN_DIR)/client: $(C_SOURCES)
	g++ -o $@ $(FLAGS) $(C_SOURCES)

$(BIN_DIR)/server: $(S_SOURCES)
	g++ -o $@ $(FLAGS) $(S_SOURCES)