#pragma once
#include "Common.h"

class Client
{
public:
    Client();       //构造函数
    void Connect(); //连接函数

    void Login();                            //登录
    void Login(Account acc);                 //登录
    void Signup();                           //注册
    void fileLogin();                        //本地凭据登录
    static void *HeartBeat(void *pointer);   //定时发送心跳包
    void dealwithmsg(char *Target);
    void Start();
    void dealWithQuery(string command);
    void getOnlineFriends();
    void getHistory(string command);

    void mainMenu();
    string friendlistMenu(bool isFriend);
    void groupMenu(string toGroup);
    int getQueryBox(bool show);

    /*好友列表部分*/
    void queryFriendList(bool show); //请求好友列表
    /*好友列表部分*/

    /*群聊相关权限部分*/
    void Grouptalk(string command);
    void setGroupAdmin(string command, string toGroup);
    void leaveGroup(string toGroup);
    void deleteGroupMember(string command, string toGroup);
    void queryGroupMember(const char *Group, bool show);
    /*群聊与相关权限部分*/

    /*私聊部分*/
    void Privatetalk(string command); //处理用户私聊请求
    /*私聊部分*/
    ~Client();

    int sock_fd;    //要连接的服务器句柄（句柄应该是这个意思吧
    struct Msg msg; //消息
private:
    int pipe_fd[2]; //读写管道pipe()……
    int listpipe_fd[2]; //列表接收管道
    int pid;        //多进程时fork()返回进程号时用得到
    int epoll_fd;   //epoll二叉树根的句柄，真的玄妙
    bool isLogin;       //登陆状态
    struct Account acc; //账号信息
    map<string, int> friendlist; //用list类存储好友列表
    map<string, int> memberlist;
    map<int, string> Querybox; //我觉得我真的是太依赖STL了

protected:
};
