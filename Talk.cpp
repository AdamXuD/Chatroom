#include "Server.h"
#include "Client.h"

/*客户端部分*/
/*群聊相关权限部分*/
void Client::createGroupTalk() //创建群聊
{
}
void Client::setGroupAdmin() //设置管理员
{
}
void Client::joinGroup() //加入群聊
{
}
void Client::leaveGroup() //主动离开群聊
{
}
void Client::deleteGroupMember() //踢人
{
}
void Client::Grouptalk(string command) //处理用户群聊请求
{
    string toGroup;
    char buf[65535];
    toGroup = command.substr(sizeof("group ") - 1);
    cout << "你正在 " << toGroup << " 群内发言：" << endl;
    msg.type = PIPE;
    strcpy(msg.toUser, toGroup.c_str());
    memset(buf, 0, sizeof(buf));
    memcpy(buf, &msg, sizeof(msg));
    write(pipe_fd[1], buf, sizeof(buf));
    while (1)
    {
        string tmp;
        getline(cin, tmp, '\n');
        if (tmp == "_exit")
        {
            msg.type = PIPE;
            strcpy(msg.toUser, "\0");
            memset(buf, 0, sizeof(buf));
            memcpy(buf, &msg, sizeof(msg));
            write(pipe_fd[1], buf, sizeof(buf));
            break;
        }
        else
        {
            msg.type = GROUPTALK;
            strcpy(msg.fromUser, acc.account);
            strcpy(msg.toUser, toGroup.c_str());
            strcpy(msg.content, tmp.c_str());
            memset(buf, 0, sizeof(buf));
            memcpy(buf, &msg, sizeof(msg));
            write(pipe_fd[1], buf, sizeof(buf));
        }
    }
}
/*群聊与相关权限部分*/

/*私聊部分*/
void Client::Privatetalk(string command) //处理用户私聊请求
{
    string toUser;
    char buf[65535];
    toUser = command.substr(sizeof("private ") - 1);
    cout << "你正在与 " << toUser << " 聊天：" << endl;
    msg.type = PIPE;
    strcpy(msg.toUser, toUser.c_str());
    memset(buf, 0, sizeof(buf));
    memcpy(buf, &msg, sizeof(msg));
    write(pipe_fd[1], buf, sizeof(buf));
    while (1)
    {
        string tmp;
        getline(cin, tmp, '\n');
        if (tmp == "_exit")
        {
            msg.type = PIPE;
            strcpy(msg.toUser, "\0");
            memset(buf, 0, sizeof(buf));
            memcpy(buf, &msg, sizeof(msg));
            write(pipe_fd[1], buf, sizeof(buf));
            break;
        }
        else
        {
            msg.type = PRIVTALK;
            strcpy(msg.fromUser, acc.account);
            strcpy(msg.toUser, toUser.c_str());
            strcpy(msg.content, tmp.c_str());
            memset(buf, 0, sizeof(buf));
            memcpy(buf, &msg, sizeof(msg));
            write(pipe_fd[1], buf, sizeof(buf));
        }
    }
}
/*私聊部分*/
/*客户端部分*/

/*服务端部分*/
/*群聊及其权限部分*/
void Server::createGroupTalk() //创建群聊
{
}
void Server::setGroupAdmin() //设置管理员
{
}
void Server::joinGroup() //处理加入群聊请求
{
}
void Server::leaveGroup() //处理离开群聊请求
{
}
void Server::deleteGroupMember() //处理踢出群聊请求
{
}
void Server::Grouptalk(int call) //处理用户群聊请求
{
}
/*群聊及其权限部分*/
/*私聊部分*/
void Server::Privatetalk(int call) //处理用户私聊请求
{
    map<int, pair<string, int>>::iterator i;

    for (i = onlinelist.begin(); i != onlinelist.end();i++)
    {
        if(i->second.first == msg.toUser)
        {
            sendMsg(msg, i->first);
        }
    }
}
/*私聊部分*/
/*服务端部分*/
