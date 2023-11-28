# Makefile for compiling client.cpp and server.cpp

# Compiler and flags
CPP = g++
CPPFLAGS = -std=c++11 -Wall

# Source files and target executables
CLIENT_SRCS = client.cpp
SERVER_SRCS = server.cpp
CLIENT_TARGET = client
SERVER_TARGET = server

cmp: $(CLIENT_TARGET) $(SERVER_TARGET)

# Rule to build the client executable
$(CLIENT_TARGET): $(CLIENT_SRCS)
	$(CPP) $(CPPFLAGS) $(CLIENT_SRCS) -o $(CLIENT_TARGET)

# Rule to build the server executable
$(SERVER_TARGET): $(SERVER_SRCS)
	$(CPP) $(CPPFLAGS) $(SERVER_SRCS) -o $(SERVER_TARGET)

# Phony target to clean the project
.PHONY: clean
clean:
	rm -f $(CLIENT_TARGET) $(SERVER_TARGET)
