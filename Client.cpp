#include "Client.h"

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
    cout << "请输入服务器IP：" << endl;
    cin.clear();
    cin.sync();
    cin >> ip;
    getchar();
    cout << "请输入端口号：" << endl;
    cin.clear();
    cin.sync();
    cin >> port;
    getchar();
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
void Client::Start() //客户端入口
{
    static struct epoll_event events[2];
    char buf[65535];
    //Getfriendlist();
    Connect();
    if (LOGINMODE)
    {
        fileLogin();
    }
    else
    {
        string tmp;
        cout << "请输入昵称：" << endl;
        getline(cin, tmp, '\n'); //非登录模式下获取用户名
        strcpy(acc.account, tmp.c_str());
        isLogin = true;
    }
    pthread_t heartbeat;  //建立新进程
    if(pthread_create(&heartbeat, NULL, HeartBeat, (void *)this) < 0) //创建一个子进程用来发送心跳包 并维持连接状态（传递参数为该客户端对象指针）
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
    else if (pid == 0) //子进程执行区（pid = 0）
    {
        close(pipe_fd[0]); //子进程负责写入，关闭读端
        cout << "全服广播(请直接在下方输入消息，enter键发送)>" << endl;
        while (isLogin)
        {
            string tmp;
            getline(cin, tmp, '\n'); //按行读取指令 不跳空格的那种
            /*判断指令类型*/
            if (*tmp.begin() == '/') //指令语意处理
            {
                tmp.erase(0, 1);                          //吃掉一个斜杠
                string command = tmp;
                if (command.find("private ") != string::npos) //如果指令是私聊的话
                {
                    Privatetalk(command);
                }
                if (command.find("group ") != string::npos)
                {
                    Grouptalk(command);
                }
                if (command.find("group ", 0) == string::npos && command.find("private ", 0) == string::npos)
                {
                    cout << "指令有误，请检查指令格式！" << endl;
                }
                command.clear();
            }
            else
            {
                msg.type = ALL;
                strcpy(msg.fromUser, acc.account);
                strcpy(msg.content, tmp.c_str());
                memset(buf, 0, sizeof(buf));
                memcpy(buf, &msg, sizeof(msg));
                write(pipe_fd[1], buf, sizeof(buf));
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
                    recvMsg(sock_fd, msg);
                    char tmpmsg[5120] = {0};
                    if (msg.fromUser == Target)
                    {
                        sprintf(tmpmsg, "\033[92m>%s 说：%s\033[0m", msg.fromUser, msg.content); //当指定了聊天对象时 聊天对象消息高亮
                    }
                    else
                    {
                        sprintf(tmpmsg, ">%s 说：%s", msg.fromUser, msg.content); //没有指定或者非当前聊天对象发来消息时标准显示
                    }
                    cout << tmpmsg << endl;
                }
                else //管道标识符响应时
                {
                    memset(buf, 0, sizeof(buf));
                    read(events[i].data.fd, buf, sizeof(buf));
                    memcpy(&msg, buf, sizeof(msg));
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