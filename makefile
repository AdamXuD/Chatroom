OBJ=$(wildcard ./*cpp)
APP=Chatroom
CC = g++
CFLAGS = -std=c++11
LIB = -lcrypt -lmysqlclient -L /usr/local/mysql/lib/*.a -lpthread

$(APP): $(OBJ)
        $(CC) $(CFLAGS) $^ -o $@ $(LIB)
.PHONY:clean
clean:
        rm -f $(APP)
#终端输入 make clean 可以删除产生的可执行文件
