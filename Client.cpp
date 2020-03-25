#include "Client.h"

extern char content[5120];
extern char query[10240];

Client::Client() //构造
{
    sock_fd = -1;
    epoll_fd = -1;
    pipe_fd[2] = {0};
    memset(&acc, 0, sizeof(acc));
    isLogin = false;
}
void Client::Connect() //连接服务器用函数
{
    clear();
    char ip[17] = {0};
    int port;
    input(ip, "请输入服务器IP：");
    input(port, "请输入端口号：");
    sock_fd = socket(AF_INET, SOCK_STREAM, 0); //socket部分开始
    if (sock_fd < 0)
    {
        cout << "Socket error;" << endl;
        user_wait();
        exit(-1);
    }
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons((unsigned)port);
    for (int i = 0; i <= 3; i++)
    {
        cout << "Connecting……" << endl;
        if (connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            cout << "Connecting error;" << endl;
            if (i == 3)
            {
                cout << "Please try to restart client!" << endl;
                user_wait();
                exit(-1);
            }
            else
            {
                cout << "Try to reconnect;" << endl;
            }
        }
        else
        {
            break;
        }
    }
    if (pipe(pipe_fd) < 0) //多进程时使用（建立管道描述符）
    {
        cout << "Other error;" << endl;
        user_wait();
        exit(-1);
    }
    epoll_fd = epoll_create(1024);
    if (epoll_fd < 0)
    {
        cout << "Other error;" << endl;
        user_wait();
        exit(-1);
    }
    addepollfd(epoll_fd, sock_fd);
    addepollfd(epoll_fd, pipe_fd[0]);
    cout << "Connected." << endl;
    user_wait();
}
void *HeartBeat(void *pointer)
{
    int count = 0;
    Client *ptr = (Client *)pointer;
    while (count < 3)
    {
        if (sendHeartBeats(ptr->sock_fd, ptr->msg) < 0)
        {
            cout << "与服务器断开连接，正在尝试第 " << count + 1 << " 次重连..." << endl;
            count++;
        }
        sleep(3);
    }
    if (count >= 3)
    {
        cout << "重连失败，程序即将退出！" << endl;
        exit(-1);
    }
}

void Client::dealwithmsg(char *Target)
{
    switch (msg.type)
    {
    case ALL:
    {
        cout << "\033[31m[广播]\033[0m";
        break;
    }
    case PRIVTALK:
    {
        cout << "\033[32m[私聊]\033[0m";
        break;
    }
    case GROUPTALK:
    {
        cout << "\033[34m[群聊]\033[0m";
        break;
    }
    default:
    {
        return;
    }
    }
    if (strEqual(msg.fromUser, Target) || strEqual(msg.fromUser, ADMIN))
    {
        cout << "\033[33m>" << msg.fromUser << " 说：" << msg.content << "\033[0m" << endl; //当指定了聊天对象时 聊天对象消息高亮
    }
    else
    {
        cout << msg.fromUser << " 说：" << msg.content << endl;
    }
}

void Client::mainMenu()
{
    setMsg(msg, WAIT, nullptr, nullptr, nullptr);
    string command;
    string main_menu[] = {"发起私聊", // 1
                          "发起群聊",
                          "添加好友",
                          "删除好友",
                          "处理请求", // 5
                          "设置特别关心",
                          "设置黑名单",
                          "拉取好友列表",
                          "创建群组",
                          "加入群组", //10
                          "返回"};    //11
    switch (menu(main_menu, 10))
    {
    case 1:
    {
        command = friendlistMenu(true);
        Privatetalk(command);
        break;
    }
    case 2:
    {
        command = friendlistMenu(false);
        Grouptalk(command);
        break;
    }
    case 3:
    {
        input(command, "请输入好友昵称>");
        setMsg(msg, MAKEFRIEND, acc.account, nullptr, command.c_str());
        sendMsg(msg, sock_fd);
        break;
    }
    case 4:
    {
        command = friendlistMenu(true);
        setMsg(msg, DELETEFRIEND, acc.account, nullptr, command.c_str());
        sendMsg(msg, sock_fd);
        break;
    }
    case 5:
    {
        getQueryBox(false);

        sendMsg(msg, sock_fd);
        break;
    }
    case 6:
    {
        command = friendlistMenu(true);
        setMsg(msg, SUKI, acc.account, nullptr, command.c_str());
        sendMsg(msg, sock_fd);
        break;
    }
    case 7:
    {
        command = friendlistMenu(true);
        setMsg(msg, KIRAI, acc.account, nullptr, command.c_str());
        sendMsg(msg, sock_fd);
        break;
    }
    case 8:
    {
        queryFriendList(true);
        break;
    }
    case 9:
    {
        input(command, "请输入群名称>");
        setMsg(msg, CREATEGROUP, acc.account, nullptr, command.c_str());
        sendMsg(msg, sock_fd);
        break;
    }
    case 10:
    {
        input(command, "请输入群名称>");
        setMsg(msg, JOINGROUP, acc.account, nullptr, command.c_str());
        sendMsg(msg, sock_fd);
        break;
    }
    default:
    {
        break;
    }
    }
}

