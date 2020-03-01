#include "Server.h"
#include "Client.h"

/*客户端部分*/
/*群聊相关权限部分*/
void Client::createGroupTalk(string command) //创建群聊
{
    setMsg(msg, COMMAND, acc.account, nullptr, command.c_str());
    if (sendMsg(msg, sock_fd) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
}
void Client::setGroupAdmin(string command, string toGroup) //设置管理员
{
    setMsg(msg, COMMAND, acc.account, toGroup.c_str(), command.c_str());
    if (sendMsg(msg, sock_fd) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
}
void Client::joinGroup(string command) //加入群聊
{
    setMsg(msg, COMMAND, acc.account, command.substr(sizeof("joingroup ") - 1).c_str(), "JOINGROUP");
    if (sendMsg(msg, sock_fd) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
}
void Client::leaveGroup(string Group) //主动离开群聊
{
    setMsg(msg, COMMAND, acc.account, Group.c_str(), "LEAVEGROUP");
    if (sendMsg(msg, sock_fd) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
}
void Client::deleteGroupMember(string command, string Group) //踢人
{
    setMsg(msg, COMMAND, acc.account, Group.c_str(), command.c_str());
    if (sendMsg(msg, sock_fd) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
}
void Client::queryGroupMember(string Group)
{
    setMsg(msg, COMMAND, acc.account, Group.c_str(), "QUERYMEMBER");
    if (sendMsg(msg, sock_fd) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
    else
    {
        cout << "正在请求群员列表..." << endl;
        char memberList[1536][32] = {0};
        fstream ml;
        ml.open(Group.c_str(), ios::out);
        while (1)
        {
            recvMsg(sock_fd, (char *)memberList);
            if (strEqual(memberList[0], "account"))
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
    setMsg(msg, PIPE, nullptr, toGroup.c_str(), nullptr);
    sendMsg(msg, pipe_fd[1]);
    while (1)
    {
        string command;
        getline(cin, command, '\n');
        if (*command.begin() == '/') //指令语意处理
        {
            command.erase(0, 1); //吃掉一个斜杠
            if (strEqual(command, "admin "))
            {
                setGroupAdmin(command, toGroup);
            }
            else if (strEqual(command, "leave"))
            {
                leaveGroup(toGroup);
            }
            else if (strEqual(command, "kickoff "))
            {
                deleteGroupMember(command, toGroup);
            }
            else if (strEqual(command, "querymember"))
            {
                queryGroupMember(toGroup);
            }
        }
        else
        {
            setMsg(msg, GROUPTALK, acc.account, toGroup.c_str(), command.c_str());
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
    sendMsg((char *)memberList, call);
}
void Server::sendAdminMsg(Msg message, bool sw_query, char *fromUser)
{
    char query[5120], memberlist[1536][32] = {0};
    sprintf(query, "select account from group_%s where flag = '1' or flag = '2';", msg.toUser);
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
        setMsg(msg, PRIVTALK, ADMIN, msg.fromUser, "创建群聊请求失败，请稍后再试！");
        Privatetalk(msg);
        return;
    }
    sprintf(query, "create table group_%s like group;", Group);
    if (mysql_query(&mysql, query) != 0)
    {
        cout << "Create group error: " << mysql_error(&mysql) << endl;
        sprintf(query, "delete from groupinfo where group = '%s';", Group);
        mysql_query(&mysql, query);
        setMsg(msg, PRIVTALK, ADMIN, msg.fromUser, "创建群聊请求失败，请稍后再试！");
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
        setMsg(msg, PRIVTALK, ADMIN, msg.fromUser, "创建群聊请求失败，请稍后再试！");
        Privatetalk(msg);
        return;
    }
}
int Server::addGroupMember(char *Group, char *Member)
{
    char query[1024], content[4096];
    sprintf(query, "insert into group_%s values ('%s', '0');", Group, Member);
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
        sprintf(content, "%s 加入群聊 %s 啦！", Member, Group);
        setMsg(msg, GROUPTALK, ADMIN, Group, content);
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
            char content[4096];
            sprintf(content, "%s 已经被设置成管理员。", toM);
            setMsg(msg, GROUPTALK, ADMIN, fromM, content);
            Grouptalk(msg);
        }
        else
        {
            cout << "Query error: " << mysql_error(&mysql) << endl;
            setMsg(msg, PRIVTALK, ADMIN, fromM, "管理员设置请求失败，请稍后再试！");
            Privatetalk(msg);
        }
    }
    else
    {
        setMsg(msg, PRIVTALK, ADMIN, fromM, "管理员设置请求失败，权限不足！");
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
            char content[4096];
            sprintf(tmp.content, "用户 %s 请求加入%s群聊！", msg.fromUser, msg.toUser);
            setMsg(tmp, PRIVTALK, ADMIN, msg.toUser, content);
            sendAdminMsg(tmp, true, msg.fromUser);
        }
        else
        {
            setMsg(msg, PRIVTALK, ADMIN, msg.toUser, "好友添加请求失败，请确认该用户是否存在！");
            Privatetalk(msg);
        }
    }
}
void Server::leaveGroup() //处理离开群聊请求
{
    char query[1024];
    sprintf(query, "delete from group_%s where account = '%s'", msg.toUser, msg.fromUser);
    if(mysql_query(&mysql, query) == 0)
    {
        char content[4096];
        sprintf(content, "%s 已离开 %s 群聊！", msg.fromUser, msg.toUser);
        sprintf(query, "delete from %s_friendlist where account = '%s'", msg.fromUser, msg.toUser);
        mysql_query(&mysql, query);
        setMsg(msg, GROUPTALK, ADMIN, msg.toUser, content);
        Grouptalk(msg);
    }
    else
    {
        cout << "Leaving group error: " << mysql_error(&mysql) << endl;
        setMsg(msg, PRIVTALK, ADMIN, msg.fromUser, "请求失败，请稍后再试！");
        Privatetalk(msg);
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
            char content[4096];
            sprintf(query, "delete from %s_friendlist where account = '%s'", toM, Group);
            mysql_query(&mysql, query);
            sprintf(msg.content, "%s 已经被移除群聊。", toM);
            setMsg(msg, GROUPTALK, ADMIN, Group, content);
            Grouptalk(msg);
            sprintf(msg.content, "您已经被移除 %s 群聊。", Group);
            setMsg(msg, PRIVTALK, ADMIN, toM, content);
            Privatetalk(msg);
        }
        else
        {
            cout << "Query error: " << mysql_error(&mysql) << endl;
            setMsg(msg, PRIVTALK, ADMIN, fromM, "移除群聊请求失败，请稍后再试！");
            Privatetalk(msg);
        }
    }
    else
    {
        setMsg(msg, PRIVTALK, ADMIN, fromM, "管理员设置请求失败，权限不足！");
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
            char content[5120];
            sprintf(content, "发向用户 %s 的 “%s” 发送失败！可能是用户名错误或者对方不在线！", msg.toUser, msg.content);
            setMsg(msg, PRIVTALK, ADMIN, msg.fromUser, content);
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
