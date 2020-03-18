#include "Client.h"
#include "Server.h"

extern char content[5120];
extern char query[5120];

/*客户端部分*/
void Client::queryFriendList() //请求好友列表
{
    setMsg(msg, QUERYFRIENDLIST, acc.account, nullptr, nullptr);
    if (sendMsg(msg, sock_fd) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
    else
    {
        cout << "正在请求好友列表..." << endl;
        fstream fl;
        fl.open("friendlist", ios::out);
        fl << "account flag" << endl;
        while (1)
        {
            recvMsg(sock_fd, msg);
            if (msg.type == LIST)
            {
                if (strcmp(msg.content, "") != 0)
                {
                    fl << msg.fromUser << " " << msg.content << endl;
                    friendlist[msg.fromUser] = atoi(msg.content);
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
                else
                {
                    break;
                }
            }
        }
        fl.close();
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
    mysql_query(&mysql, query);
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
    mysql_query(&mysql, query);
    MYSQL_RES res;
    MYSQL_ROW row;
    res = *mysql_store_result(&mysql);
    row = mysql_fetch_row(&res);
    if (row != NULL)
    {
        if (targetExisted(true) == false)
        {
            sprintf(content, "用户 %s 请求添加你为好友！", recv_msg.fromUser);
            sprintf(query, "insert into %s_querybox values (null, '%s', '%s', '%s', '%s');", recv_msg.toUser, "FRIEND", recv_msg.fromUser, recv_msg.toUser, content);
            mysql_query(&mysql, query);
            adminMsg(content, recv_msg.toUser);
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
    mysql_query(&mysql, query);
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
                mysql_query(&mysql, query);
            }
            else if (strEqual(type, "GROUP"))
            {
                addGroupMember(target, fromUser);
                sprintf(query, "delete from %s_querybox where id = %s", recv_msg.fromUser, recv_msg.content);
                mysql_query(&mysql, query);
            }
        }
        else if (recv_msg.type == REFUSE)
        {
            sprintf(query, "delete from %s_querybox where id = %s", recv_msg.fromUser, recv_msg.content);
            mysql_query(&mysql, query);
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
    mysql_query(&mysql, query);
    sprintf(query, "insert into %s_friendlist values ('%s', '0');", whichfriend, account);
    mysql_query(&mysql, query);
    sprintf(content, "你已经和 %s 是好友啦！请手动刷新好友列表（/queryfriendlist），和他打招呼吧！", whichfriend);
    adminMsg(content, account);
    sprintf(content, "你已经和 %s 是好友啦！请手动刷新好友列表（/queryfriendlist），和他打招呼吧！", account);
    adminMsg(content, whichfriend);
}

void Server::deleteFriend() //删除好友
{
    sprintf(query, "delete from %s_friendlist where account = '%s';", recv_msg.fromUser, recv_msg.content);
    mysql_query(&mysql, query);
    sprintf(query, "delete from %s_friendlist where account = '%s';", recv_msg.content, recv_msg.fromUser);
    mysql_query(&mysql, query);
}
void Server::setFriendFlag() //设为特别关心
{
    if (recv_msg.type == SUKI)
    {
        sprintf(query, "update %s_friendlist set flag = '2' where account = %s", recv_msg.fromUser, recv_msg.toUser);
    }
    else if (recv_msg.type == KIRAI)
    {
        sprintf(query, "update %s_friendlist set flag = '3' where account = %s", recv_msg.fromUser, recv_msg.toUser);
    }
    mysql_query(&mysql, query);
}
void Server::createFriendList() //注册完成时创建该用户的好友列表
{
    sprintf(query, "create table %s_friendlist like friendlist;", acc.account);
    mysql_query(&mysql, query);
    cout << "New friend list is been created." << endl;
}
void Server::createQuerybox() //注册完成时创建该用户的好友列表
{
    sprintf(query, "create table %s_querybox like querybox;", acc.account);
    mysql_query(&mysql, query);
    cout << "New querybox is been created." << endl;
}

void Server::sendFriendList(int call) //发送好友列表（登录后马上调用，以获取该用户的好友列表）
{
    sprintf(query, "select account, flag from %s_friendlist", recv_msg.fromUser);
    mysql_query(&mysql, query);
    MYSQL_RES *res;
    MYSQL_ROW row;
    res = mysql_store_result(&mysql);
    while (row = mysql_fetch_row(res))
    {
        if (row != NULL)
        {
            setMsg(send_msg, LIST, row[0], nullptr, row[1]);
            sendMsg(send_msg, call);
        }
        else
        {
            setMsg(send_msg, LIST, nullptr, nullptr, nullptr);
            sendMsg(send_msg, call);
            break;
        }
    }
}
/*服务端部分*/