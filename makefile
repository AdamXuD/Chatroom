CC = g++
CFLAGS = -std=c++11
LIB = -lcrypt -lmysqlclient -L /usr/local/mysql/lib/*.a
 
all: main.cpp Client.cpp Server.cpp Common.cpp 
	$(CC) $(CFLAGS) main.cpp Client.cpp Server.cpp Common.cpp -o Chatroom $(LIB)

