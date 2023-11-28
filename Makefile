client: client.o
	g++ -o client client.cpp

server: server.o
	g++ -o server server.cpp