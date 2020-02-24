#pragma once
#include "Common.h"

class Client
{
public:
    Client();      //构造函数
    void Connect(); //连接函数

    void Login();   //登录
    void Login(Account acc); //登录
    void Signup(); //注册
    void fileLogin(); //本地凭据登录
    static void *sendHeartBeats(void *pointer); //定时发送心跳包
    void dealwithmsg(char *Target);
    void dealwithQuery(string command);

    void Start();
    /*好友列表部分*/
    void makeFriend(string command);                   //添加好友
    void deleteFriend(string command);                 //删除好友
    void queryFriendList(); //请求好友列表
    void setSuki(string command);  //设为特别关心
    void setKirai(string command);          //拉黑名单
    /*好友列表部分*/

    /*群聊相关权限部分*/
    void createGroupTalk(string command);      //创建群聊
    void setGroupAdmin(string command, string toGroup); //设置管理员
    void joinGroup(string command);                     //加入群聊
    void leaveGroup(string Group);           //主动离开群聊
    void deleteGroupMember(string command, string Group);  //踢人
    void Grouptalk(string command);            //处理用户群聊请求
    void queryGroupMember(string Group);
    /*群聊与相关权限部分*/

    /*私聊部分*/
    void Privatetalk(string command); //处理用户私聊请求
    /*私聊部分*/
    ~Client();

    int sock_fd; //要连接的服务器句柄（句柄应该是这个意思吧
    struct Msg msg; //消息
private:
    int pipe_fd[2];              //读写管道pipe()……
    int pid;                     //多进程时fork()返回进程号时用得到
    int epoll_fd;                //epoll二叉树根的句柄，真的玄妙
    bool isLogin;                //登陆状态
    map<string, int> friendlist;  //用list类存储好友列表
    struct Account acc; //账号信息
protected:
};
