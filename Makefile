CC = g++
all: main
	./main
main: main.o
	${CC} -o main main.o
main.o: main.cpp
	${CC} -c main.cpp