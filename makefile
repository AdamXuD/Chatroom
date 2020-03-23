CC = g++
CFLAGS = -std=c++11
LIB = -lcrypt -lmysqlclient -L /usr/local/mysql/lib/*.a -lpthread
 
all: main.cpp Client.cpp Server.cpp Common.cpp Login.cpp Friendlist.cpp Talk.cpp
	$(CC) $(CFLAGS) main.cpp Client.cpp Server.cpp Common.cpp Login.cpp Friendlist.cpp Talk.cpp -o Chatroom $(LIB)

