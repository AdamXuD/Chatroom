#include "Client.h"
#include "Server.h"

/*客户端部分*/
void Client::makeFriend() //添加好友
{
    input(command, "请输入需要添加的好友昵称>");
    setMsg(msg, MAKEFRIEND, acc.account, nullptr, command.substr(sizeof("makefriend ") - 1).c_str());
    sendMsg(msg, pipe_fd[1]);
    cout << "请求已发送！" << endl;
}
void Client::deleteFriend() //删除好友
{
    input(command, "请输入需要删除的好友昵称>");
    setMsg(msg, DELETEFRIEND, acc.account, nullptr, command.c_str());
    sendMsg(msg, pipe_fd[1]);
    cout << "请求已发送！" << endl;
}
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
void Client::setSuki() //设为特别关心
{
    input(command, "请输入特别关心的好友昵称>");
    setMsg(msg, SUKI, acc.account, nullptr, command.substr(sizeof("suki ") - 1).c_str());
    sendMsg(msg, pipe_fd[1]);
    cout << "请求已发送！" << endl;
}
void Client::setKirai() //拉黑名单
{
    input(command, "请输入拉入黑名单的好友昵称>");
    setMsg(msg,  KIRAI, acc.account, nullptr, command.substr(sizeof("kirai ") - 1).c_str());
    sendMsg(msg, pipe_fd[1]);
    cout << "请求已发送！" << endl;
}
int Client::getQueryBox(bool show) //show表示是否回显内容
{
    if (show)
    {
        cout << "正在请求待处理请求列表..." << endl;
    }
    sleep(1); //象征性停一秒
    setMsg(msg, QUERYBOX, acc.account, nullptr, nullptr); //发送请求
    if (sendMsg(msg, pipe_fd[1]) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
    else
    {
        Querybox.clear(); //清空已有map容器
        while (1)
        {
            recvMsg(listpipe_fd[0], msg, true); //循环接收服务器消息
            if (msg.type == LIST) //要是服务器消息类型为LIST
            {
                Querybox[atoi(msg.fromUser)] = msg.content; //存储
                if (show)
                {
                    cout << "id：" << msg.fromUser << endl;
                    cout << "内容：" << msg.content << endl;
                    cout << endl;
                }
            }
            else if (msg.type == EOF) //类型为EOF的话，表示传输结束
            {
                break; //跳出循环
            }
        }
        if (show)
        {
            cout << "请求完毕！" << endl;
        }
    }
}
/*客户端部分*/

/*服务端部分*/
void Server::makeFriendQuery();
{
    char qyery[1024];
    sprintf(query, "select account from userinfo where account='%s';", recv_msg.content);
    Mysql_query(&mysql, query);
    MYSQL_RES res;
    MYSQL_ROW row;
    res = *mysql_store_result(&mysql);
    row = mysql_fetch_row(&res);
        if (row != NULL) /*判断该用户是否存在*/
         {
            if (targetExisted(true) == false)
                {
                    sprintf(content, "用户 %s 请求添加你为好友！", recv_msg.fromUser);
                    sprintf(query, "insert into %s_querybox values (null, '%s', '%s', '%s', '%s');", recv_msg.content, "FRIEND", recv_msg.fromUser, recv_msg.content, content);
                    Mysql_query(&mysql, query); //将请求置入目标用户请求列表中
                    adminMsg(content, recv_msg.content, true); //提示目标用户有人添加好友
                }
            else
                {
                    adminMsg("该用户已是您的好友，无需重复添加！", recv_msg.fromUser);
                    //提示来源用户不能重复添加好友
                }
         }
        else
        {
            adminMsg("好友添加请求失败，请确认该用户是否存在！", recv_msg.fromUser);
                //提示来源用户目标好友不存在
        }
}
void Server::addFriend(char *account, char *whichfriend)
{
    sprintf(query, "insert into %s_friendlist values ('%s', '0');", account, whichfriend);
    Mysql_query(&mysql, query);
    sprintf(query, "insert into %s_friendlist values ('%s', '0');", whichfriend, account);
    Mysql_query(&mysql, query);
    sprintf(content, "你已经和 %s 是好友啦！请手动刷新好友列表（/queryfriendlist），和他打招呼吧！", whichfriend);
    adminMsg(content, account); //发送提示
    sprintf(content, "你已经和 %s 是好友啦！请手动刷新好友列表（/queryfriendlist），和他打招呼吧！", account); //发送提示
    adminMsg(content, whichfriend);
}
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
void Server::adminMsg(const char *content, char *target, bool query)
{
    if (query)
    {
        setMsg(send_msg, QUERY, ADMIN, target, content);
    }
    else
    {
        setMsg(send_msg, PRIVTALK, ADMIN, target, content);
    }
    Privatetalk(send_msg);
}
void Server::sendQueryBox(int call)
{
    sprintf(query, "select id, content from `%s_querybox`", recv_msg.fromUser);
    Mysql_query(&mysql, query);
    MYSQL_RES *res;
    MYSQL_ROW row;
    res = mysql_store_result(&mysql); //获取请求用户的消息列表
    while (row = mysql_fetch_row(res)) //只要结果集不为空，循环获取下一条结果集
    {
        usleep(30000); //象征性停三十毫秒
        if (row != NULL) //结果集不为空的话
        {
            setMsg(send_msg, LIST, row[0], nullptr, row[1]); //将请求id和请求内容发送给客户端
            sendMsg(send_msg, call, true);
        }
    }
    usleep(30000); //结束后也要象征性停三十毫秒
    setMsg(send_msg, EOF, nullptr, nullptr, "EOF"); //向客户端发送结束标志
    sendMsg(send_msg, call, true);
}
void Server::deleteFriend() //删除好友
{
    cout << "A user is trying to delete friend." << endl;
    char query[1024];
    sprintf(query, "delete from %s_friendlist where account = '%s';", recv_msg.fromUser, recv_msg.content);
    Mysql_query(&mysql, query);
    sprintf(query, "delete from %s_friendlist where account = '%s';", recv_msg.content, recv_msg.fromUser);
    Mysql_query(&mysql, query);
}
void Server::setFriendFlag() //设为特别关心或黑名单
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
void Server::createQuerybox() //注册完成时创建该用户的请求列表
{
    char query[1024];
    sprintf(query, "create table %s_querybox like querybox;", acc.account);
    if (mysql_query(&mysql, query) == 0) {
        cout << "Database created list successfully." << endl;
    } else
        cout << "Database created list abortively." << endl;

}
void Server::createFriendList() //注册完成时创建该用户的好友列表
{   char query[1024];
    sprintf(query, "create table %s_querybox like friendlist;", acc.account);
    if (mysql_query(&mysql, query) == 0) {
        cout << "Database created list successfully." << endl;
    } else
        cout << "Database created list abortively." << endl;
}
void Server::sendFriendList(int call) //发送好友列表（登录后马上调用，以获取该用户的好友列表）
{
    char query[1024];
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
/*服务端部分*/