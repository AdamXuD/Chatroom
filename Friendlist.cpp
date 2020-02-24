#include "Client.h"
#include "Server.h"

/*客户端部分*/
void Client::makeFriend(string command) //添加好友
{
    memset(&msg, 0, sizeof(msg));
    msg.type = COMMAND;
    strcpy(msg.toUser, command.substr(sizeof("private ") - 1).c_str());
    strcpy(msg.fromUser, acc.account);
    strcpy(msg.content, "MAKEFRIEND");
    sendMsg(msg, sock_fd);
}
void Client::deleteFriend() //删除好友
{
}
void Client::queryFriendList() //请求好友列表
{
}
void Client::setSuki(string command) //设为特别关心
{
}
void Client::setKirai(string command) //拉黑名单
{
}
void Client::dealwithQuery(string command)
{
    msg.type = COMMAND;
    strcpy(msg.fromUser, acc.account);
    strcpy(msg.content, command.c_str());
    sendMsg(msg, sock_fd);
}

/*客户端部分*/

/*服务端部分*/
void Server::makeFriendQuery() //添加好友
{
    char query[5120];
    sprintf(query, "select account from userinfo where account='%s';", msg.toUser);
    if (mysql_query(&mysql, query) == 0)
    {
        MYSQL_RES res;
        MYSQL_ROW row;
        res = *mysql_store_result(&mysql);
        row = mysql_fetch_row(&res);
        if (row != NULL)
        {
            Msg tmp;
            memset(&tmp, 0, sizeof(tmp));
            tmp.type = PRIVTALK;
            strcpy(tmp.fromUser, "Admin");
            strcpy(tmp.toUser, msg.toUser);
            sprintf(tmp.content, "用户\033[33m %s \033[0m请求添加你为好友！", msg.fromUser);
            sprintf(query, "insert into %s_querybox values (null, '%s', '%s', '%s');", msg.toUser, "FRIEND", msg.fromUser, tmp.content);
            if (mysql_query(&mysql, query) == 0)
            {
                Privatetalk(tmp);
            }
        }
        else
        {
            char tmpusr[32];
            strcpy(tmpusr, msg.fromUser);
            memset(&msg, 0, sizeof(msg));
            msg.type = PRIVTALK;
            strcpy(msg.fromUser, "Admin");
            strcpy(msg.toUser, tmpusr);
            strcpy(msg.content, "好友添加请求失败，请确认该用户是否存在！");
            Privatetalk(msg);
        }
    }
}
void Server::dealwithQuery()
{
    string command = msg.content;
    int queryID;
    char query[1024];
    queryID = atoi(command.substr(sizeof("accept ") - 1).c_str());
    sprintf(query, "select type, fromuser from %s_querybox where id = %d;", msg.fromUser, queryID);
    if (mysql_query(&mysql, query) == 0)
    {
        MYSQL_RES res;
        MYSQL_ROW row;
        res = *mysql_store_result(&mysql);
        row = mysql_fetch_row(&res);
        if (row != NULL)
        {
            char type[10], fromUser[32];
            strcpy(type, row[0]);
            strcpy(fromUser, row[1]);
            if (command.find("accept ") != string::npos)
            {
                if (strstr(type, "FRIEND") != NULL)
                {
                    addFriend(msg.fromUser, fromUser);
                    sprintf(query, "delete from %s_querybox where id = %d", msg.fromUser, queryID);
                    mysql_query(&mysql, query);
                }
                else if(strstr(type, "GROUP") != NULL)
                {
                    
                }
            }
            else if (command.find("refuse ") != string::npos)
            {
                sprintf(query, "delete from %s_querybox where id = %d", msg.fromUser, queryID);
                mysql_query(&mysql, query);
            }
        }
    }
}

void Server::addFriend(char *account, char *whichfriend)
{
    char query[1024];
    sprintf(query, "insert into %s_friendlist values ('%s', 0);", account, whichfriend);
    mysql_query(&mysql, query);
    sprintf(query, "insert into %s_friendlist values ('%s', 0);", whichfriend, account);
    mysql_query(&mysql, query);
    
}

void Server::deleteFriend(char *account, char *whichfriend) //删除好友
{
}
void Server::setSuki() //设为特别关心
{
}
void Server::setKirai() //拉黑名单
{
}
void Server::createFriendList() //注册完成时创建该用户的好友列表
{
    char query[4096];
    sprintf(query, "create table %s_friendslist like friendlist;", acc.account);
    if (mysql_query(&mysql, query) == 0)
    {
        cout << "New friend list is been created." << endl;
    }
}
void Server::sendFriendList() //发送好友列表（登录后马上调用，以获取该用户的好友列表）
{
}
/*服务端部分*/