string Client::friendlistMenu(bool isFriend) //1为提取群组 0为提取好友
{
    string tmp[friendlist.size()];
    map<string, int>::iterator it;
    int i = 0;
    for (it = friendlist.begin(); it != friendlist.end(); it++)
    {
        if (isFriend)
        {
            if (it->second == 0)
            {
                tmp[i] = it->first;
                i++;
            }
        }
        else
        {
            if (it->second == 1)
            {
                tmp[i] = it->first;
                i++;
            }
        }
    }
    return tmp[menu(tmp, i) - 1];
}

int Client::getQueryBox(bool show)
{
    if (show)
    {
        cout << "正在请求待处理请求列表..." << endl;
    }
    sleep(1);
    setMsg(msg, QUERYBOX, acc.account, nullptr, nullptr);
    if (sendMsg(msg, sock_fd) < 0)
    {
        cout << "请求失败，请检查网络连接！" << endl;
    }
    else
    {
        while (1)
        {
            recvMsg(sock_fd, msg, true);
            if (msg.type == QUERYBOX)
            {
                Querybox.push_back(msg);
                if (show)
                {
                    cout << "id：" << msg.fromUser << endl;
                    cout << "内容：" << msg.content << endl;
                    cout << endl;
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

void Client::Start() //客户端入口
{
    static struct epoll_event events[2];
    char buf[65535];
    Connect();
    if (LOGINMODE)
    {
        fileLogin();
    }
    else
    {
        input(acc.account, "请输入昵称："); //非登录模式下获取用户名
        isLogin = true;
    }
    pthread_t heartbeat;                                               //建立新进程
    if (pthread_create(&heartbeat, NULL, HeartBeat, (void *)this) < 0) //创建一个子进程用来发送心跳包 并维持连接状态（传递参数为该客户端对象指针）
    {
        cout << "Heartbeat thread error..." << endl;
        user_wait();
        exit(-1);
    }
    pid = fork();
    if (pid < 0)
    {
        cout << "程序出错，程序即将退出！" << endl;
        exit(-1);
    }
    else if (pid > 0) //子进程执行区（pid = 0）
    {
        close(pipe_fd[0]); //子进程负责写入，关闭读端
        cout << "正在获取信息..." << endl;
        queryFriendList(false);
        getQueryBox(true);
        cout << "获取完毕！" << endl;
        cout << "全服广播(请直接在下方输入消息，enter键发送，/menu唤出菜单)>" << endl;
        while (isLogin)
        {
            string tmp;
            getline(cin, tmp, '\n'); //按行读取指令 不跳空格的那种
            /*判断指令类型*/
            if (strEqual(tmp, "/menu")) //指令语意处理
            {
                tmp.clear();
                mainMenu();
            }
            else
            {
                setMsg(msg, ALL, acc.account, nullptr, tmp.c_str());
                sendMsg(msg, pipe_fd[1]);
            }
        }
    }
    else //父进程执行区（pid > 0）
    {
        close(pipe_fd[1]); //关闭写端

        char Target[32] = {0}; //存储聊天对象

        while (isLogin)
        {
            int epoll_events_count = epoll_wait(epoll_fd, events, 2, -1);
            for (int i = 0; i < epoll_events_count; i++)
            {
                memset(buf, 0, sizeof(buf));
                if (events[i].data.fd == sock_fd) //epoll响应来自服务器标识符时
                {
                    recvMsg(sock_fd, msg, false);
                    dealwithmsg(Target);
                }
                else //管道标识符响应时
                {
                    recvMsg(pipe_fd[0], msg, false);
                    if (msg.type == PIPE)
                    {
                        strcpy(Target, msg.toUser);
                    }
                    else
                    {
                        if (sendMsg(msg, sock_fd) < 0)
                        {
                            cout << "发送失败！" << endl;
                            cout << "服务器连接失败，请检查网络连接！" << endl;
                        }
                    }
                }
            }
        }
    }
}
Client::~Client() //析构
{
    if (pid > 0)
    {
        close(pipe_fd[0]); //父进程关闭管道读端
        close(sock_fd);    //关闭socket句柄
    }
    else
    {
        close(pipe_fd[1]); //子进程关闭写端
    }
}