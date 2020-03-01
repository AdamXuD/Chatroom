#include "Client.h"
#include "Server.h"

/*客户端部分*/
void Client::makeFriend(string command) //添加好友
{
    setMsg(msg, COMMAND, acc.account, command.substr(sizeof("makefriend ") - 1).c_str(), "MAKEFRIEND");
    if (sendMsg(msg, sock_fd) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
}
void Client::deleteFriend(string command) //删除好友
{
    setMsg(msg, COMMAND, acc.account, command.substr(sizeof("delete ") - 1).c_str(), "DELETEFRIEND");
    if (sendMsg(msg, sock_fd) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
}
void Client::queryFriendList() //请求好友列表
{
    setMsg(msg, COMMAND, acc.account, nullptr, "QUERYFRIENDLIST");
    if (sendMsg(msg, sock_fd) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
    else
    {
        cout << "正在请求好友列表..." << endl;
        char friendList[1536][32] = {0};
        fstream fl;
        fl.open("friendlist", ios::out);
        while (1)
        {
            sleep(1);
            recvMsg(sock_fd, (char *)friendList);
            if (strEqual(friendList[0], "account"))
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
    setMsg(msg, COMMAND, acc.account, command.substr(sizeof("suki ") - 1).c_str(), "SUKI");
    if (sendMsg(msg, sock_fd) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
}
void Client::setKirai(string command) //拉黑名单
{
    setMsg(msg, COMMAND, acc.account, command.substr(sizeof("kirai ") - 1).c_str(), "KIRAI");
    if (sendMsg(msg, sock_fd) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
}
void Client::dealwithQuery(string command)
{
    setMsg(msg, COMMAND, acc.account, nullptr, command.c_str());
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
            char content[4096], toUser[32];
            strcpy(toUser, msg.toUser);
            sprintf(content, "用户 %s 请求添加你为好友！", msg.fromUser);
            sprintf(query, "insert into %s_querybox values (null, '%s', '%s', '%s', '%s');", msg.toUser, "FRIEND", msg.fromUser, msg.toUser, content);
            setMsg(msg, PRIVTALK, ADMIN, toUser, content);
            if (mysql_query(&mysql, query) == 0)
            {
                Privatetalk(msg);
            }
            else
            {
                cout << mysql_error(&mysql) << endl;
            }
        }
        else
        {
            char fromUser[32];
            strcpy(fromUser, msg.fromUser);
            setMsg(msg, PRIVTALK, ADMIN, fromUser, "好友添加请求失败，请确认该用户是否存在！");
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
    sprintf(query, "select type, fromuser, target from %s_querybox where id = %d;", msg.fromUser, queryID);
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
            strcpy(toUser, row[2]);
            if (strEqual(command, "accept "))
            {
                if (strEqual(type, "FRIEND"))
                {
                    if (addFriend(toUser, fromUser) == 0)
                    {
                        sprintf(query, "delete from %s_querybox where id = %d", msg.fromUser, queryID);
                        mysql_query(&mysql, query);
                    }
                }
                else if (strEqual(type, "GROUP"))
                {
                    if(addGroupMember(toUser, fromUser) == 0)
                    {
                        sprintf(query, "delete from %s_querybox where id = %d", msg.fromUser, queryID);
                        mysql_query(&mysql, query);
                    }
                }
            }
            else if (strEqual(command, "refuse "))
            {
                sprintf(query, "delete from %s_querybox where id = %d", msg.fromUser, queryID);
                mysql_query(&mysql, query);
            }
        }
    }
}

int Server::addFriend(char *account, char *whichfriend)
{
    char query[1024], content[4096];
    sprintf(query, "insert into %s_friendlist values ('%s', '0');", account, whichfriend);
    if (mysql_query(&mysql, query) != 0)
    {
        cout << "Query error: " << mysql_error(&mysql) << endl;
        setMsg(msg, PRIVTALK, ADMIN, account, "请求失败，请稍后再试！");
        Privatetalk(msg);
        return -1;
    }
    sprintf(query, "insert into %s_friendlist values ('%s', '0');", whichfriend, account);
    if (mysql_query(&mysql, query) != 0)
    {
        cout << "Query error: " << mysql_error(&mysql) << endl;
        sprintf(query, "delete from %s_friendlist where account = '%s'", account, whichfriend);
        mysql_query(&mysql, query);
        setMsg(msg, PRIVTALK, ADMIN, account, "请求失败，请稍后再试！");
        Privatetalk(msg);
        return -1;
    }
    Msg msg;
    sprintf(content, "你已经和 %s 是好友啦！请手动刷新好友列表（/queryfriendlist），和他打招呼吧！", whichfriend);
    setMsg(msg, PRIVTALK, ADMIN, account, content);
    Privatetalk(msg);
    sprintf(content, "你已经和 %s 是好友啦！请手动刷新好友列表（/queryfriendlist），和他打招呼吧！", account);
    setMsg(msg, PRIVTALK, ADMIN, whichfriend, content);
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
    sprintf(query, "update %s_friendlist set flag = '2' where account = %s", msg.fromUser, msg.toUser);
    mysql_query(&mysql, query);
}
void Server::setKirai() //拉黑名单
{
    char query[4096];
    sprintf(query, "update %s_friendlist set flag = '3' where account = %s", msg.fromUser, msg.toUser);
    mysql_query(&mysql, query);
}
void Server::createFriendList(char *account) //注册完成时创建该用户的好友列表
{
    char query[4096];
    sprintf(query, "create table %s_friendlist like friendlist;", account);
    if (mysql_query(&mysql, query) == 0)
    {
        cout << "New friend list is been created." << endl;
    }
}
void Server::createQuerybox(char *account) //注册完成时创建该用户的好友列表
{
    char query[4096];
    sprintf(query, "create table %s_querybox like querybox;", account);
    if (mysql_query(&mysql, query) == 0)
    {
        cout << "New querybox is been created." << endl;
    }
}

void Server::sendFriendList(int call) //发送好友列表（登录后马上调用，以获取该用户的好友列表）
{
    char query[1024], friendList[1536][32] = {0};
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
    sendMsg((char *)friendList, call);
}
/*服务端部分*/