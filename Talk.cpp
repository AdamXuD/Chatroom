#include "Server.h"
#include "Client.h"

extern char content[5120];
extern char query[10240];

/*客户端部分*/
/*群聊相关权限部分*/
void Client::createGroupTalk()
{
    string command;
    input(command, "请输入群名称>");
    setMsg(msg, CREATEGROUP, acc.account, nullptr, command.c_str());
    sendMsg(msg, pipe_fd[1]);
    cout << "请求已发送！" << endl;
}

void Client::joinGroup()
{
    string command;
    input(command, "请输入群名称>");
    setMsg(msg, JOINGROUP, acc.account, nullptr, command.c_str());
    sendMsg(msg, pipe_fd[1]);
    cout << "请求已发送！" << endl;
}

void Client::queryGroupMember(const char *Group, bool show)
{
    if (show)
    {
        cout << "正在请求群员列表..." << endl;
    }
    sleep(1);
    setMsg(msg, QUERYMEMBER, acc.account, Group, nullptr);
    if (sendMsg(msg, pipe_fd[1]) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
    else
    {
        memberlist.clear();
        char query[5120];
        sprintf(query, "CREATE TABLE IF NOT EXISTS `group_%s` (account VARCHAR (32) NOT NULL, flag INT);", Group);
        sqlite3_exec(db, query, [](void *, int, char **, char **){return 0;}, nullptr, &err);
        sprintf(query, "DELETE FROM `group_%s`;", Group);
        sqlite3_exec(db, query, [](void *, int, char **, char **){return 0;}, nullptr, &err);
        while (1)
        {
            recvMsg(listpipe_fd[0], msg, true);
            if (msg.type == LIST)
            {
                sprintf(query, "INSERT INTO `group_%s` VALUES ('%s', %s);", Group, msg.fromUser, msg.content);
                sqlite3_exec(db, query, [](void *, int, char **, char **) { return 0; }, nullptr, &err);
                memberlist[msg.fromUser] = atoi(msg.content);
                if (show)
                {
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
    }
}

string Client::memberlistMenu(const char *group) //1为提取群组 0为提取好友
{
    queryGroupMember(group, false);
    string tmp[memberlist.size()];
    cout << "请用上下键选择目标>" << endl;
    map<string, int>::iterator it;
    int i = 0;
    for (it = memberlist.begin(); it != memberlist.end(); it++)
    {
        tmp[i] = it->first;
        i++;
    }
    return tmp[menu(tmp, i) - 1];
}

void Client::groupMenu(string toGroup)
{
    string group_menu[] = {"设定管理员",
                           "踢人",
                           "离开群组",
                           "拉取群员列表",
                           "返回"};
    string command;
    switch (menu(group_menu, 5))
    {
    case 1:
    {
        command = memberlistMenu(toGroup.c_str());
        if (strEqual(command, "__nullstr") == false)
        {
            setMsg(msg, SETADMIN, acc.account, toGroup.c_str(), command.c_str());
            sendMsg(msg, pipe_fd[1]);
        }
        break;
    }
    case 2:
    {
        command = memberlistMenu(toGroup.c_str());
        if (strEqual(command, "__nullstr") == false)
        {
            setMsg(msg, KICKOFFMEMBER, acc.account, toGroup.c_str(), command.c_str());
            sendMsg(msg, pipe_fd[1]);
        }
        break;
    }
    case 3:
    {
        cout << "您确定要退出 " << toGroup << "吗？（确定请按y）" << endl;
        if (getch() == 'y' || getch() == 'Y')
        {
            setMsg(msg, LEAVEGROUP, acc.account, toGroup.c_str(), nullptr);
            sendMsg(msg, pipe_fd[1]);
        }
        break;
    }
    case 4:
    {
        queryGroupMember(toGroup.c_str(), true);
        user_wait();
        break;
    }
    default:
    {
        break;
    }
    }
    clear();
}

void Client::Grouptalk(string command) //处理用户群聊请求
{
    string toGroup = command;
    setMsg(msg, PIPE, nullptr, toGroup.c_str(), nullptr);
    queryGroupMember(toGroup.c_str(), false);
    sendMsg(msg, pipe_fd[1]);
    cout << "你正在 " << toGroup << " 群内发言：（按ESC退出）" << endl;
    while (1)
    {
        string command;
        if (mygetline(command) == EOF)
        {
            cout << "您已结束与" << toGroup << "的聊天！" << endl;
            setMsg(msg, PIPE, nullptr, "__ null__", nullptr);
            sendMsg(msg, pipe_fd[1]);
            break;
        }
        if (strEqual(command, "/menu")) //指令语意处理
        {
            command.clear();
            groupMenu(toGroup);
            cout << "你正在 " << toGroup << " 群内发言：" << endl;
        }
        else
        {
            setMsg(msg, GROUPTALK, acc.account, toGroup.c_str(), command.c_str());
            sendMsg(msg, pipe_fd[1]);
        }
    }
}
/*群聊与相关权限部分*/

/*私聊部分*/
void Client::Privatetalk(string command) //处理用户私聊请求
{
    string toUser = command;
    char buf[65535];
    cout << "你正在与 " << toUser << " 聊天：(按ESC退出)" << endl;
    setMsg(msg, PIPE, nullptr, toUser.c_str(), nullptr);
    sendMsg(msg, pipe_fd[1]);
    while (1)
    {
        string tmp;
        if (mygetline(tmp) == EOF)
        {
            cout << "您已结束与" << toUser << "的聊天！" << endl;
            setMsg(msg, PIPE, nullptr, "__ null__", nullptr);
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
    sprintf(query, "select flag from `group_%s` where account = '%s';", Group, Member);
    Mysql_query(&mysql, query);
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
    sprintf(query, "select account, flag from `group_%s`", recv_msg.toUser);
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
            sendMsg(send_msg, call);
        }
    }
    usleep(30000);
    setMsg(send_msg, EOF, nullptr, nullptr, "EOF");
    sendMsg(send_msg, call);
}
void Server::sendAdminMsg(Msg message, bool sw_query, char *Group, char *fromUser)
{
    char memberlist[1536][32] = {0};
    sprintf(query, "select account from `group_%s` where flag = '1' or flag = '2';", Group);
    if (Mysql_query(&mysql, query) == 0)
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
    if (sw_query)
    {
        if (fromUser != nullptr && Group != nullptr)
        {
            for (int i = 0; strcmp(memberlist[i], "") != 0; i++)
            {
                sprintf(query, "insert into %s_querybox values (null, '%s', '%s', '%s', '%s');", memberlist[i], "GROUP", fromUser, Group, message.content);
                Mysql_query(&mysql, query);
            }
        }
    }
    map<int, pair<string, int>>::iterator it;
    for (it = onlinelist->begin(); it != onlinelist->end(); it++)
    {
        for (int i = 0; strcmp(memberlist[i], "") != 0; i++)
        {
            if (strEqual(it->second.first.c_str(), memberlist[i]))
            {
                sendMsg(message, it->first);
            }
        }
    }
}
void Server::createGroupTalk() //创建群聊
{
    sprintf(query, "insert into groupinfo values ('%s')", recv_msg.content);
    if (Mysql_query(&mysql, query) != 0)
    {
        adminMsg("创建群组失败，可能是同名群组已存在！", recv_msg.fromUser);
        return;
    }
    sprintf(query, "create table `group_%s` like `group`;", recv_msg.content);
    Mysql_query(&mysql, query);
    sprintf(query, "insert into %s_friendlist values ('%s', '1');", recv_msg.fromUser, recv_msg.content);
    Mysql_query(&mysql, query);
    sprintf(query, "insert into group_%s values ('%s', '2');", recv_msg.content, recv_msg.fromUser);
    Mysql_query(&mysql, query);
    adminMsg("群组创建成功，请手动刷新好友列表！", recv_msg.fromUser);
}
void Server::addGroupMember(char *Group, char *Member)
{
    sprintf(query, "insert into `group_%s` values ('%s', '0');", Group, Member);
    Mysql_query(&mysql, query);
    sprintf(query, "insert into %s_friendlist values ('%s', '1');", Member, Group);
    Mysql_query(&mysql, query);
    sprintf(content, "%s 加入群聊 %s 啦！", Member, Group);
    setMsg(send_msg, GROUPTALK, ADMIN, Group, content);
    Grouptalk(send_msg);
    usleep(30000);
    sprintf(content, "你已加入群聊 %s 啦！", Group);
    adminMsg(content, Member);
    usleep(30000);
}
void Server::setGroupAdmin() //设置管理员
{
    char fromM[32], toM[32], Group[32];
    strcpy(fromM, recv_msg.fromUser);
    strcpy(toM, recv_msg.content);
    strcpy(Group, recv_msg.toUser);
    if (groupPermission(Group, fromM) > groupPermission(Group, toM))
    {
        sprintf(query, "update `group_%s` set flag = '1' where account = '%s'", Group, toM);
        Mysql_query(&mysql, query);
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
    sprintf(query, "select `group` from groupinfo where `group`='%s';", recv_msg.content);
    if (Mysql_query(&mysql, query) == 0)
    {
        MYSQL_RES res;
        MYSQL_ROW row;
        res = *mysql_store_result(&mysql);
        row = mysql_fetch_row(&res);
        if (row != NULL)
        {
            if (targetExisted(false) == 0)
            {
                sprintf(content, "用户 %s 请求加入 %s 群聊！", recv_msg.fromUser, recv_msg.content);
                setMsg(send_msg, QUERY, ADMIN, nullptr, content);
                sendAdminMsg(send_msg, true, recv_msg.content, recv_msg.fromUser);
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
    sprintf(query, "delete from `group_%s` where account = '%s'", recv_msg.toUser, recv_msg.fromUser);
    Mysql_query(&mysql, query);
    sprintf(query, "delete from %s_friendlist where account = '%s'", recv_msg.fromUser, recv_msg.toUser);
    Mysql_query(&mysql, query);
    sprintf(content, "%s 已离开 %s 群聊！", recv_msg.fromUser, recv_msg.toUser);
    setMsg(send_msg, GROUPTALK, ADMIN, recv_msg.toUser, content);
    sendAdminMsg(send_msg, false, recv_msg.toUser, recv_msg.fromUser);
}
void Server::deleteGroupMember() //处理踢出群聊请求
{
    char fromM[32], toM[32], Group[32];
    strcpy(fromM, recv_msg.fromUser);
    strcpy(toM, recv_msg.content);
    strcpy(Group, recv_msg.toUser);
    if (groupPermission(Group, fromM) > groupPermission(Group, toM))
    {
        sprintf(query, "delete from `group_%s` where account = '%s'", Group, toM);
        Mysql_query(&mysql, query);
        sprintf(query, "delete from %s_friendlist where account = '%s'", toM, Group);
        Mysql_query(&mysql, query);
        sprintf(content, "%s 已经被移除群聊。", toM);
        setMsg(send_msg, GROUPTALK, ADMIN, Group, content);
        sendAdminMsg(send_msg, false, Group);
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
    if (strEqual(message.fromUser, ADMIN) == false)
    {
        sprintf(query, "insert into history values (null, '%s', '%s', '%s', '%s');", message.fromUser, message.toUser, message.content, getTime());
        Mysql_query(&mysql, query);
    }
    char query[1024], memberlist[1536][32] = {0};
    sprintf(query, "select account from `group_%s`;", message.toUser);
    if (Mysql_query(&mysql, query) == 0)
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
    for (it = onlinelist->begin(); it != onlinelist->end(); it++)
    {
        for (int i = 0; strcmp(memberlist[i], ""); i++)
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
    if (strEqual(msg.fromUser, ADMIN) == false)
    {
        sprintf(query, "insert into history values (null, '%s', '%s', '%s', '%s');", msg.fromUser, msg.toUser, msg.content, getTime());
        Mysql_query(&mysql, query);
    }
    map<int, pair<string, int>>::iterator i;
    int count = 0;
    for (i = onlinelist->begin(); i != onlinelist->end(); i++)
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
    if (count == onlinelist->size())
    {
        cout << "No hits found." << endl;
        /*发送失败反馈*/
        if (strEqual(msg.fromUser, ADMIN) == false)
        {
            sprintf(content, "发向用户 %s 的 “%s” 发送失败！可能是用户名错误或者对方不在线！", msg.toUser, msg.content);
            adminMsg(content, msg.fromUser);
        }
    }
}
/*私聊部分*/

/*服务端部分*/
