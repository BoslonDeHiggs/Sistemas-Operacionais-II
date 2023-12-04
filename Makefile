C_SOURCES = c_app.cpp client/client.cpp
S_SOURCES = s_app.cpp server/server.cpp
FLAGS = -std=c++11 -Wall -g

cmp: c_app s_app 

c_app: $(C_SOURCES)
	g++ -o c_app $(FLAGS) $(C_SOURCES)

s_app: $(S_SOURCES)
	g++ -o s_app $(FLAGS) $(S_SOURCES)