#include "Client.h"
#include "Server.h"

/*客户端部分*/
void Client::makeFriend(string command) //添加好友
{
    memset(&msg, 0, sizeof(msg));
    msg.type = COMMAND;
    strcpy(msg.toUser, command.substr(sizeof("makefriend ") - 1).c_str());
    strcpy(msg.fromUser, acc.account);
    strcpy(msg.content, "MAKEFRIEND");
    if (sendMsg(msg, sock_fd) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
}
void Client::deleteFriend(string command) //删除好友
{
    memset(&msg, 0, sizeof(msg));
    msg.type = COMMAND;
    strcpy(msg.toUser, command.substr(sizeof("delete ") - 1).c_str());
    strcpy(msg.fromUser, acc.account);
    strcpy(msg.content, "DELETEFRIEND");
    if (sendMsg(msg, sock_fd) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
}
void Client::queryFriendList() //请求好友列表
{
    msg.type = COMMAND;
    strcpy(msg.fromUser, acc.account);
    strcpy(msg.content, "QUERYFRIENDLIST");
    if (sendMsg(msg, sock_fd) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
    else
    {
        cout << "正在请求好友列表..." << endl;
        char buf[65535] = {0};
        char friendList[1536][32] = {0};
        fstream fl;
        fl.open("friendlist", ios::out);
        while (1)
        {
            sleep(1);
            recv(sock_fd, buf, sizeof(buf), 0);
            memcpy(friendList, buf, sizeof(buf));
            if (strstr(friendList[0], "account") != NULL)
            {
                fl << "account ";
                fl << "flag" << endl;
                for (int i = 2; friendList[i] != "\0"; i += 2)
                {
                    friendlist[friendList[i]] = atoi(friendList[i + 1]);
                    fl << friendList[i] << " ";
                    fl << friendList[i + 1] << endl;
                }
                break;
            }
        }
        fl.close();
    }
}
void Client::setSuki(string command) //设为特别关心
{
    memset(&msg, 0, sizeof(msg));
    msg.type = COMMAND;
    strcpy(msg.fromUser, acc.account);
    strcpy(msg.toUser, command.substr(sizeof("suki ") - 1).c_str());
    strcpy(msg.content, "SUKI");
    if (sendMsg(msg, sock_fd) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
}
void Client::setKirai(string command) //拉黑名单
{
    memset(&msg, 0, sizeof(msg));
    msg.type = COMMAND;
    strcpy(msg.fromUser, acc.account);
    strcpy(msg.toUser, command.substr(sizeof("kirai ") - 1).c_str());
    strcpy(msg.content, "KIRAI");
    if (sendMsg(msg, sock_fd) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
}
void Client::dealwithQuery(string command)
{
    msg.type = COMMAND;
    strcpy(msg.fromUser, acc.account);
    strcpy(msg.content, command.c_str());
    if (sendMsg(msg, sock_fd) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
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
            sprintf(tmp.content, "用户 %s 请求添加你为好友！", msg.fromUser);
            sprintf(query, "insert into %s_querybox values (null, '%s', '%s', '%s', '%s');", msg.toUser, "FRIEND", msg.fromUser, msg.toUser, tmp.content);
            if (mysql_query(&mysql, query) == 0)
            {
                Privatetalk(tmp);
            }
            else
            {
                cout << mysql_error(&mysql) << endl;
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
    sprintf(query, "select type, fromuser, toUser from %s_querybox where id = %d;", msg.fromUser, queryID);
    if (mysql_query(&mysql, query) == 0)
    {
        MYSQL_RES res;
        MYSQL_ROW row;
        res = *mysql_store_result(&mysql);
        row = mysql_fetch_row(&res);
        if (row != NULL)
        {
            char type[10], fromUser[32], toUser[32];
            strcpy(type, row[0]);
            strcpy(fromUser, row[1]);
            strcpy(fromUser, row[2]);
            if (command.find("accept ") != string::npos)
            {
                if (strstr(type, "FRIEND") != NULL)
                {
                    if (addFriend(msg.fromUser, fromUser) == 0)
                    {
                        sprintf(query, "delete from %s_querybox where id = %d", msg.fromUser, queryID);
                        mysql_query(&mysql, query);
                    }
                }
                else if (strstr(type, "GROUP") != NULL)
                {
                    if(addGroupMember(toUser, fromUser) == 0)
                    {
                        sprintf(query, "delete from %s_querybox where id = %d", msg.fromUser, queryID);
                        mysql_query(&mysql, query);
                    }
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

int Server::addFriend(char *account, char *whichfriend)
{
    char query[1024];
    sprintf(query, "insert into %s_friendlist values ('%s', 0);", account, whichfriend);
    if (mysql_query(&mysql, query) != 0)
    {
        cout << "Query error: " << mysql_error(&mysql) << endl;
        msg.type = PRIVTALK;
        strcpy(msg.fromUser, "Admin");
        strcpy(msg.toUser, account);
        strcpy(msg.content, "请求失败，请稍后再试！");
        Privatetalk(msg);
        return -1;
    }
    sprintf(query, "insert into %s_friendlist values ('%s', 0);", whichfriend, account);
    if (mysql_query(&mysql, query) != 0)
    {
        cout << "Query error: " << mysql_error(&mysql) << endl;
        sprintf(query, "delete from %s_friendlist where account = '%s'", account, whichfriend);
        mysql_query(&mysql, query);
        msg.type = PRIVTALK;
        strcpy(msg.fromUser, "Admin");
        strcpy(msg.toUser, account);
        strcpy(msg.content, "请求失败，请稍后再试！");
        Privatetalk(msg);
        return -1;
    }
    msg.type = PRIVTALK;
    strcpy(msg.fromUser, "Admin");
    sprintf(msg.content, "你已经和 %s 是好友啦！请手动刷新好友列表（/queryfriendlist），和他打招呼吧！", whichfriend);
    strcpy(msg.toUser, account);
    Privatetalk(msg);
    sprintf(msg.content, "你已经和 %s 是好友啦！请手动刷新好友列表（/queryfriendlist），和他打招呼吧！", account);
    strcpy(msg.toUser, whichfriend);
    Privatetalk(msg);
    return 0;
}

void Server::deleteFriend(char *account, char *whichfriend) //删除好友
{
    char query[1024];
    sprintf(query, "delete from %s_friendlist where account = '%s';", account, whichfriend);
    mysql_query(&mysql, query);
    sprintf(query, "delete from %s_friendlist where account = '%s';", whichfriend, account);
    mysql_query(&mysql, query);
}
void Server::setSuki() //设为特别关心
{
    char query[4096];
    sprintf(query, "update %s_friendlist set flag = 2 where account = %s", msg.fromUser, msg.toUser);
    mysql_query(&mysql, query);
}
void Server::setKirai() //拉黑名单
{
    char query[4096];
    sprintf(query, "update %s_friendlist set flag = 3 where account = %s", msg.fromUser, msg.toUser);
    mysql_query(&mysql, query);
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
void Server::createQuerybox() //注册完成时创建该用户的好友列表
{
    char query[4096];
    sprintf(query, "create table %s_querybox like querybox;", acc.account);
    if (mysql_query(&mysql, query) == 0)
    {
        cout << "New querybox is been created." << endl;
    }
}

void Server::sendFriendList(int call) //发送好友列表（登录后马上调用，以获取该用户的好友列表）
{
    char query[1024], buf[65535], friendList[1536][32] = {0};
    strcpy(friendList[0], "account");
    strcpy(friendList[1], "flag");
    sprintf(query, "select account, flag from %s_friendlist", msg.fromUser);
    if (mysql_query(&mysql, query) == 0)
    {
        MYSQL_RES *res;
        MYSQL_ROW row;
        res = mysql_store_result(&mysql);
        if (res != NULL)
        {
            for (int i = 2; row = mysql_fetch_row(res); i += 2)
            {
                strcpy(friendList[i], row[0]);
                strcpy(friendList[i + 1], row[1]);
            }
        }
        else
        {
            cout << "Query friendlist error:" << mysql_error(&mysql) << endl;
        }
    }
    else
    {
        cout << "Query friendlist error:" << mysql_error(&mysql) << endl;
    }
    memset(buf, 0, sizeof(buf));
    memcpy(buf, friendList, sizeof(friendList));
    send(call, buf, sizeof(buf), 0);
}
/*服务端部分*/