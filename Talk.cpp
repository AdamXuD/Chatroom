#include "Server.h"
#include "Client.h"

extern char content[5120];
extern char query[5120];

/*客户端部分*/
/*群聊相关权限部分*/
void Client::queryGroupMember(char *Group)
{
    setMsg(msg, QUERYMEMBER, acc.account, Group, nullptr);
    if (sendMsg(msg, sock_fd) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
    else
    {
        cout << "正在请求群员列表..." << endl;
        fstream ml;
        ml.open(Group, ios::out);
        ml << "account flag" << endl;
        while (1)
        {
            recvMsg(sock_fd, msg);
            if (msg.type == LIST)
            {
                if (strcmp(msg.content, "") != 0)
                {
                    ml << msg.fromUser << " " << msg.content << endl;
                    cout << msg.fromUser << " ";
                    switch (atoi(msg.content))
                    {
                    case 0:
                    {
                        cout << "[群员]" << endl;
                        break;
                    }
                    case 1:
                    {
                        cout << "[管理员]" << endl;
                        break;
                    }
                    case 2:
                    {
                        cout << "[群主]" << endl;
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
        ml.close();
    }
}
void Client::Grouptalk(string command) //处理用户群聊请求
{
    char toGroup[32];
    strcpy(toGroup, command.substr(sizeof("group ") - 1).c_str());
    cout << "你正在 " << toGroup << " 群内发言：" << endl;
    setMsg(msg, PIPE, nullptr, toGroup, nullptr);
    sendMsg(msg, pipe_fd[1]);
    while (1)
    {
        string command;
        getline(cin, command, '\n');
        if (*command.begin() == '/') //指令语意处理
        {
            command.erase(0, 1); //吃掉一个斜杠
            if (strEqual(command, "querymember"))
            {
                queryGroupMember(toGroup);
            }
            else
            {
                if (strEqual(command, "admin "))
                {
                    setMsg(msg, SETADMIN, acc.account, toGroup, command.substr(sizeof("admin ") - 1).c_str());
                }
                else if (strEqual(command, "leave"))
                {
                    setMsg(msg, LEAVEGROUP, acc.account, toGroup, nullptr);
                }
                else if (strEqual(command, "kickoff "))
                {
                    setMsg(msg, KICKOFFMEMBER, acc.account, toGroup, command.substr(sizeof("kickoff ") - 1).c_str());
                }
                else
                {
                    cout << "指令有误，请检查指令格式！" << endl;
                }
                if (sendMsg(msg, sock_fd) < 0)
                {
                    cout << "请求失败，请检查网络连接！" << endl;
                }
            }
        }
        else
        {
            setMsg(msg, GROUPTALK, acc.account, toGroup, command.c_str());
            sendMsg(msg, pipe_fd[1]);
        }
        if (command == "_exit")
        {
            setMsg(msg, PIPE, nullptr, "\0", nullptr);
            sendMsg(msg, pipe_fd[1]);
            break;
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
    setMsg(msg, PIPE, nullptr, toUser.c_str(), nullptr);
    sendMsg(msg, pipe_fd[1]);
    while (1)
    {
        string tmp;
        getline(cin, tmp, '\n');
        if (tmp == "_exit")
        {
            setMsg(msg, PIPE, nullptr, "\0", nullptr);
            sendMsg(msg, pipe_fd[1]);
            break;
        }
        else
        {
            setMsg(msg, PRIVTALK, acc.account, toUser.c_str(), tmp.c_str());
            sendMsg(msg, pipe_fd[1]);
        }
    }
}
/*私聊部分*/
/*客户端部分*/

/*服务端部分*/
/*群聊及其权限部分*/
int Server::groupPermission(char *Group, char *Member)
{
    sprintf(query, "select flag from group_%s where account = '%s';", Group, Member);
    mysql_query(&mysql, query);
    MYSQL_RES res;
    MYSQL_ROW row;
    res = *mysql_store_result(&mysql);
    row = mysql_fetch_row(&res);
    if (row != NULL)
    {
        char tmp[10] = {0};
        strcpy(tmp, row[0]);
        return atoi(tmp);
    }
}
void Server::SendGroupMember(int call)
{
    sprintf(query, "select account, flag from group_%s", recv_msg.toUser);
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
void Server::sendAdminMsg(Msg message, bool sw_query, char *fromUser, char *Group)
{
    char memberlist[1536][32] = {0};
    sprintf(query, "select account from group_%s where flag = '1' or flag = '2';", recv_msg.content);
    if (mysql_query(&mysql, query) == 0)
    {
        MYSQL_RES *res;
        MYSQL_ROW row;
        res = mysql_store_result(&mysql);
        if (res != NULL)
        {
            for (int i = 0; row = mysql_fetch_row(res); i++)
            {
                strcpy(memberlist[i], row[0]);
            }
        }
    }
    map<int, pair<string, int>>::iterator it;
    for (it = onlinelist.begin(); it != onlinelist.end(); it++)
    {
        for (int i = 0; strcmp(memberlist[i], "") != 0; i++)
        {
            if (strEqual(it->second.first.c_str(), memberlist[i]))
            {
                sendMsg(message, it->first);
            }
        }
    }
    if (sw_query)
    {
        if (fromUser != nullptr && Group != nullptr)
        {
            for (int i = 0; strcmp(memberlist[i], "") != 0; i++)
            {
                sprintf(query, "insert into %s_querybox values (null, '%s', '%s', '%s', '%s');", memberlist[i], "GROUP", fromUser, Group, message.content);
                mysql_query(&mysql, query);
            }
        }
    }
}
void Server::createGroupTalk() //创建群聊
{
    sprintf(query, "insert into groupinfo values ('%s')", recv_msg.content);
    if (mysql_query(&mysql, query) != 0)
    {
        adminMsg("创建群组失败，可能是同名群组已存在！", recv_msg.fromUser);
        return;
    }
    sprintf(query, "create table group_%s like group;", recv_msg.content);
    mysql_query(&mysql, query);
    sprintf(query, "insert into %s_friendlist values ('%s', 1);", recv_msg.fromUser, recv_msg.content);
    mysql_query(&mysql, query);
    adminMsg("群组创建成功，请手动刷新好友列表（/queryfriendlist）！", recv_msg.fromUser);
}
void Server::addGroupMember(char *Group, char *Member)
{
    sprintf(query, "insert into group_%s values ('%s', '0');", Group, Member);
    mysql_query(&mysql, query);
    sprintf(query, "insert into %s_friendlist values ('%s', 1);", Member, Group);
    mysql_query(&mysql, query);
    sprintf(content, "%s 加入群聊 %s 啦！", Member, Group);
    setMsg(send_msg, GROUPTALK, ADMIN, Group, content);
    Grouptalk(send_msg);
}
void Server::setGroupAdmin() //设置管理员
{
    char fromM[32], toM[32], Group[32];
    strcpy(fromM, recv_msg.fromUser);
    strcpy(toM, recv_msg.content);
    strcpy(Group, recv_msg.toUser);
    if (groupPermission(Group, fromM) > groupPermission(Group, toM))
    {
        sprintf(query, "update Group_%s set flag = '1' where account = '%s'", Group, toM);
        mysql_query(&mysql, query);
        sprintf(content, "%s 已经被设置成管理员。", toM);
        setMsg(send_msg, GROUPTALK, ADMIN, fromM, content);
        Grouptalk(send_msg);
    }
    else
    {
        adminMsg("管理员设置请求失败，权限不足！", fromM);
    }
}
void Server::joinGroupQuery() //处理加入群聊请求
{
    sprintf(query, "select group from groupinfo where group='%s';", recv_msg.content);
    if (mysql_query(&mysql, query) == 0)
    {
        MYSQL_RES res;
        MYSQL_ROW row;
        res = *mysql_store_result(&mysql);
        row = mysql_fetch_row(&res);
        if (row != NULL)
        {
            if (targetExisted(false) == 0)
            {
                sprintf(send_msg.content, "用户 %s 请求加入 %s 群聊！", recv_msg.fromUser, recv_msg.content);
                setMsg(send_msg, PRIVTALK, ADMIN, nullptr, content);
                sendAdminMsg(send_msg, true, recv_msg.fromUser, recv_msg.content);
            }
            else
            {
                adminMsg("你已加入该群，无需重复添加！", recv_msg.fromUser);
            }
        }
        else
        {
            adminMsg("群组添加请求失败，请确认该用户是否存在！", recv_msg.fromUser);
        }
    }
}
void Server::leaveGroup() //处理离开群聊请求
{
    sprintf(query, "delete from group_%s where account = '%s'", recv_msg.toUser, recv_msg.fromUser);
    mysql_query(&mysql, query);
    sprintf(query, "delete from %s_friendlist where account = '%s'", recv_msg.fromUser, recv_msg.toUser);
    mysql_query(&mysql, query);
    sprintf(content, "%s 已离开 %s 群聊！", recv_msg.fromUser, recv_msg.toUser);
    setMsg(send_msg, GROUPTALK, ADMIN, recv_msg.toUser, content);
    sendAdminMsg(send_msg, false, recv_msg.fromUser);
}
void Server::deleteGroupMember() //处理踢出群聊请求
{
    char fromM[32], toM[32], Group[32];
    strcpy(fromM, recv_msg.fromUser);
    strcpy(toM, recv_msg.content);
    strcpy(Group, recv_msg.toUser);
    if (groupPermission(Group, fromM) > groupPermission(Group, toM))
    {
        sprintf(query, "delete from Group_%s where account = '%s'", Group, toM);
        mysql_query(&mysql, query);
        sprintf(query, "delete from %s_friendlist where account = '%s'", toM, Group);
        mysql_query(&mysql, query);
        sprintf(content, "%s 已经被移除群聊。", toM);
        setMsg(send_msg, GROUPTALK, ADMIN, Group, content);
        sendAdminMsg(send_msg, false);
        sprintf(content, "您已经被移除 %s 群聊。", Group);
        adminMsg(content, toM);
    }
    else
    {
        adminMsg("管理员设置请求失败，权限不足！", fromM);
    }
}
void Server::Grouptalk(Msg message, int call) //处理用户群聊请求
{
    char query[1024], memberlist[1536][32] = {0};
    sprintf(query, "select account from group_%s;", recv_msg.toUser);
    if (mysql_query(&mysql, query) == 0)
    {
        MYSQL_RES *res;
        MYSQL_ROW row;
        res = mysql_store_result(&mysql);
        if (res != NULL)
        {
            for (int i = 0; row = mysql_fetch_row(res); i++)
            {
                strcpy(memberlist[i], row[0]);
            }
        }
        else
        {
            cout << "Query error: " << mysql_error(&mysql) << endl;
        }
    }
    map<int, pair<string, int>>::iterator it;
    for (it = onlinelist.begin(); it != onlinelist.end(); it++)
    {
        for (int i = 0; memberlist[i] != "\0"; i++)
        {
            if (strEqual(it->second.first.c_str(), memberlist[i]))
            {
                if (it->first != call)
                {
                    sendMsg(message, it->first);
                }
            }
        }
    }
}
/*群聊及其权限部分*/
/*私聊部分*/
void Server::Privatetalk(Msg msg) //处理用户私聊请求
{
    map<int, pair<string, int>>::iterator i;
    int count = 0;
    for (i = onlinelist.begin(); i != onlinelist.end(); i++)
    {
        if (strEqual(i->second.first, msg.toUser))
        {
            sendMsg(msg, i->first);
        }
        else
        {
            count++;
        }
    }
    if (count == onlinelist.size())
    {
        cout << "No hits found." << endl;
        /*发送失败反馈*/
        if (strEqual(msg.fromUser, ADMIN) == false)
        {
            sprintf(content, "发向用户 %s 的 “%s” 发送失败！可能是用户名错误或者对方不在线！", msg.toUser, msg.content);
            adminMsg(content, msg.fromUser);
        }
        else
        {
            cout << "Feedback error;" << endl;
        }
    }
}
/*私聊部分*/
/*服务端部分*/
