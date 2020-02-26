#include "Server.h"
#include "Client.h"

/*客户端部分*/
/*群聊相关权限部分*/
void Client::createGroupTalk(string command) //创建群聊
{
    msg.type = COMMAND;
    strcpy(msg.fromUser, acc.account);
    strcpy(msg.content, command.c_str());
    if (sendMsg(msg, sock_fd) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
}
void Client::setGroupAdmin(string command, string toGroup) //设置管理员
{
    msg.type = COMMAND;
    strcpy(msg.fromUser, acc.account);
    strcpy(msg.toUser, toGroup.c_str());
    strcpy(msg.content, command.c_str());
    if (sendMsg(msg, sock_fd) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
}
void Client::joinGroup(string command) //加入群聊
{
    msg.type = COMMAND;
    strcpy(msg.fromUser, acc.account);
    strcpy(msg.toUser, command.substr(sizeof("joingroup ") - 1).c_str());
    strcpy(msg.content, "JOINGROUP");
    if (sendMsg(msg, sock_fd) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
}
void Client::leaveGroup(string Group) //主动离开群聊
{
    msg.type = COMMAND;
    strcpy(msg.fromUser, acc.account);
    strcpy(msg.toUser, Group.c_str());
    strcpy(msg.content, "LEAVEGROUP");
    if (sendMsg(msg, sock_fd) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
}
void Client::deleteGroupMember(string command, string Group) //踢人
{
    msg.type = COMMAND;
    strcpy(msg.fromUser, acc.account);
    strcpy(msg.toUser, Group.c_str());
    strcpy(msg.content, command.c_str());
    if (sendMsg(msg, sock_fd) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
}
void Client::queryGroupMember(string Group)
{
    msg.type = COMMAND;
    strcpy(msg.fromUser, acc.account);
    strcpy(msg.toUser, Group.c_str());
    strcpy(msg.content, "QUERYMEMBER");
    if (sendMsg(msg, sock_fd) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
    else
    {
        cout << "正在请求群员列表..." << endl;
        char buf[65535] = {0};
        char memberList[1536][32] = {0};
        fstream ml;
        ml.open(Group.c_str(), ios::out);
        while (1)
        {
            sleep(1);
            recv(sock_fd, buf, sizeof(buf), 0);
            memcpy(memberList, buf, sizeof(buf));
            if (strstr(memberList[0], "account") != NULL)
            {
                ml << "account ";
                ml << "flag" << endl;
                for (int i = 2; memberList[i] != "\0"; i += 2)
                {
                    ml << memberList[i] << " ";
                    ml << memberList[i + 1] << endl;
                }
                break;
            }
        }
        ml.close();
    }
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
        if (*tmp.begin() == '/') //指令语意处理
        {
            tmp.erase(0, 1); //吃掉一个斜杠
            string command = tmp;
            if (command.find("admin ") != string::npos)
            {
                setGroupAdmin(command, toGroup);
            }
            else if (command.find("leave") != string::npos)
            {
                leaveGroup(toGroup);
            }
            else if(command.find("kickoff ") != string::npos)
            {
                deleteGroupMember(command, toGroup);
            }
            else if (command.find("querymember"))
            {
                queryGroupMember(toGroup);
            }
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
        if (tmp == "_exit")
        {
            msg.type = PIPE;
            strcpy(msg.toUser, "\0");
            memset(buf, 0, sizeof(buf));
            memcpy(buf, &msg, sizeof(msg));
            write(pipe_fd[1], buf, sizeof(buf));
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
int Server::groupPermission(char *Group, char *Member)
{
    char query[1024];
    sprintf(query, "select flag from group_%s where account = '%s';", Group, Member);
    if (mysql_query(&mysql, query) == 0)
    {
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
        return -1;
    }
    return -1;
}
void Server::SendGroupMember(int call)
{
    char query[1024], buf[65535], memberList[1536][32] = {0};
    strcpy(memberList[0], "account");
    strcpy(memberList[1], "flag");
    sprintf(query, "select account, flag from group_%s", msg.toUser);
    if (mysql_query(&mysql, query) == 0)
    {
        MYSQL_RES *res;
        MYSQL_ROW row;
        res = mysql_store_result(&mysql);
        if (res != NULL)
        {
            for (int i = 2; row = mysql_fetch_row(res); i += 2)
            {
                strcpy(memberList[i], row[0]);
                strcpy(memberList[i + 1], row[1]);
            }
        }
        else
        {
            cout << "Query memberlist error:" << mysql_error(&mysql) << endl;
        }
    }
    else
    {
        cout << "Query memberlist error:" << mysql_error(&mysql) << endl;
    }
    memset(buf, 0, sizeof(buf));
    memcpy(buf, memberList, sizeof(memberList));
    send(call, buf, sizeof(buf), 0);
}
void Server::sendAdminMsg(Msg message, bool sw_query, char *fromUser)
{
    char query[5120], memberlist[1536][32] = {0};
    sprintf(query, "select account from group_%s where flag = 1 or flag = 2;", msg.toUser);
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
            if (strstr(it->second.first.c_str(), memberlist[i]) != NULL)
            {
                sendMsg(message, it->first);
            }
        }
    }
    if(sw_query)
    {
        for (int i = 0; memberlist[i] != "\0"; i++)
        {
            sprintf(query, "insert into %s_querybox values (null, '%s', '%s', '%s', '%s');", memberlist[i], "GROUP", fromUser, message.toUser, message.content);
            if(mysql_query(&mysql, query) != 0)
            {
                cout << memberlist[i] << ": Query error: " << mysql_error(&mysql) << endl;
            }
        }
    }
}
void Server::createGroupTalk() //创建群聊
{
    char Group[32], query[1024];
    string command = msg.content;
    strcpy(Group, command.substr(sizeof("creategroup ") - 1).c_str());
    sprintf(query, "insert into groupinfo values ('%s')", Group);
    if(mysql_query(&mysql, query) != 0)
    {
        cout << "Create group error: " << mysql_error(&mysql) <<endl;
        Msg tmp;
        tmp.type = PRIVTALK;
        strcpy(tmp.fromUser, "Admin");
        strcpy(tmp.toUser, msg.fromUser);
        strcpy(tmp.content, "创建群聊请求失败，请稍后再试！");
        Privatetalk(msg);
        return;
    }
    sprintf(query, "create table group_%s like group;", Group);
    if (mysql_query(&mysql, query) != 0)
    {
        cout << "Create group error: " << mysql_error(&mysql) << endl;
        sprintf(query, "delete from groupinfo where group = '%s';", Group);
        mysql_query(&mysql, query);
        Msg tmp;
        tmp.type = PRIVTALK;
        strcpy(tmp.fromUser, "Admin");
        strcpy(tmp.toUser, msg.fromUser);
        strcpy(tmp.content, "创建群聊请求失败，请稍后再试！");
        Privatetalk(msg);
        return;
    }
    sprintf(query, "insert into %s_friendlist values ('%s', 1);", msg.fromUser, Group);
    if (mysql_query(&mysql, query) != 0)
    {
        cout << "Create group error: " << mysql_error(&mysql) << endl;
        sprintf(query, "delete from groupinfo where group = '%s';", Group);
        mysql_query(&mysql, query);
        sprintf(query, "drop table group_%s;", Group);
        mysql_query(&mysql, query);
        Msg tmp;
        tmp.type = PRIVTALK;
        strcpy(tmp.fromUser, "Admin");
        strcpy(tmp.toUser, msg.fromUser);
        strcpy(tmp.content, "创建群聊请求失败，请稍后再试！");
        Privatetalk(msg);
        return;
    }
}
int Server::addGroupMember(char *Group, char *Member)
{
    char query[1024];
    sprintf(query, "insert into group_%s values ('%s', 0);", Group, Member);
    int ret = mysql_query(&mysql, query);
    if(ret < 0)
    {
        return ret;
    }
    else
    {
        sprintf(query, "insert into %s_friendlist values ('%s', 1);", Member, Group);
        if(mysql_query(&mysql, query) != 0)
        {
            sprintf(query, "delete from group_%s where account = '%s'", Group, Member);
            mysql_query(&mysql, query);
            return -1;
        }
        msg.type = GROUPTALK;
        strcpy(msg.fromUser, "Admin");
        strcpy(msg.toUser, Group);
        sprintf(msg.content, "%s 加入群聊 %s 啦！", Member, Group);
        Grouptalk(msg);
        return ret;
    }
}
void Server::setGroupAdmin() //设置管理员
{
    char fromM[32], toM[32], Group[32];
    string command = msg.content;
    strcpy(fromM, msg.fromUser);
    strcpy(toM, command.substr(sizeof("admin ") - 1).c_str());
    strcpy(Group, msg.toUser);
    if (groupPermission(Group, fromM) > groupPermission(Group, toM))
    {
        char query[1024];
        sprintf(query, "update Group_%s set flag = '1' where account = '%s'", Group, toM);
        if (mysql_query(&mysql, query) == 0)
        {
            msg.type = GROUPTALK;
            strcpy(msg.fromUser, "Admin");
            strcpy(msg.toUser, fromM);
            sprintf(msg.content, "%s 已经被设置成管理员。", toM);
            Grouptalk(msg);
        }
        else
        {
            cout << "Query error: " << mysql_error(&mysql) << endl;
            msg.type = PRIVTALK;
            strcpy(msg.fromUser, "Admin");
            strcpy(msg.toUser, fromM);
            strcpy(msg.content, "管理员设置请求失败，请稍后再试！");
            Privatetalk(msg);
        }
    }
    else
    {
        msg.type = PRIVTALK;
        strcpy(msg.fromUser, "Admin");
        strcpy(msg.toUser, fromM);
        strcpy(msg.content, "管理员设置请求失败，权限不足！");
        Privatetalk(msg);
    }
}
void Server::joinGroupQuery() //处理加入群聊请求
{
    char query[5120];
    sprintf(query, "select group from groupinfo where group='%s';", msg.toUser);
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
            sprintf(tmp.content, "用户\033[33m %s \033[0m请求加入%s群聊！", msg.fromUser, msg.toUser);
            sendAdminMsg(tmp, true, msg.fromUser);
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
void Server::leaveGroup() //处理离开群聊请求
{
    char query[1024];
    Msg tmp;
    sprintf(query, "delete from group_%s where account = '%s'", msg.toUser, msg.fromUser);
    if(mysql_query(&mysql, query) == 0)
    {
        sprintf(query, "delete from %s_friendlist where account = '%s'", msg.fromUser, msg.toUser);
        mysql_query(&mysql, query);
        tmp.type = GROUPTALK;
        strcpy(tmp.fromUser, "Admin");
        strcpy(tmp.toUser, msg.toUser);
        sprintf(tmp.content, "%s 已离开 %s 群聊！", msg.fromUser, msg.toUser);
        Grouptalk(tmp);
    }
    else
    {
        cout << "Leaving group error: " << mysql_error(&mysql) << endl;
        tmp.type = PRIVTALK;
        strcpy(tmp.fromUser, "Admin");
        strcpy(tmp.toUser, msg.fromUser);
        strcpy(tmp.content, "请求失败，请稍后再试！");
        Privatetalk(tmp);
    }
}
void Server::deleteGroupMember() //处理踢出群聊请求
{
    char fromM[32], toM[32], Group[32];
    string command = msg.content;
    strcpy(fromM, msg.fromUser);
    strcpy(toM, command.substr(sizeof("kickoff ") - 1).c_str());
    strcpy(Group, msg.toUser);
    if (groupPermission(Group, fromM) > groupPermission(Group, toM))
    {
        char query[1024];
        sprintf(query, "delete from Group_%s where account = '%s'", Group, toM);
        if (mysql_query(&mysql, query) == 0)
        {
            sprintf(query, "delete from %s_friendlist where account = '%s'", toM, Group);
            mysql_query(&mysql, query);
            msg.type = GROUPTALK;
            strcpy(msg.fromUser, "Admin");
            strcpy(msg.toUser, fromM);
            sprintf(msg.content, "%s 已经被移除群聊。", toM);
            Grouptalk(msg);
            msg.type = PRIVTALK;
            strcpy(msg.fromUser, "Admin");
            strcpy(msg.toUser, toM);
            sprintf(msg.content, "您已经被移除 %s 群聊。", Group);
            Privatetalk(msg);
        }
        else
        {
            cout << "Query error: " << mysql_error(&mysql) << endl;
            msg.type = PRIVTALK;
            strcpy(msg.fromUser, "Admin");
            strcpy(msg.toUser, fromM);
            strcpy(msg.content, "移除群聊请求失败，请稍后再试！");
            Privatetalk(msg);
        }
    }
    else
    {
        msg.type = PRIVTALK;
        strcpy(msg.fromUser, "Admin");
        strcpy(msg.toUser, fromM);
        strcpy(msg.content, "管理员设置请求失败，权限不足！");
        Privatetalk(msg);
    }
}
void Server::Grouptalk(Msg message, int call) //处理用户群聊请求
{
    char query[1024], memberlist[1536][32] = {0};
    sprintf(query, "select account from group_%s;", msg.toUser);
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
            if (strstr(it->second.first.c_str(), memberlist[i]) != NULL)
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
        if (i->second.first == msg.toUser)
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
        if (strstr(msg.fromUser, "Admin") == NULL)
        {
            char tmpacc[32], tmpcon[5120];
            sprintf(tmpcon, "发向用户 %s 的 “%s” 发送失败！可能是用户名错误或者对方不在线！", msg.toUser, msg.content);
            strcpy(tmpacc, msg.fromUser);
            memset(&msg, 0, sizeof(msg));
            msg.type = PRIVTALK;
            strcpy(msg.fromUser, "Admin");
            strcpy(msg.toUser, tmpacc);
            strcpy(msg.content, tmpcon);
            Privatetalk(msg);
        }
        else
        {
            cout << "Feedback error;" << endl;
        }
    }
}
/*私聊部分*/
/*服务端部分*/
