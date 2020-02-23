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
    pthread_t heartbeat;
    if(pthread_create(&heartbeat, NULL, HeartBeat, (void *)this) < 0)
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
            getline(cin, tmp, '\n'); //按行读取消息 不跳空格的那种
            /*广播消息部分开始*/
            memset(&msg, 0, sizeof(msg));
            msg.type = ALL;
            strcpy(msg.fromUser, acc.account);
            strcpy(msg.content, tmp.c_str());
            memset(buf, 0, 65535);
            memcpy(buf, &msg, sizeof(msg));
            write(pipe_fd[1], buf, sizeof(buf));
            /*广播消息部分结束*/
        }
    }
    else //父进程执行区（pid > 0）
    {
        close(pipe_fd[1]); //关闭写端
        while (isLogin)
        {
            int epoll_events_count = epoll_wait(epoll_fd, events, 2, -1);
            for (int i = 0; i < epoll_events_count; i++)
            {
                memset(buf, 0, sizeof(buf));
                if (events[i].data.fd == sock_fd) //epoll响应来自服务器标识符时
                {
                    recvMsg(sock_fd, msg);
                    cout << ">" << msg.fromUser << " 说：" << msg.content << endl;
                }
                else //管道标识符响应时
                {
                    read(events[i].data.fd, buf, 65535);
                    if (send(sock_fd, buf, sizeof(buf), 0) < 0)
                    {
                        cout << "发送失败！" << endl;
                        cout << "服务器连接失败，请检查网络连接！" << endl;
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