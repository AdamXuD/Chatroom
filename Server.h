#pragma once
#include "Common.h"

class Server
{
public:
    Server();
    void Prepare();
    void Mysql_query(MYSQL *mysql, const char *q);

    void adminMsg(char *content, char *target); //错误反馈

    void addonlinelist(int clnt_fd, char *acc = nullptr);//储存在线用户
    void BroadcastMsg(int call);               //群发
    void Onlineremind(int call);               //上线提醒
    void Login(int call);                      //登录处理函数
    void Signup(int call);                     //注册处理函数
    static void *dealWithHeartbeat(void *ptr); //处理心跳包
    void dealwithQuery();
    void addFriend(char *account, char *whichfriend);
    int groupPermission(char *Group, char *Member);

    /*===========================================待完成部分===========================================*/
    /*好友列表部分*/
    bool targetExisted(bool isFriend);
    void makeFriendQuery();                              //添加好友
    void deleteFriend(); //删除好友
    void setFriendFlag();          //设为特别关心/黑名单
    void createFriendList(); //注册完成时创建该用户的好友列表
    void sendFriendList(int call);   //发送好友列表（登录后马上调用，以获取该用户的好友列表）
    void createQuerybox();

    /*好友列表部分*/

    /*群聊及其权限部分*/
    void sendAdminMsg(Msg message, bool sw_query, char *fromUser = nullptr, char *Group = nullptr);
    void SendGroupMember(int call);

    void createGroupTalk(); //创建群聊
    void setGroupAdmin();   //设置管理员
    void addGroupMember(char *Group, char *Member);
    void joinGroupQuery();  //处理加入群聊请求
    void leaveGroup();      //处理离开群聊请求
    void deleteGroupMember(); //处理踢出群聊请求
    void Grouptalk(Msg message, int call = -1); //处理用户群聊请求
    /*群聊及其权限部分*/

    /*私聊部分*/
    void Privatetalk(Msg msg); //处理用户私聊请求
    /*私聊部分*/
    /*===========================================待完成部分===========================================*/

    void dealWithMsg(int call); //消息分类处理
    void Start();               //服务端程序入口
    ~Server();//析构函数

private:
    int sock_fd;                 //socket产生的句柄
    int epoll_fd;                //epoll_create()返回的句柄
    map<int, pair<string, int>> onlinelist; //用map记录当前socket和<在线用户名，还有心跳包参数>（一一对应）
    Msg recv_msg;                     //接收到的消息
    Msg send_msg;                     //发送的消息
    MYSQL mysql; //mysql句柄
    Account acc;
    fstream mysql_log;
protected:
};
/*服务端类结束*/
