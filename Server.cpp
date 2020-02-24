#include "Server.h"

Server::Server()
{
    sock_fd = 0;
    epoll_fd = 0;
}
void Server::Prepare()
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
    cout << "服务器启动中……" << endl;
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
    if (bind(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        cout << "Socket errer;" << endl;
        user_wait();
        exit(-1);
    }
    if (listen(sock_fd, 5) < 0)
    {
        cout << "Change to LISTENING errer;" << endl;
        user_wait();
        exit(-1);
    }
    else
    {
        cout << "Server is LISTENING..." << endl;
    }
    epoll_fd = epoll_create(4096);
    if (epoll_fd < 0)
    {
        cout << "Other error" << endl;
        user_wait();
        exit(-1);
    }
    addepollfd(epoll_fd, sock_fd);
    /*数据库初始化部分开始*/
    if (LOGINMODE)
    {
        mysql_init(&mysql);
        if (!mysql_real_connect(&mysql, "127.0.0.1", "root", "root", "Chatroom", 3306, NULL, 0))
        {
            cout << "Failed to connect to database: Error:" << mysql_error(&mysql) << endl;
            user_wait();
            exit(-1);
        }
        else
        {
            cout << "Success to connect database" << endl;
        }
    }
    /*数据库初始化部分结束*/
    user_wait();
}
void Server::BroadcastMsg(int call)
{
    cout << "A user sends a broadcast message." << endl;
    map<int, pair<string, int>>::iterator i; //声明一个迭代器（相当于int i
    for (i = onlinelist.begin(); i != onlinelist.end(); i++)
    {
        if (i->first != call) //除了发信者外
        {
            cout << "Send message to client " << i->first << endl;
            sendMsg(msg, i->first); //给所有在线玩家发送消息
        }
    }
}
void Server::dealWithMsg(int call)
{
    switch (msg.type)
    {
    case 0:
    {
        cout << "illegal message:" << msg.content << endl;
        break;
    }
    case COMMAND: //如果收到命令类消息
    {
        if (strstr(msg.content, "SIGNIN") != NULL) //接收并处理客户端的登录请求
        {
            Login(call);
        }
        else if (strstr(msg.content, "SIGNUP") != NULL) //接收并处理客户端的注册请求
        {
            Signup(call);
        }
        else //还可以再添加其他的else 我想通过这种方式来实现组群加好友之类的操作 但是我觉得这样子效率有点低 你们看看有没有更好的方法
        {
            cout << "illegal message:" << msg.content << endl;
        }
        break;
    }
    case ALL:
    {
        cout << "From:" << msg.fromUser << endl;
        cout << "Say:" << msg.content << endl;
        BroadcastMsg(call);
        break;
    }
    case PRIVTALK: //下面两个可能写不完 先写
    {
        cout << "A user sends a message to another user." << endl;
        Privatetalk(call);
        break;
    }
    case GROUPTALK:
    {
        cout << "A user sends a message to a group" << endl;
        Grouptalk(call);
        break;
    }
    case HEARTBEAT:
    {
        onlinelist[call].second = 0;  //每收到一次心跳包心跳包参数置零
        break;
    }
    }
}
void* Server::dealWithHeartbeat(void *pointer)
{
    cout << "Heartbeat thread has been ready." << endl;
    Server *ptr = (Server *)pointer;
    while (1)
    {
        map<int, pair<string, int>>::iterator i = ptr->onlinelist.begin();
        for (; i != ptr->onlinelist.end(); i++) //遍历在线列表
        {
            if (i->second.second == 5)  //若存在心跳包参数为5的
            {
                cout << i->second.first << "has been offline." << endl;  //丢人 直接踢下线
                close(i->first);
                ptr->onlinelist.erase(i);
            }
            else if (i->second.second < 5)  //若心跳包参数小于5
            {
                i->second.second += 1;  //心跳包参数+1
            }
        }
        sleep(3);  //每三秒执行一次该循环
    }
    return 0;
}
void Server::Start() //服务端程序入口
{
    static struct epoll_event events[16384]; //设置标识符监听队列
    Prepare();
    pthread_t heartbeat;  //新建立一个pthread
    if(pthread_create(&heartbeat, NULL, dealWithHeartbeat, (void *)this) < 0)  //创建一个子进程处理心跳包（传递参数为该服务器对象指针）
    {
        cout << "Heartbeat thread error..." << endl;
        user_wait();
        exit(-1);
    }
    while (1)
    {
        int events_counter = epoll_wait(epoll_fd, events, 16384, -1); //在接收到事件之前保持阻塞
        for (int i = 0; i < events_counter; i++)
        {
            int call = events[i].data.fd;
            if (call == sock_fd) //如果事件是由服务器bind的socket上发生的
            {
                struct sockaddr_in clnt_addr;
                memset(&clnt_addr, 0, sizeof(clnt_addr));
                socklen_t addr_len = sizeof(struct sockaddr_in);
                int clnt_sock = accept(sock_fd, (struct sockaddr *)&clnt_addr, &addr_len); //接收第一次连接的客户端并在服务端上为其分配新的socket（离谱为什么bind只需要地址长度的值而accept要引用）
                cout << "Get a new Connect from:" << inet_ntoa(clnt_addr.sin_addr) << ":" << ntohs(clnt_addr.sin_port) << "..." << endl;
                cout << "Waiting for login……" << endl;
                addepollfd(epoll_fd, clnt_sock); //将分配后的标识符加入监听队列
                if (LOGINMODE == 0)
                {
                    cout << "Login success." << endl;
                    addonlinelist(call, acc.account);
                    cout << "Now there are " << onlinelist.size() << " user(s) online." << endl;
                    Onlineremind(clnt_sock);
                }
            }
            else //如果服务器来自epoll监控队列中的其他描述符（已被accept分配过新的标识符）
            {
                if (recvMsg(call, msg) < 0)
                {
                    cout << "Receive error." << endl;
                }
                else
                {
                    dealWithMsg(call);
                }
            }
        }
    }
}
Server::~Server()
{
    close(sock_fd);
    close(epoll_fd);
}