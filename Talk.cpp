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
void Client::Grouptalk() //处理用户群聊请求
{
}
/*群聊与相关权限部分*/

/*私聊部分*/
void Client::Privatetalk() //处理用户私聊请求
{
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
void Server::Grouptalk(Msg &msg, int call) //处理用户群聊请求
{
}
/*群聊及其权限部分*/
/*私聊部分*/
void Server::Privatetalk(Msg &msg) //处理用户私聊请求
{
}
/*私聊部分*/
/*服务端部分*/
