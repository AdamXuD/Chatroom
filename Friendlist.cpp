#include "Client.h"
#include "Server.h"

extern char content[5120];
extern char query[10240];

/*客户端部分*/
void Client::queryFriendList(bool show) //请求好友列表
{
    if (show)
    {
        cout << "正在请求好友列表..." << endl;
    }
    sleep(1);
    setMsg(msg, QUERYFRIENDLIST, acc.account, nullptr, nullptr);
    if (sendMsg(msg, pipe_fd[1]) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
    else
    {
        friendlist.clear();
        fstream fl;
        fl.open("friendlist", ios::out);
        fl << "account flag" << endl;
        while (1)
        {
            recvMsg(listpipe_fd[0], msg, true);
            if (msg.type == LIST)
            {
                fl << msg.fromUser << " " << msg.content << endl;
                friendlist[msg.fromUser] = atoi(msg.content);
                if (show)
                {
                    cout << msg.fromUser << " ";
                    switch (atoi(msg.content))
                    {
                    case 0:
                    {
                        cout << "[好友]" << endl;
                        break;
                    }
                    case 1:
                    {
                        cout << "[群组]" << endl;
                        break;
                    }
                    case 2:
                    {
                        cout << "[特别关心]" << endl;
                        break;
                    }
                    case 3:
                    {
                        cout << "[黑名单]" << endl;
                        break;
                    }
                    default:
                    {
                        cout << "[ERROR]" << endl;
                        break;
                    }
                    }
                }
            }
            else if (msg.type == EOF)
            {
                break;
            }
        }
        if (show)
        {
            cout << "请求完毕！" << endl;
        }
        fl.close();
    }
}

void Client::getOnlineFriends()
{
    cout << "正在请求在线列表..." << endl;
    sleep(1);
    setMsg(msg, ONLINELIST, acc.account, nullptr, nullptr);
    if (sendMsg(msg, pipe_fd[1]) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
    else
    {
        cout << "在线好友如下：" << endl;
        while (1)
        {
            recvMsg(listpipe_fd[0], msg, true);
            if (msg.type == LIST)
            {
                cout << msg.fromUser << endl;
            }
            else if (msg.type == EOF)
            {
                break;
            }
        }
        cout << "请求完毕！" << endl;
    }
}
/*客户端部分*/

/*服务端部分*/
bool Server::targetExisted(bool isFriend)
{
    if (isFriend)
    {
        sprintf(query, "select account from %s_friendlist where account='%s'", recv_msg.fromUser, recv_msg.content);
    }
    else
    {
        sprintf(query, "select account from group_%s where account='%s'", recv_msg.content, recv_msg.fromUser);
    }
    Mysql_query(&mysql, query);
    MYSQL_RES res;
    MYSQL_ROW row;
    res = *mysql_store_result(&mysql);
    row = mysql_fetch_row(&res);
    if (row != NULL)
    {
        return true;
    }
    else
    {
        return false;
    }
}
void Server::makeFriendQuery() //添加好友
{
    sprintf(query, "select account from userinfo where account='%s';", recv_msg.content);
    Mysql_query(&mysql, query);
    MYSQL_RES res;
    MYSQL_ROW row;
    res = *mysql_store_result(&mysql);
    row = mysql_fetch_row(&res);
    if (row != NULL)
    {
        if (targetExisted(true) == false)
        {
            sprintf(content, "用户 %s 请求添加你为好友！", recv_msg.fromUser);
            sprintf(query, "insert into %s_querybox values (null, '%s', '%s', '%s', '%s');", recv_msg.content, "FRIEND", recv_msg.fromUser, recv_msg.content, content);
            Mysql_query(&mysql, query);
            adminMsg(content, recv_msg.content, true);
        }
        else
        {
            adminMsg("该用户已是您的好友，无需重复添加！", recv_msg.fromUser);
        }
    }
    else
    {
        adminMsg("好友添加请求失败，请确认该用户是否存在！", recv_msg.fromUser);
    }
}

void Server::dealwithQuery()
{
    sprintf(query, "select type, fromuser, target from %s_querybox where id = %s;", recv_msg.fromUser, recv_msg.content);
    Mysql_query(&mysql, query);
    MYSQL_RES res;
    MYSQL_ROW row;
    res = *mysql_store_result(&mysql);
    row = mysql_fetch_row(&res);
    if (row != NULL)
    {
        char type[10], fromUser[32], target[32];
        strcpy(type, row[0]);
        strcpy(fromUser, row[1]);
        strcpy(target, row[2]);
        if (recv_msg.type == ACCEPT)
        {
            if (strEqual(type, "FRIEND"))
            {
                addFriend(target, fromUser);
                sprintf(query, "delete from %s_querybox where id = %s", recv_msg.fromUser, recv_msg.content);
                Mysql_query(&mysql, query);
            }
            else if (strEqual(type, "GROUP"))
            {
                addGroupMember(target, fromUser);
                sprintf(query, "delete from %s_querybox where id = %s", recv_msg.fromUser, recv_msg.content);
                Mysql_query(&mysql, query);
            }
        }
        else if (recv_msg.type == REFUSE)
        {
            sprintf(query, "delete from %s_querybox where id = %s", recv_msg.fromUser, recv_msg.content);
            Mysql_query(&mysql, query);
        }
    }
    else
    {
        adminMsg("请求失败，请检查请求ID是否正确！", recv_msg.fromUser);
    }
}

void Server::addFriend(char *account, char *whichfriend)
{
    sprintf(query, "insert into %s_friendlist values ('%s', '0');", account, whichfriend);
    Mysql_query(&mysql, query);
    sprintf(query, "insert into %s_friendlist values ('%s', '0');", whichfriend, account);
    Mysql_query(&mysql, query);
    sprintf(content, "你已经和 %s 是好友啦！请手动刷新好友列表（/queryfriendlist），和他打招呼吧！", whichfriend);
    adminMsg(content, account);
    sprintf(content, "你已经和 %s 是好友啦！请手动刷新好友列表（/queryfriendlist），和他打招呼吧！", account);
    adminMsg(content, whichfriend);
}

void Server::deleteFriend() //删除好友
{
    sprintf(query, "delete from %s_friendlist where account = '%s';", recv_msg.fromUser, recv_msg.content);
    Mysql_query(&mysql, query);
    sprintf(query, "delete from %s_friendlist where account = '%s';", recv_msg.content, recv_msg.fromUser);
    Mysql_query(&mysql, query);
}
void Server::setFriendFlag() //设为特别关心
{
    if (recv_msg.type == SUKI)
    {
        sprintf(query, "update %s_friendlist set flag = '2' where account = '%s'", recv_msg.fromUser, recv_msg.content);
    }
    else if (recv_msg.type == KIRAI)
    {
        sprintf(query, "update %s_friendlist set flag = '3' where account = '%s'", recv_msg.fromUser, recv_msg.content);
    }
    Mysql_query(&mysql, query);
}
void Server::createFriendList() //注册完成时创建该用户的好友列表
{
    sprintf(query, "create table %s_friendlist like friendlist;", acc.account);
    Mysql_query(&mysql, query);
    cout << "New friend list is been created." << endl;
}
void Server::createQuerybox() //注册完成时创建该用户的好友列表
{
    sprintf(query, "create table %s_querybox like querybox;", acc.account);
    Mysql_query(&mysql, query);
    cout << "New querybox is been created." << endl;
}

void Server::sendFriendList(int call) //发送好友列表（登录后马上调用，以获取该用户的好友列表）
{
    sprintf(query, "select account, flag from %s_friendlist", recv_msg.fromUser);
    Mysql_query(&mysql, query);
    MYSQL_RES *res;
    MYSQL_ROW row;
    res = mysql_store_result(&mysql);
    while (row = mysql_fetch_row(res))
    {
        usleep(30000);
        if (row != NULL)
        {
            setMsg(send_msg, LIST, row[0], nullptr, row[1]);
            sendMsg(send_msg, call, true);
        }
    }
    usleep(30000);
    setMsg(send_msg, EOF, nullptr, nullptr, "EOF");
    sendMsg(send_msg, call, true);
}

void Server::sendOnlineFriends(int call)
{
    string friendlist[1536];
    int i = 0;
    sprintf(query, "select account, flag from %s_friendlist", recv_msg.fromUser);
    Mysql_query(&mysql, query);
    MYSQL_RES *res;
    MYSQL_ROW row;
    res = mysql_store_result(&mysql);
    while (row = mysql_fetch_row(res))
    {
        if (row != NULL)
        {
            if (strEqual(row[1], "0"))
            {
                friendlist[i] = row[0];
                i++;
            }
        }
    }
    map<int, pair<string, int>>::iterator it;
    for (it = onlinelist.begin(); it != onlinelist.end(); it++)
    {
        for (int j = 0; j < i; j++)
        {
            if (strEqual(it->second.first, friendlist[j].c_str()))
            {
                usleep(30000);
                setMsg(send_msg, LIST, it->second.first.c_str(), nullptr, nullptr);
                sendMsg(send_msg, call, true);
            }
        }
    }
    usleep(30000);
    setMsg(send_msg, EOF, nullptr, nullptr, "EOF");
    sendMsg(send_msg, call, true);
}
/*服务端部分*/