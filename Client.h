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
    void dealWithQuery(string command);
    void mainMenu();

    void Start();
    /*好友列表部分*/
    void makeFriend();                          //添加好友
    void deleteFriend();                        //删除好友
    void queryFriendList(); //请求好友列表
    void setSuki();           //设为特别关心
    void setKirai();          //拉黑名单
    /*好友列表部分*/

    /*群聊相关权限部分*/
    void createGroupTalk();      //创建群聊
    void setGroupAdmin();        //设置管理员
    void joinGroup();            //加入群聊
    void leaveGroup();           //主动离开群聊
    void deleteGroupMember();    //踢人
    void Grouptalk();            //处理用户群聊请求
    /*群聊与相关权限部分*/
    
    /*私聊部分*/
    void Privatetalk(); //处理用户私聊请求
    /*私聊部分*/
    ~Client();

    int sock_fd; //要连接的服务器句柄（句柄应该是这个意思吧
    struct Msg msg; //消息
private:
    int pipe_fd[2];              //读写管道pipe()……
    int pid;                     //多进程时fork()返回进程号时用得到
    int epoll_fd;                //epoll二叉树根的句柄，真的玄妙
    bool isLogin;                //登陆状态
    map<int, string> onlinelist; //用map记录在线列表（包含名称和）（一一对应）
    struct Account acc;          //账号信息
protected:
};
