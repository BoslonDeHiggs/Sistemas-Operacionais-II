CC = g++
all: main
main: main.o client.o
	${CC} -o main main.o client.o
main.o: main.cpp client.cpp
	${CC} -c main.cpp client.cpp