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
#include <pthread.h>
//#include<ncurses.h>  //用户界面库
#define LOGINMODE 1 //默认关闭登录模式
#define ADMIN "Admin"
#define BUFSIZE 4096

#define COMMAND 1

#define FAILED 100
#define SUCCESS 101

#define MAKEFRIEND 110
#define DELETEFRIEND 111
#define SUKI 112
#define KIRAI 113

#define ACCEPT 121
#define REFUSE 120

#define CREATEGROUP 130
#define JOINGROUP 131
#define SETADMIN 132
#define LEAVEGROUP 133
#define KICKOFFMEMBER 134

#define LIST 140
#define QUERYFRIENDLIST 141
#define QUERYMEMBER 142

#define ALL 2
#define PRIVTALK 3
#define GROUPTALK 4
#define HEARTBEAT 5
#define PIPE 6
#define LOGIN 7
#define SIGNUP 8

using namespace std;

char content[4096];
char query[5120];

struct Msg //信息类
{
    int type;           //消息类型
    char toUser[32];    //目标账号
    char fromUser[32];  //用户来源
    char content[4096]; //内容
};

struct Account //账户类
{
    char account[32];
    char pwd[32];
    
};

void clear();                       //清屏函数
void user_wait();                   //用户等待
void input(char *ptr, const char *tips = nullptr); //输入框
void input(int &num, const char *tips = nullptr); //输入框
int input(const char *tips = nullptr);
void setMsg(Msg &msg, const int type, const char *fromUser = nullptr, const char *toUser = nullptr, const char *content = nullptr); //对消息对象赋值
bool strEqual(string str1, const char *str2);  //判断字符串是否相等（或1是否包含2）
bool strEqual(const char *str1, const char *str2); //判断字符串是否相等（或1是否包含2）

int sendMsg(Msg &msg, int fd);                   //发送消息用接口
int recvMsg(int fd, Msg &msg);                 //接收消息用接口
// int sendMsg(Account &msg, int fd);                 //发送消息用接口
// int recvMsg(int fd, Account &msg);                 //接收消息用接口
int sendMsg(char *msg, int fd);           //发送消息用接口
int recvMsg(int fd, char *msg);           //接收消息用接口

int sendHeartBeats(int fd, Msg &msg);//心跳包发送
void addepollfd(int epoll_fd, int fd); //增加监听描述符
struct tm *getTime();
