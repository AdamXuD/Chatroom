#pragma once

#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <crypt.h>
#include <fcntl.h>
#include <mysql/mysql.h>
#include <map>
//#include<ncurses.h>  //用户界面库
#define LOGINMODE 0 //默认关闭登录模式

#define COMMAND 1
#define PIPE 2
#define ALL 3
#define PRIVTALK 4
#define GROUPTALK 5

using namespace std;

struct Msg //信息类
{
    int type;           //消息类型
    char toUser[32];    //目标账号
    char fromUser[32];  //用户来源
    char content[4096]; //内容
};

struct Account //账户类
{
    bool flag; //判断传输的账户类是否为空
    char account[32];
    char pwd[32];
};

void clear();                       //清屏函数
void user_wait();                   //用户等待
int sendMsg(const Msg msg, int fd); //发送消息用接口
int recvMsg(int fd, Msg &msg);      //接收消息用接口
void addepollfd(int epoll_fd, int fd); //增加监听描述符
